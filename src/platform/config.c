/*
 * config.c — JSON 配置加载（cJSON）
 *
 * 平台：Linux
 * 字段缺失时保留 gateway_config_defaults 的值。
 */

#include "platform/config.h"

#include <cJSON.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void copy_string(char *dst, size_t dst_len, const char *src)
{
    if (dst_len == 0U) {
        return;
    }
    snprintf(dst, dst_len, "%s", src != NULL ? src : "");
}

static int read_file(const char *path, char **out)
{
    FILE *fp = fopen(path, "rb");
    long size;
    char *buf;

    if (fp == NULL) {
        return -1;
    }
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return -1;
    }
    size = ftell(fp);
    if (size < 0 || fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return -1;
    }
    buf = calloc((size_t)size + 1U, 1U);
    if (buf == NULL) {
        fclose(fp);
        return -1;
    }
    if (fread(buf, 1U, (size_t)size, fp) != (size_t)size) {
        free(buf);
        fclose(fp);
        return -1;
    }
    fclose(fp);
    *out = buf;
    return 0;
}

static void json_copy_string(const cJSON *root, const char *key, char *dst, size_t dst_len)
{
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(root, key);

    if (cJSON_IsString(item) && item->valuestring != NULL) {
        copy_string(dst, dst_len, item->valuestring);
    }
}

static void json_copy_bool(const cJSON *root, const char *key, bool *dst)
{
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(root, key);

    if (cJSON_IsBool(item)) {
        *dst = cJSON_IsTrue(item);
    }
}

static void json_copy_uint32(const cJSON *root, const char *key, uint32_t *dst)
{
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(root, key);

    if (cJSON_IsNumber(item)) {
        *dst = (uint32_t)item->valuedouble;
    }
}

static void json_copy_uint16(const cJSON *root, const char *key, uint16_t *dst)
{
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(root, key);

    if (cJSON_IsNumber(item)) {
        *dst = (uint16_t)item->valuedouble;
    }
}

static void json_copy_uint8(const cJSON *root, const char *key, uint8_t *dst)
{
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(root, key);

    if (cJSON_IsNumber(item)) {
        *dst = (uint8_t)item->valuedouble;
    }
}

static void json_copy_double(const cJSON *root, const char *key, double *dst)
{
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(root, key);

    if (cJSON_IsNumber(item)) {
        *dst = item->valuedouble;
    }
}

static void json_copy_parity(const cJSON *root, const char *key, char *dst)
{
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(root, key);

    if (cJSON_IsString(item) && item->valuestring != NULL && item->valuestring[0] != '\0') {
        *dst = item->valuestring[0];
    }
}

void gateway_config_defaults(gateway_config_t *cfg)
{
    if (cfg == NULL) {
        return;
    }
    memset(cfg, 0, sizeof(*cfg));

    copy_string(cfg->device_id, sizeof(cfg->device_id), "gw-dev");
    cfg->simulate = true;
    copy_string(cfg->serial_device, sizeof(cfg->serial_device), "/dev/ttyUSB0");
    cfg->poll_interval_ms = 1000U;
    cfg->queue_size = 1024U;
    cfg->deadband = 0.1;
    cfg->temp_high = 55.0;
    cfg->peak_shaving_threshold_kw = 100.0;
    cfg->max_discharge_kw = 30.0;
    cfg->min_discharge_soc = 20.0;
    copy_string(cfg->broker_host, sizeof(cfg->broker_host), "127.0.0.1");
    cfg->broker_port = 1883U;
    copy_string(cfg->mqtt_topic, sizeof(cfg->mqtt_topic), "edgeflow/telemetry");
    copy_string(cfg->mqtt_alarm_topic, sizeof(cfg->mqtt_alarm_topic), "edgeflow/alarm");
    copy_string(cfg->mqtt_heartbeat_topic, sizeof(cfg->mqtt_heartbeat_topic), "edgeflow/heartbeat");
    copy_string(cfg->log_dir, sizeof(cfg->log_dir), "/tmp/edgeflow");
    copy_string(cfg->metrics_path, sizeof(cfg->metrics_path), "/tmp/edgeflow/metrics.prom");
    copy_string(cfg->cache_path, sizeof(cfg->cache_path), "/tmp/edgeflow/offline-cache.jsonl");
    cfg->use_sqlite = true;
    copy_string(cfg->sqlite_path, sizeof(cfg->sqlite_path), "/tmp/edgeflow/edgeflow.db");
    cfg->mqtt_replay_batch = 50U;
    cfg->mqtt_keepalive_sec = 60U;
    cfg->watchdog_interval_ms = 2000U;
    cfg->heartbeat_interval_ms = 5000U;

    copy_string(cfg->modbus_transport, sizeof(cfg->modbus_transport), "rtu");
    copy_string(cfg->modbus_tcp_host, sizeof(cfg->modbus_tcp_host), "127.0.0.1");
    cfg->modbus_tcp_port = 502U;
    cfg->modbus_slave_id = 1U;
    cfg->modbus_baud_rate = 9600U;
    cfg->modbus_parity = 'N';
    cfg->modbus_data_bits = 8U;
    cfg->modbus_stop_bits = 1U;
    cfg->modbus_timeout_ms = 500U;
}

int gateway_config_load(const char *path, gateway_config_t *cfg)
{
    char *text = NULL;
    cJSON *root = NULL;
    int rc = -1;

    if (path == NULL || cfg == NULL) {
        return -1;
    }

    gateway_config_defaults(cfg);
    if (read_file(path, &text) != 0) {
        return -1;
    }

    root = cJSON_Parse(text);
    free(text);
    if (root == NULL || !cJSON_IsObject(root)) {
        cJSON_Delete(root);
        return -1;
    }

    json_copy_string(root, "device_id", cfg->device_id, sizeof(cfg->device_id));
    json_copy_bool(root, "simulate", &cfg->simulate);
    json_copy_string(root, "serial_device", cfg->serial_device, sizeof(cfg->serial_device));
    json_copy_uint32(root, "poll_interval_ms", &cfg->poll_interval_ms);
    json_copy_uint32(root, "queue_size", &cfg->queue_size);
    json_copy_double(root, "deadband", &cfg->deadband);
    json_copy_double(root, "temp_high", &cfg->temp_high);
    json_copy_double(root, "peak_shaving_threshold_kw", &cfg->peak_shaving_threshold_kw);
    json_copy_double(root, "max_discharge_kw", &cfg->max_discharge_kw);
    json_copy_double(root, "min_discharge_soc", &cfg->min_discharge_soc);
    json_copy_string(root, "broker_host", cfg->broker_host, sizeof(cfg->broker_host));
    json_copy_uint16(root, "broker_port", &cfg->broker_port);
    json_copy_string(root, "mqtt_topic", cfg->mqtt_topic, sizeof(cfg->mqtt_topic));
    json_copy_string(root, "mqtt_alarm_topic", cfg->mqtt_alarm_topic, sizeof(cfg->mqtt_alarm_topic));
    json_copy_string(root, "mqtt_heartbeat_topic", cfg->mqtt_heartbeat_topic, sizeof(cfg->mqtt_heartbeat_topic));
    json_copy_string(root, "log_dir", cfg->log_dir, sizeof(cfg->log_dir));
    json_copy_string(root, "metrics_path", cfg->metrics_path, sizeof(cfg->metrics_path));
    json_copy_string(root, "cache_path", cfg->cache_path, sizeof(cfg->cache_path));
    json_copy_string(root, "sqlite_path", cfg->sqlite_path, sizeof(cfg->sqlite_path));
    json_copy_bool(root, "use_sqlite", &cfg->use_sqlite);
    json_copy_uint32(root, "mqtt_replay_batch", &cfg->mqtt_replay_batch);
    json_copy_uint32(root, "mqtt_keepalive_sec", &cfg->mqtt_keepalive_sec);
    json_copy_uint32(root, "watchdog_interval_ms", &cfg->watchdog_interval_ms);
    json_copy_uint32(root, "heartbeat_interval_ms", &cfg->heartbeat_interval_ms);

    json_copy_string(root, "modbus_transport", cfg->modbus_transport, sizeof(cfg->modbus_transport));
    json_copy_string(root, "modbus_tcp_host", cfg->modbus_tcp_host, sizeof(cfg->modbus_tcp_host));
    json_copy_uint16(root, "modbus_tcp_port", &cfg->modbus_tcp_port);
    json_copy_uint8(root, "modbus_slave_id", &cfg->modbus_slave_id);
    json_copy_uint32(root, "modbus_baud_rate", &cfg->modbus_baud_rate);
    json_copy_parity(root, "modbus_parity", &cfg->modbus_parity);
    json_copy_uint8(root, "modbus_data_bits", &cfg->modbus_data_bits);
    json_copy_uint8(root, "modbus_stop_bits", &cfg->modbus_stop_bits);
    json_copy_uint32(root, "modbus_timeout_ms", &cfg->modbus_timeout_ms);

    cJSON_Delete(root);
    rc = 0;
    return rc;
}
