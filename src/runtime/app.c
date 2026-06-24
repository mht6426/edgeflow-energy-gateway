#include "runtime/app.h"

#include "common/time_util.h"
#include "egress/mqtt_client.h"
#include "ingress/modbus_rtu.h"
#include "platform/logger.h"
#include "platform/storage.h"

#include <stdio.h>
#include <string.h>

static void apply_rules(const gateway_config_t *cfg, data_point_t *point)
{
    if (strcmp(point->point_id, "max_cell_temp_c") == 0 && point->value > cfg->temp_high) {
        point->quality = TELEMETRY_QUALITY_BAD;
    }
}

static void *ingress_main(void *arg)
{
    gateway_app_t *app = arg;

    while (atomic_load(&app->running)) {
        data_point_t points[8];
        int n = modbus_rtu_poll(&app->cfg, points, 8U);
        if (n > 0) {
            for (int i = 0; i < n; i++) {
                if (spsc_ring_push(&app->ring, &points[i])) {
                    atomic_fetch_add(&app->metrics.ingress_points, 1ULL);
                }
            }
        }
        atomic_store(&app->metrics.queue_depth, spsc_ring_count(&app->ring));
        atomic_store(&app->metrics.queue_dropped, spsc_ring_dropped(&app->ring));
        edgeflow_sleep_ms(app->cfg.poll_interval_ms);
    }
    return NULL;
}

static void *worker_main(void *arg)
{
    gateway_app_t *app = arg;
    uint64_t last_metrics_ms = 0U;

    while (atomic_load(&app->running)) {
        data_point_t point;
        if (!spsc_ring_pop(&app->ring, &point)) {
            edgeflow_sleep_ms(20U);
            continue;
        }

        apply_rules(&app->cfg, &point);
        if (storage_append_point(app->cfg.cache_path, &point) != 0) {
            log_warn("failed to append telemetry cache: path=%s", app->cfg.cache_path);
        }
        atomic_fetch_add(&app->metrics.processed_points, 1ULL);

        alarm_event_t alarm = {0};
        command_t command = {0};
        if (control_loop_process_telemetry(&app->control_loop, &app->cfg, &point, &alarm, &command)) {
            if (alarm.alarm_id != 0U) {
                if (storage_append_alarm(app->cfg.cache_path, &alarm) != 0) {
                    log_warn("failed to append alarm cache: path=%s", app->cfg.cache_path);
                }
                log_warn("alarm active: device=%s code=%s message=%s",
                         alarm.device_id,
                         alarm.code,
                         alarm.message);
            } else if (command.command_id != 0U) {
                if (storage_append_command(app->cfg.cache_path, &command) != 0) {
                    log_warn("failed to append command cache: path=%s", app->cfg.cache_path);
                }
                log_info("command verified: id=%llu target=%s point=%s value=%.3f state=%s",
                         (unsigned long long)command.command_id,
                         command.target_device_id,
                         command.point_id,
                         command.target_value,
                         command_state_to_string(command.state));
            }
        }

        if (mqtt_publish_point(&app->cfg, &point) == 0) {
            atomic_fetch_add(&app->metrics.mqtt_publish_ok, 1ULL);
        } else {
            atomic_fetch_add(&app->metrics.mqtt_publish_fail, 1ULL);
        }

        atomic_store(&app->metrics.queue_depth, spsc_ring_count(&app->ring));
        atomic_store(&app->metrics.queue_dropped, spsc_ring_dropped(&app->ring));

        if (edgeflow_now_ms() - last_metrics_ms > 1000U) {
            if (metrics_write_file(app->cfg.metrics_path, &app->metrics) != 0) {
                log_warn("failed to write metrics: path=%s", app->cfg.metrics_path);
            }
            last_metrics_ms = edgeflow_now_ms();
        }
    }
    if (metrics_write_file(app->cfg.metrics_path, &app->metrics) != 0) {
        log_warn("failed to write final metrics: path=%s", app->cfg.metrics_path);
    }
    return NULL;
}

int gateway_app_init(gateway_app_t *app, const gateway_config_t *cfg)
{
    memset(app, 0, sizeof(*app));
    app->cfg = *cfg;
    control_loop_init(&app->control_loop);
    metrics_init(&app->metrics);
    atomic_init(&app->running, false);
    if (spsc_ring_init(&app->ring, cfg->queue_size) != 0) {
        return -1;
    }
    return 0;
}

int gateway_app_run(gateway_app_t *app)
{
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

    log_info("edgeflow started: device_id=%s broker=%s:%u simulate=%s",
             app->cfg.device_id,
             app->cfg.broker_host,
             app->cfg.broker_port,
             app->cfg.simulate ? "true" : "false");
    return 0;
}

void gateway_app_stop(gateway_app_t *app)
{
    atomic_store(&app->running, false);
    pthread_join(app->ingress_thread, NULL);
    pthread_join(app->worker_thread, NULL);
    log_info("edgeflow stopped");
}

void gateway_app_destroy(gateway_app_t *app)
{
    spsc_ring_destroy(&app->ring);
}
