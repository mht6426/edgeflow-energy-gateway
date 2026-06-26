/*
 * config.c — JSON 配置加载实现（M2）
 *
 * 平台：Linux（fopen、路径分隔符 /）
 *
 * 解析策略：
 *   轻量手写解析，无 cJSON 等第三方库；仅支持本项目 gateway.json 的扁平键值。
 *   不支持嵌套对象、数组、注释。字段缺失时保留 gateway_config_defaults 的值。
 *
 * 局限（已知，M10 可改进）：
 *   - 无 schema 校验、无字段范围检查
 *   - 键名子串误匹配理论风险（当前键名互不包含，可接受）
 *
 * 不负责：热更新、环境变量覆盖、加密字段。
 */

#include "platform/config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 安全字符串拷贝，保证 NUL 终止 */
static void copy_string(char *dst, size_t dst_len, const char *src)
{
    if (dst_len == 0U) {
        return;
    }
    snprintf(dst, dst_len, "%s", src != NULL ? src : "");
}

/*
 * 将整个配置文件读入堆内存（调用方 free）。
 * 使用二进制模式 "rb" 避免 Windows 换行干扰（本工程以 Linux 为主，仍保持可移植读取）。
 */
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

/*
 * 从扁平 JSON 文本提取字符串字段："key": "value"
 * 未找到或格式不符时 dst 保持不变。
 */
static void json_get_string(const char *json, const char *key, char *dst, size_t dst_len)
{
    char pattern[64];
    const char *start;
    const char *end;

    if (json == NULL || key == NULL || dst == NULL || dst_len == 0U) {
        return;
    }
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    start = strstr(json, pattern);
    if (start == NULL) {
        return;
    }
    start = strchr(start, ':');
    if (start == NULL) {
        return;
    }
    start++;
    while (*start == ' ' || *start == '\t') {
        start++;
    }
    if (*start != '"') {
        return;
    }
    start++;
    end = strchr(start, '"');
    if (end == NULL) {
        return;
    }
    snprintf(dst, dst_len, "%.*s", (int)(end - start), start);
}

/* 解析 "key": true|false；找到并解析成功返回 true */
static bool json_get_bool(const char *json, const char *key, bool *out)
{
    char pattern[64];
    const char *pos;

    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    pos = strstr(json, pattern);
    if (pos == NULL) {
        return false;
    }
    pos = strchr(pos, ':');
    if (pos == NULL) {
        return false;
    }
    pos++;
    while (*pos == ' ' || *pos == '\t') {
        pos++;
    }
    if (strncmp(pos, "true", 4) == 0) {
        *out = true;
        return true;
    }
    if (strncmp(pos, "false", 5) == 0) {
        *out = false;
        return true;
    }
    return false;
}

/* 解析无符号整数 "key": 123 */
static bool json_get_uint(const char *json, const char *key, uint32_t *out)
{
    char pattern[64];
    const char *pos;

    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    pos = strstr(json, pattern);
    if (pos == NULL) {
        return false;
    }
    pos = strchr(pos, ':');
    if (pos == NULL) {
        return false;
    }
    pos++;
    while (*pos == ' ' || *pos == '\t') {
        pos++;
    }
    *out = (uint32_t)strtoul(pos, NULL, 10);
    return true;
}

/* 解析浮点 "key": 60.0 */
static bool json_get_double(const char *json, const char *key, double *out)
{
    char pattern[64];
    const char *pos;

    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    pos = strstr(json, pattern);
    if (pos == NULL) {
        return false;
    }
    pos = strchr(pos, ':');
    if (pos == NULL) {
        return false;
    }
    pos++;
    while (*pos == ' ' || *pos == '\t') {
        pos++;
    }
    *out = strtod(pos, NULL);
    return true;
}

void gateway_config_defaults(gateway_config_t *cfg)
{
    if (cfg == NULL) {
        return;
    }
    memset(cfg, 0, sizeof(*cfg));

    /* 与 configs/gateway.json 示例对齐，便于开发机 /tmp 下零配置试运行 */
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
    copy_string(cfg->log_dir, sizeof(cfg->log_dir), "/tmp/edgeflow");
    copy_string(cfg->metrics_path, sizeof(cfg->metrics_path), "/tmp/edgeflow/metrics.prom");
    copy_string(cfg->cache_path, sizeof(cfg->cache_path), "/tmp/edgeflow/offline-cache.jsonl");
}

int gateway_config_load(const char *path, gateway_config_t *cfg)
{
    char *json = NULL;
    bool simulate_flag;
    uint32_t u32_val;

    if (path == NULL || cfg == NULL) {
        return -1;
    }

    /* 先 defaults，再按 JSON 覆盖 — 保证缺字段时仍可运行 */
    gateway_config_defaults(cfg);
    if (read_file(path, &json) != 0) {
        return -1;
    }

    json_get_string(json, "device_id", cfg->device_id, sizeof(cfg->device_id));
    if (json_get_bool(json, "simulate", &simulate_flag)) {
        cfg->simulate = simulate_flag;
    }
    json_get_string(json, "serial_device", cfg->serial_device, sizeof(cfg->serial_device));
    if (json_get_uint(json, "poll_interval_ms", &u32_val)) {
        cfg->poll_interval_ms = u32_val;
    }
    if (json_get_uint(json, "queue_size", &u32_val)) {
        cfg->queue_size = u32_val;
    }
    (void)json_get_double(json, "deadband", &cfg->deadband);
    (void)json_get_double(json, "temp_high", &cfg->temp_high);
    (void)json_get_double(json, "peak_shaving_threshold_kw", &cfg->peak_shaving_threshold_kw);
    (void)json_get_double(json, "max_discharge_kw", &cfg->max_discharge_kw);
    (void)json_get_double(json, "min_discharge_soc", &cfg->min_discharge_soc);
    json_get_string(json, "broker_host", cfg->broker_host, sizeof(cfg->broker_host));
    if (json_get_uint(json, "broker_port", &u32_val)) {
        cfg->broker_port = (uint16_t)u32_val;
    }
    json_get_string(json, "mqtt_topic", cfg->mqtt_topic, sizeof(cfg->mqtt_topic));
    json_get_string(json, "log_dir", cfg->log_dir, sizeof(cfg->log_dir));
    json_get_string(json, "metrics_path", cfg->metrics_path, sizeof(cfg->metrics_path));
    json_get_string(json, "cache_path", cfg->cache_path, sizeof(cfg->cache_path));

    free(json);
    return 0;
}
