#ifndef EDGEFLOW_APP_H
#define EDGEFLOW_APP_H

#include "core/ring.h"
#include "platform/config.h"
#include "platform/metrics.h"
#include "runtime/control_loop.h"

#include <stdatomic.h>
#include <pthread.h>

typedef struct {
    gateway_config_t cfg;
    spsc_ring_t ring;
    control_loop_t control_loop;
    gateway_metrics_t metrics;
    atomic_bool running;
    pthread_t ingress_thread;
    pthread_t worker_thread;
} gateway_app_t;

int gateway_app_init(gateway_app_t *app, const gateway_config_t *cfg);
int gateway_app_run(gateway_app_t *app);
void gateway_app_stop(gateway_app_t *app);
void gateway_app_destroy(gateway_app_t *app);

#endif
