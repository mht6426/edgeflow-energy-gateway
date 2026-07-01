#ifndef EDGEFLOW_CONFIG_H
#define EDGEFLOW_CONFIG_H

/*
 * Config Manager — 网关运行参数（基础版 + 进阶版字段）
 *
 * 平台：Linux
 */

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
    char mqtt_alarm_topic[128];
    char mqtt_heartbeat_topic[128];

    char log_dir[128];
    char metrics_path[128];
    char cache_path[128];

    /* 进阶版 */
    bool use_sqlite;
    char sqlite_path[128];
    uint32_t mqtt_replay_batch;
    uint32_t mqtt_keepalive_sec;
    uint32_t watchdog_interval_ms;
    uint32_t heartbeat_interval_ms;

    /* Modbus（libmodbus 后端） */
    char modbus_transport[8];   /* "rtu" | "tcp" */
    char modbus_tcp_host[128];
    uint16_t modbus_tcp_port;
    uint8_t modbus_slave_id;
    uint32_t modbus_baud_rate;
    char modbus_parity;
    uint8_t modbus_data_bits;
    uint8_t modbus_stop_bits;
    uint32_t modbus_timeout_ms;
} gateway_config_t;

void gateway_config_defaults(gateway_config_t *cfg);
int gateway_config_load(const char *path, gateway_config_t *cfg);

#endif
