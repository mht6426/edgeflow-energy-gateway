#include "platform/config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void copy_string(char *dst, size_t dst_len, const char *src)
{
    if (dst_len == 0U) {
        return;
    }
    snprintf(dst, dst_len, "%s", src);
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

static void json_get_string(const char *json, const char *key, char *dst, size_t dst_len)
{
    char pattern[64];
    char *p;
    char *start;
    char *end;
    size_t len;

    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    p = strstr(json, pattern);
    if (p == NULL) {
        return;
    }
    p = strchr(p + strlen(pattern), ':');
    if (p == NULL) {
        return;
    }
    start = strchr(p, '"');
    if (start == NULL) {
        return;
    }
    start++;
    end = strchr(start, '"');
    if (end == NULL) {
        return;
    }
    len = (size_t)(end - start);
    if (len >= dst_len) {
        len = dst_len - 1U;
    }
    memcpy(dst, start, len);
    dst[len] = '\0';
}

static void json_get_u32(const char *json, const char *key, uint32_t *value)
{
    char pattern[64];
    char *p;
    unsigned long parsed;

    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    p = strstr(json, pattern);
    if (p == NULL) {
        return;
    }
    p = strchr(p + strlen(pattern), ':');
    if (p == NULL) {
        return;
    }
    parsed = strtoul(p + 1, NULL, 10);
    if (parsed <= UINT32_MAX) {
        *value = (uint32_t)parsed;
    }
}

static void json_get_double(const char *json, const char *key, double *value)
{
    char pattern[64];
    char *p;

    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    p = strstr(json, pattern);
    if (p == NULL) {
        return;
    }
    p = strchr(p + strlen(pattern), ':');
    if (p != NULL) {
        *value = strtod(p + 1, NULL);
    }
}

static void json_get_bool(const char *json, const char *key, bool *value)
{
    char pattern[64];
    char *p;

    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    p = strstr(json, pattern);
    if (p == NULL) {
        return;
    }
    p = strchr(p + strlen(pattern), ':');
    if (p == NULL) {
        return;
    }
    while (*p == ':' || *p == ' ' || *p == '\t') {
        p++;
    }
    if (strncmp(p, "true", 4U) == 0) {
        *value = true;
    } else if (strncmp(p, "false", 5U) == 0) {
        *value = false;
    }
}

void gateway_config_defaults(gateway_config_t *cfg)
{
    memset(cfg, 0, sizeof(*cfg));
    copy_string(cfg->device_id, sizeof(cfg->device_id), "gw-001");
    cfg->simulate = true;
    copy_string(cfg->serial_device, sizeof(cfg->serial_device), "/dev/ttyUSB0");
    cfg->poll_interval_ms = 500U;
    cfg->queue_size = 4096U;
    cfg->deadband = 0.1;
    cfg->temp_high = 60.0;
    cfg->peak_shaving_threshold_kw = 120.0;
    cfg->max_discharge_kw = 50.0;
    cfg->min_discharge_soc = 30.0;
    copy_string(cfg->broker_host, sizeof(cfg->broker_host), "127.0.0.1");
    cfg->broker_port = 1883U;
    copy_string(cfg->mqtt_topic, sizeof(cfg->mqtt_topic), "plant/line1/gw-001/telemetry");
    copy_string(cfg->log_dir, sizeof(cfg->log_dir), "/tmp/edgeflow");
    copy_string(cfg->metrics_path, sizeof(cfg->metrics_path), "/tmp/edgeflow/metrics.prom");
    copy_string(cfg->cache_path, sizeof(cfg->cache_path), "/tmp/edgeflow/offline-cache.jsonl");
}

int gateway_config_load(const char *path, gateway_config_t *cfg)
{
    char *json = NULL;
    uint32_t port;

    gateway_config_defaults(cfg);
    if (path == NULL) {
        return 0;
    }
    if (read_file(path, &json) != 0) {
        return -1;
    }

    json_get_string(json, "device_id", cfg->device_id, sizeof(cfg->device_id));
    json_get_bool(json, "simulate", &cfg->simulate);
    json_get_string(json, "serial_device", cfg->serial_device, sizeof(cfg->serial_device));
    json_get_u32(json, "poll_interval_ms", &cfg->poll_interval_ms);
    json_get_u32(json, "queue_size", &cfg->queue_size);
    json_get_double(json, "deadband", &cfg->deadband);
    json_get_double(json, "temp_high", &cfg->temp_high);
    json_get_double(json, "peak_shaving_threshold_kw", &cfg->peak_shaving_threshold_kw);
    json_get_double(json, "max_discharge_kw", &cfg->max_discharge_kw);
    json_get_double(json, "min_discharge_soc", &cfg->min_discharge_soc);
    json_get_string(json, "broker_host", cfg->broker_host, sizeof(cfg->broker_host));
    port = cfg->broker_port;
    json_get_u32(json, "broker_port", &port);
    cfg->broker_port = (uint16_t)port;
    json_get_string(json, "mqtt_topic", cfg->mqtt_topic, sizeof(cfg->mqtt_topic));
    json_get_string(json, "log_dir", cfg->log_dir, sizeof(cfg->log_dir));
    json_get_string(json, "metrics_path", cfg->metrics_path, sizeof(cfg->metrics_path));
    json_get_string(json, "cache_path", cfg->cache_path, sizeof(cfg->cache_path));

    free(json);
    return 0;
}
