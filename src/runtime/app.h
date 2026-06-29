#ifndef EDGEFLOW_APP_H
#define EDGEFLOW_APP_H

/*
 * 控制器运行时（进阶版）
 *
 * 三线程模型：
 *   ingress_thread — Adapter poll → SPSC push
 *   worker_thread  — pop → SQLite/JSONL → state_engine → scheduler → MQTT
 *   monitor_thread — epoll 定时器 → watchdog / 心跳 / MQTT 补传 / PING
 */

#include "core/ring.h"
#include "egress/mqtt_session.h"
#include "ingress/adapter.h"
#include "platform/config.h"
#include "platform/heartbeat.h"
#include "platform/metrics.h"
#include "platform/storage.h"
#include "runtime/command_scheduler.h"
#include "runtime/state_engine.h"

#include <pthread.h>
#include <stdatomic.h>

typedef struct {
    gateway_config_t cfg;
    adapter_registry_t registry;
    spsc_ring_t ring;
    state_engine_t state_engine;
    command_scheduler_t command_scheduler;
    gateway_metrics_t metrics;
    storage_ctx_t *storage;
    mqtt_session_t mqtt;
    heartbeat_registry_t heartbeat;
    atomic_bool running;
    pthread_t ingress_thread;
    pthread_t worker_thread;
    pthread_t monitor_thread;
} gateway_app_t;

int gateway_app_init(gateway_app_t *app, const gateway_config_t *cfg);
int gateway_app_run(gateway_app_t *app);
void gateway_app_stop(gateway_app_t *app);
void gateway_app_destroy(gateway_app_t *app);

#endif
