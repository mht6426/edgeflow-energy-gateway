/*
 * metrics.c — Prometheus 文本文件指标
 */

#include "platform/metrics.h"

#include <stdio.h>

void metrics_init(gateway_metrics_t *metrics)
{
    if (metrics == NULL) {
        return;
    }
    atomic_init(&metrics->ingress_points, 0ULL);
    atomic_init(&metrics->processed_points, 0ULL);
    atomic_init(&metrics->mqtt_publish_ok, 0ULL);
    atomic_init(&metrics->mqtt_publish_fail, 0ULL);
    atomic_init(&metrics->mqtt_replay_total, 0ULL);
    atomic_init(&metrics->queue_depth, 0U);
    atomic_init(&metrics->queue_dropped, 0U);
    atomic_init(&metrics->storage_pending_count, 0U);
    atomic_init(&metrics->heartbeat_ingress_age_ms, 0U);
    atomic_init(&metrics->heartbeat_worker_age_ms, 0U);
    atomic_init(&metrics->heartbeat_monitor_age_ms, 0U);
}

int metrics_write_file(const char *path, const gateway_metrics_t *metrics)
{
    FILE *fp;

    if (path == NULL || metrics == NULL) {
        return -1;
    }
    fp = fopen(path, "w");
    if (fp == NULL) {
        return -1;
    }

    fprintf(fp, "# HELP edgeflow_ingress_points_total Total ingress telemetry points\n");
    fprintf(fp, "edgeflow_ingress_points_total %llu\n",
            (unsigned long long)atomic_load(&metrics->ingress_points));
    fprintf(fp, "edgeflow_processed_points_total %llu\n",
            (unsigned long long)atomic_load(&metrics->processed_points));
    fprintf(fp, "edgeflow_mqtt_publish_ok_total %llu\n",
            (unsigned long long)atomic_load(&metrics->mqtt_publish_ok));
    fprintf(fp, "edgeflow_mqtt_publish_fail_total %llu\n",
            (unsigned long long)atomic_load(&metrics->mqtt_publish_fail));
    fprintf(fp, "edgeflow_mqtt_replay_total %llu\n",
            (unsigned long long)atomic_load(&metrics->mqtt_replay_total));
    fprintf(fp, "edgeflow_queue_depth %u\n", atomic_load(&metrics->queue_depth));
    fprintf(fp, "edgeflow_queue_dropped_total %u\n", atomic_load(&metrics->queue_dropped));
    fprintf(fp, "edgeflow_storage_pending_count %u\n", atomic_load(&metrics->storage_pending_count));
    fprintf(fp, "edgeflow_heartbeat_ingress_age_ms %u\n",
            atomic_load(&metrics->heartbeat_ingress_age_ms));
    fprintf(fp, "edgeflow_heartbeat_worker_age_ms %u\n",
            atomic_load(&metrics->heartbeat_worker_age_ms));
    fprintf(fp, "edgeflow_heartbeat_monitor_age_ms %u\n",
            atomic_load(&metrics->heartbeat_monitor_age_ms));

    fclose(fp);
    return 0;
}
