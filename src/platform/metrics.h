#ifndef EDGEFLOW_METRICS_H
#define EDGEFLOW_METRICS_H

/*
 * Prometheus 文本格式指标（基础 + 进阶）
 */

#include <stdatomic.h>
#include <stdint.h>

typedef struct {
    atomic_ullong ingress_points;
    atomic_ullong processed_points;
    atomic_ullong mqtt_publish_ok;
    atomic_ullong mqtt_publish_fail;
    atomic_ullong mqtt_replay_total;
    atomic_uint queue_depth;
    atomic_uint queue_dropped;
    atomic_uint storage_pending_count;
    atomic_uint heartbeat_ingress_age_ms;
    atomic_uint heartbeat_worker_age_ms;
    atomic_uint heartbeat_monitor_age_ms;
} gateway_metrics_t;

void metrics_init(gateway_metrics_t *metrics);
int metrics_write_file(const char *path, const gateway_metrics_t *metrics);

#endif
