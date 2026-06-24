#include "platform/metrics.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

static void ensure_parent_dir(const char *path)
{
    char buf[256];
    char *slash;

    snprintf(buf, sizeof(buf), "%s", path);
    slash = strrchr(buf, '/');
    if (slash == NULL || slash == buf) {
        return;
    }
    *slash = '\0';
    (void)mkdir(buf, 0755);
}

void metrics_init(gateway_metrics_t *metrics)
{
    atomic_init(&metrics->ingress_points, 0ULL);
    atomic_init(&metrics->processed_points, 0ULL);
    atomic_init(&metrics->mqtt_publish_ok, 0ULL);
    atomic_init(&metrics->mqtt_publish_fail, 0ULL);
    atomic_init(&metrics->queue_depth, 0U);
    atomic_init(&metrics->queue_dropped, 0U);
}

int metrics_write_file(const char *path, const gateway_metrics_t *metrics)
{
    FILE *fp;

    ensure_parent_dir(path);
    fp = fopen(path, "w");
    if (fp == NULL) {
        return -1;
    }

    fprintf(fp, "edgeflow_ingress_points_total %llu\n",
            (unsigned long long)atomic_load(&metrics->ingress_points));
    fprintf(fp, "edgeflow_processed_points_total %llu\n",
            (unsigned long long)atomic_load(&metrics->processed_points));
    fprintf(fp, "edgeflow_mqtt_publish_ok_total %llu\n",
            (unsigned long long)atomic_load(&metrics->mqtt_publish_ok));
    fprintf(fp, "edgeflow_mqtt_publish_fail_total %llu\n",
            (unsigned long long)atomic_load(&metrics->mqtt_publish_fail));
    fprintf(fp, "edgeflow_queue_depth %u\n", atomic_load(&metrics->queue_depth));
    fprintf(fp, "edgeflow_queue_dropped_total %u\n", atomic_load(&metrics->queue_dropped));

    fclose(fp);
    return 0;
}
