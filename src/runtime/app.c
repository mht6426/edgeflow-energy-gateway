/*
 * app.c — 三线程运行时编排（进阶版）
 */

#include "runtime/app.h"

#include "common/time_util.h"
#include "core/reactor.h"
#include "ingress/modbus_adapter.h"
#include "platform/logger.h"
#include "platform/watchdog.h"

#include <stdio.h>
#include <string.h>

static void apply_quality_rules(const gateway_config_t *cfg, telemetry_t *point)
{
    if (strcmp(point->point_id, "max_cell_temp_c") == 0 && point->value > cfg->temp_high) {
        point->quality = TELEMETRY_QUALITY_BAD;
    }
}

static void update_heartbeat_metrics(gateway_app_t *app)
{
    atomic_store(&app->metrics.heartbeat_ingress_age_ms,
                 (unsigned int)heartbeat_age_ms(&app->heartbeat, HEARTBEAT_THREAD_INGRESS));
    atomic_store(&app->metrics.heartbeat_worker_age_ms,
                 (unsigned int)heartbeat_age_ms(&app->heartbeat, HEARTBEAT_THREAD_WORKER));
    atomic_store(&app->metrics.heartbeat_monitor_age_ms,
                 (unsigned int)heartbeat_age_ms(&app->heartbeat, HEARTBEAT_THREAD_MONITOR));
}

static void *ingress_main(void *arg)
{
    gateway_app_t *app = arg;

    while (atomic_load(&app->running)) {
        telemetry_t points[8];
        int n = adapter_registry_poll_all(&app->registry, points, 8U);

        if (n > 0) {
            for (int i = 0; i < n; i++) {
                if (spsc_ring_push(&app->ring, &points[i])) {
                    atomic_fetch_add(&app->metrics.ingress_points, 1ULL);
                }
            }
        }

        heartbeat_touch(&app->heartbeat, HEARTBEAT_THREAD_INGRESS);
        atomic_store(&app->metrics.queue_depth, spsc_ring_count(&app->ring));
        atomic_store(&app->metrics.queue_dropped, spsc_ring_dropped(&app->ring));
        edgeflow_sleep_ms(app->cfg.poll_interval_ms);
    }
    return NULL;
}

static device_adapter_t *find_modbus_adapter(gateway_app_t *app)
{
    return adapter_registry_find(&app->registry, DEVICE_PROTOCOL_MODBUS_RTU);
}

static void *worker_main(void *arg)
{
    gateway_app_t *app = arg;
    uint64_t last_metrics_ms = 0U;

    while (atomic_load(&app->running)) {
        telemetry_t point;
        alarm_event_t alarm;
        command_t command;
        device_adapter_t *modbus;
        int64_t row_id = 0;

        if (!spsc_ring_pop(&app->ring, &point)) {
            heartbeat_touch(&app->heartbeat, HEARTBEAT_THREAD_WORKER);
            edgeflow_sleep_ms(20U);
            continue;
        }

        apply_quality_rules(&app->cfg, &point);
        if (storage_append_telemetry(app->storage, &point, &row_id) != 0) {
            log_warn("storage append telemetry failed");
        }
        atomic_fetch_add(&app->metrics.processed_points, 1ULL);

        memset(&alarm, 0, sizeof(alarm));
        memset(&command, 0, sizeof(command));
        if (state_engine_process_telemetry(&app->state_engine,
                                           &app->cfg,
                                           &point,
                                           &alarm,
                                           &command,
                                           app->command_scheduler.next_command_id,
                                           point.ts_ms)) {
            if (alarm.alarm_id != 0U) {
                (void)storage_append_alarm(app->storage, &alarm);
                log_warn("alarm: %s %s", alarm.code, alarm.message);
            }
            if (command.point_id[0] != '\0') {
                if (command_scheduler_submit(&app->command_scheduler, &command) == 0) {
                    modbus = find_modbus_adapter(app);
                    if (modbus != NULL) {
                        (void)command_scheduler_tick(&app->command_scheduler,
                                                     modbus,
                                                     &app->cfg,
                                                     app->storage);
                    }
                }
            }
        }

        if (mqtt_session_publish_telemetry(&app->mqtt, &point) == 0) {
            atomic_fetch_add(&app->metrics.mqtt_publish_ok, 1ULL);
            if (row_id > 0) {
                (void)storage_mark_telemetry_uploaded(app->storage, row_id);
            }
        } else {
            atomic_fetch_add(&app->metrics.mqtt_publish_fail, 1ULL);
        }

        heartbeat_touch(&app->heartbeat, HEARTBEAT_THREAD_WORKER);
        atomic_store(&app->metrics.queue_depth, spsc_ring_count(&app->ring));
        atomic_store(&app->metrics.queue_dropped, spsc_ring_dropped(&app->ring));
        atomic_store(&app->metrics.storage_pending_count,
                     (unsigned int)storage_pending_telemetry_count(app->storage));

        if (edgeflow_now_ms() - last_metrics_ms > 1000U) {
            update_heartbeat_metrics(app);
            (void)metrics_write_file(app->cfg.metrics_path, &app->metrics);
            last_metrics_ms = edgeflow_now_ms();
        }
    }

    update_heartbeat_metrics(app);
    (void)metrics_write_file(app->cfg.metrics_path, &app->metrics);
    return NULL;
}

static void monitor_on_timer(void *user_data)
{
    gateway_app_t *app = user_data;
    char hb_payload[128];
    uint64_t ingress_age;
    uint64_t worker_age;

    if (app == NULL || !atomic_load(&app->running)) {
        return;
    }

    heartbeat_touch(&app->heartbeat, HEARTBEAT_THREAD_MONITOR);
    watchdog_notify_alive();

    ingress_age = heartbeat_age_ms(&app->heartbeat, HEARTBEAT_THREAD_INGRESS);
    worker_age = heartbeat_age_ms(&app->heartbeat, HEARTBEAT_THREAD_WORKER);
    if (ingress_age > app->cfg.heartbeat_interval_ms * 3U) {
        log_warn("ingress heartbeat stale: age_ms=%llu", (unsigned long long)ingress_age);
    }
    if (worker_age > app->cfg.heartbeat_interval_ms * 3U) {
        log_warn("worker heartbeat stale: age_ms=%llu", (unsigned long long)worker_age);
    }

    (void)mqtt_session_replay_pending(&app->mqtt, app->storage, &app->metrics);
    (void)mqtt_session_ping(&app->mqtt);

    snprintf(hb_payload,
             sizeof(hb_payload),
             "{\"device_id\":\"%s\",\"ts_ms\":%llu,\"pending\":%u}",
             app->cfg.device_id,
             (unsigned long long)edgeflow_now_ms(),
             (unsigned int)storage_pending_telemetry_count(app->storage));
    (void)mqtt_session_publish_heartbeat(&app->mqtt, hb_payload);

    update_heartbeat_metrics(app);
    (void)metrics_write_file(app->cfg.metrics_path, &app->metrics);
}

static void *monitor_main(void *arg)
{
    gateway_app_t *app = arg;
    reactor_t *reactor = NULL;

    if (reactor_init(&reactor) != 0) {
        log_warn("reactor init failed, monitor thread fallback sleep loop");
        while (atomic_load(&app->running)) {
            monitor_on_timer(app);
            edgeflow_sleep_ms(app->cfg.watchdog_interval_ms);
        }
        return NULL;
    }

    if (reactor_add_timer_ms(reactor, app->cfg.watchdog_interval_ms, monitor_on_timer, app) != 0) {
        log_warn("reactor timer setup failed");
        reactor_destroy(reactor);
        while (atomic_load(&app->running)) {
            monitor_on_timer(app);
            edgeflow_sleep_ms(app->cfg.watchdog_interval_ms);
        }
        return NULL;
    }

    while (atomic_load(&app->running)) {
        if (reactor_run_once(reactor, 500) != 0) {
            break;
        }
    }

    reactor_stop(reactor);
    reactor_destroy(reactor);
    return NULL;
}

int gateway_app_init(gateway_app_t *app, const gateway_config_t *cfg)
{
    device_adapter_t modbus;

    if (app == NULL || cfg == NULL) {
        return -1;
    }

    memset(app, 0, sizeof(*app));
    app->cfg = *cfg;
    state_engine_init(&app->state_engine);
    command_scheduler_init(&app->command_scheduler);
    metrics_init(&app->metrics);
    heartbeat_registry_init(&app->heartbeat);
    mqtt_session_init(&app->mqtt, cfg);
    atomic_init(&app->running, false);

    if (storage_open(&app->storage, cfg) != 0) {
        log_error("storage_open failed");
        return -1;
    }

    adapter_registry_init(&app->registry);
    modbus_adapter_fill(&modbus);
    if (adapter_registry_register(&app->registry, &modbus) != ADAPTER_OK) {
        storage_close(app->storage);
        app->storage = NULL;
        return -1;
    }
    if (adapter_registry_init_all(&app->registry, cfg) != 0) {
        log_warn("adapter init reported failures");
    }

    if (spsc_ring_init(&app->ring, cfg->queue_size) != 0) {
        adapter_registry_shutdown_all(&app->registry);
        storage_close(app->storage);
        app->storage = NULL;
        return -1;
    }

    watchdog_init(cfg->watchdog_interval_ms);
    return 0;
}

int gateway_app_run(gateway_app_t *app)
{
    if (app == NULL) {
        return -1;
    }

    atomic_store(&app->running, true);
    if (pthread_create(&app->ingress_thread, NULL, ingress_main, app) != 0) {
        atomic_store(&app->running, false);
        return -1;
    }
    if (pthread_create(&app->worker_thread, NULL, worker_main, app) != 0) {
        atomic_store(&app->running, false);
        pthread_join(app->ingress_thread, NULL);
        return -1;
    }
    if (pthread_create(&app->monitor_thread, NULL, monitor_main, app) != 0) {
        atomic_store(&app->running, false);
        pthread_join(app->ingress_thread, NULL);
        pthread_join(app->worker_thread, NULL);
        return -1;
    }

    watchdog_notify_ready();
    log_info("edgeflow running (advanced): device_id=%s simulate=%s sqlite=%s",
             app->cfg.device_id,
             app->cfg.simulate ? "true" : "false",
             app->cfg.use_sqlite ? app->cfg.sqlite_path : "disabled");
    return 0;
}

void gateway_app_stop(gateway_app_t *app)
{
    if (app == NULL) {
        return;
    }
    atomic_store(&app->running, false);
    pthread_join(app->ingress_thread, NULL);
    pthread_join(app->worker_thread, NULL);
    pthread_join(app->monitor_thread, NULL);
    mqtt_session_shutdown(&app->mqtt);
    watchdog_shutdown();
    log_info("edgeflow stopped");
}

void gateway_app_destroy(gateway_app_t *app)
{
    if (app == NULL) {
        return;
    }
    adapter_registry_shutdown_all(&app->registry);
    spsc_ring_destroy(&app->ring);
    if (app->storage != NULL) {
        storage_close(app->storage);
        app->storage = NULL;
    }
}
