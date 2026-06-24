#ifndef EDGEFLOW_CONFIG_H
#define EDGEFLOW_CONFIG_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    char device_id[32];
    bool simulate;
    char serial_device[128];
    uint32_t poll_interval_ms;
    uint32_t queue_size;
    double deadband;
    double temp_high;
    double peak_shaving_threshold_kw;
    double max_discharge_kw;
    double min_discharge_soc;
    char broker_host[128];
    uint16_t broker_port;
    char mqtt_topic[128];
    char log_dir[128];
    char metrics_path[128];
    char cache_path[128];
} gateway_config_t;

void gateway_config_defaults(gateway_config_t *cfg);
int gateway_config_load(const char *path, gateway_config_t *cfg);

#endif
