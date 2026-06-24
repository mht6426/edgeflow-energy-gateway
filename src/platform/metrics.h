#ifndef EDGEFLOW_METRICS_H
#define EDGEFLOW_METRICS_H

#include <stdatomic.h>
#include <stdint.h>

typedef struct {
    atomic_ullong ingress_points;
    atomic_ullong processed_points;
    atomic_ullong mqtt_publish_ok;
    atomic_ullong mqtt_publish_fail;
    atomic_uint queue_depth;
    atomic_uint queue_dropped;
} gateway_metrics_t;

void metrics_init(gateway_metrics_t *metrics);
int metrics_write_file(const char *path, const gateway_metrics_t *metrics);

#endif
