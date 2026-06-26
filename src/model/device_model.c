/*
 * device_model.c — 统一设备模型辅助函数实现
 *
 * 平台：Linux 用户态（无直接系统调用；结构体设计面向后续 Linux 多线程运行时）
 *
 * 本文件职责：
 *   - 五类模型对象的构造与默认值填充
 *   - 枚举到字符串（日志/调试）
 *   - 设备在线状态维护、测点工程值换算
 *
 * 本文件不负责：
 *   - 设备注册表、JSON 点表加载（M2/M4）
 *   - 状态机转换、告警联锁（M5）
 *   - 命令调度与协议下发（M6/M4）
 */

#include "model/device_model.h"

#include <stdio.h>
#include <string.h>

/*
 * 安全拷贝字符串到固定长度缓冲区。
 * 使用 snprintf 保证 NUL 终止，避免 strncpy 不补 '\0' 的问题。
 */
static void copy_string(char *dst, size_t dst_len, const char *src)
{
    if (dst == NULL || dst_len == 0U) {
        return;
    }
    snprintf(dst, dst_len, "%s", src != NULL ? src : "");
}

/* ---------- 枚举 → 字符串 ---------- */

const char *telemetry_quality_to_string(telemetry_quality_t quality)
{
    switch (quality) {
    case TELEMETRY_QUALITY_GOOD:
        return "good";
    case TELEMETRY_QUALITY_BAD:
        return "bad";
    case TELEMETRY_QUALITY_UNCERTAIN:
        return "uncertain";
    default:
        return "unknown";
    }
}

const char *system_state_to_string(system_state_t state)
{
    switch (state) {
    case SYSTEM_STATE_INIT:
        return "INIT";
    case SYSTEM_STATE_STANDBY:
        return "STANDBY";
    case SYSTEM_STATE_RUNNING:
        return "RUNNING";
    case SYSTEM_STATE_DEGRADED:
        return "DEGRADED";
    case SYSTEM_STATE_FAULT:
        return "FAULT";
    case SYSTEM_STATE_STOPPED:
        return "STOPPED";
    default:
        return "UNKNOWN";
    }
}

const char *command_state_to_string(command_state_t state)
{
    switch (state) {
    case COMMAND_STATE_PENDING:
        return "PENDING";
    case COMMAND_STATE_SENT:
        return "SENT";
    case COMMAND_STATE_ACKED:
        return "ACKED";
    case COMMAND_STATE_VERIFIED:
        return "VERIFIED";
    case COMMAND_STATE_TIMEOUT:
        return "TIMEOUT";
    case COMMAND_STATE_RETRYING:
        return "RETRYING";
    case COMMAND_STATE_FAILED:
        return "FAILED";
    default:
        return "UNKNOWN";
    }
}

/* ---------- 静态元数据：设备与测点 ---------- */

int device_init(device_t *device,
                const char *id,
                const char *name,
                device_protocol_t protocol)
{
    if (device == NULL || id == NULL || name == NULL) {
        return -1;
    }

    memset(device, 0, sizeof(*device));
    copy_string(device->id, sizeof(device->id), id);
    copy_string(device->name, sizeof(device->name), name);
    device->protocol = protocol;
    /* 首次创建时尚未采集，健康状态为未知 */
    device->online_state = DEVICE_ONLINE_UNKNOWN;
    return 0;
}

int point_init(point_t *point,
               const char *id,
               const char *device_id,
               const char *name,
               point_type_t type,
               const char *unit,
               double scale,
               double offset)
{
    if (point == NULL || id == NULL || device_id == NULL || name == NULL) {
        return -1;
    }

    memset(point, 0, sizeof(*point));
    copy_string(point->id, sizeof(point->id), id);
    copy_string(point->device_id, sizeof(point->device_id), device_id);
    copy_string(point->name, sizeof(point->name), name);
    point->type = type;
    copy_string(point->unit, sizeof(point->unit), unit != NULL ? unit : "");
    point->scale = scale;
    point->offset = offset;
    /* low_limit / high_limit 默认 0，由配置加载或调用方后续赋值 */
    return 0;
}

/*
 * Modbus 等协议通常返回整数原始值，需按点表换算为工程值。
 * 例：原始 785，scale=0.1 → 78.5 %
 */
double point_raw_to_engineering(const point_t *point, double raw_value)
{
    if (point == NULL) {
        return raw_value;
    }
    return raw_value * point->scale + point->offset;
}

/* ---------- 设备连接健康（Adapter 采集后调用） ---------- */

void device_mark_online(device_t *device, uint64_t ts_ms)
{
    if (device == NULL) {
        return;
    }
    device->online_state = DEVICE_ONLINE_ONLINE;
    device->last_seen_ms = ts_ms;
    device->consecutive_failures = 0U;
    device->last_error[0] = '\0';
}

/*
 * 渐进降级策略，避免一次超时就判离线（现场通信常有抖动）：
 *   第 1~2 次连续失败 → DEGRADED
 *   第 3 次及以上     → OFFLINE
 */
void device_mark_failure(device_t *device, uint64_t ts_ms, const char *error)
{
    if (device == NULL) {
        return;
    }
    device->last_seen_ms = ts_ms;
    device->consecutive_failures++;
    copy_string(device->last_error, sizeof(device->last_error), error);
    if (device->consecutive_failures >= 3U) {
        device->online_state = DEVICE_ONLINE_OFFLINE;
    } else if (device->consecutive_failures > 0U) {
        device->online_state = DEVICE_ONLINE_DEGRADED;
    }
}

/* ---------- 运行时对象：遥测、命令、告警 ---------- */

void telemetry_init(telemetry_t *telemetry,
                    uint64_t ts_ms,
                    const char *device_id,
                    const char *point_id,
                    double value,
                    const char *unit,
                    uint16_t source_id)
{
    if (telemetry == NULL) {
        return;
    }
    memset(telemetry, 0, sizeof(*telemetry));
    telemetry->ts_ms = ts_ms;
    copy_string(telemetry->device_id, sizeof(telemetry->device_id), device_id);
    copy_string(telemetry->point_id, sizeof(telemetry->point_id), point_id);
    telemetry->value = value;
    copy_string(telemetry->unit, sizeof(telemetry->unit), unit);
    /* Adapter 构造时默认 GOOD；规则过滤或校验失败由上层改为 BAD */
    telemetry->quality = TELEMETRY_QUALITY_GOOD;
    telemetry->source_id = source_id;
}

int command_init_setpoint(command_t *command,
                          uint64_t command_id,
                          uint64_t created_ts_ms,
                          const char *target_device_id,
                          const char *point_id,
                          double target_value,
                          int max_retries)
{
    if (command == NULL || target_device_id == NULL || point_id == NULL || max_retries < 0) {
        return -1;
    }

    memset(command, 0, sizeof(*command));
    command->command_id = command_id;
    command->created_ts_ms = created_ts_ms;
    copy_string(command->target_device_id, sizeof(command->target_device_id), target_device_id);
    command->type = COMMAND_TYPE_SET_POINT_VALUE;
    /* 策略只负责置 PENDING；后续状态由 Command Scheduler（M6）推进 */
    command->state = COMMAND_STATE_PENDING;
    copy_string(command->point_id, sizeof(command->point_id), point_id);
    command->target_value = target_value;
    command->max_retries = max_retries;
    return 0;
}

/* 演示/测试用；生产路径应在 Scheduler 回读校验通过后调用 */
int command_mark_verified(command_t *command, const char *message)
{
    if (command == NULL) {
        return -1;
    }
    command->state = COMMAND_STATE_VERIFIED;
    copy_string(command->result_message, sizeof(command->result_message), message);
    return 0;
}

int alarm_make(alarm_event_t *alarm,
               uint64_t alarm_id,
               uint64_t ts_ms,
               const char *device_id,
               const char *code,
               alarm_level_t level,
               const char *message)
{
    if (alarm == NULL || device_id == NULL || code == NULL) {
        return -1;
    }

    memset(alarm, 0, sizeof(*alarm));
    alarm->alarm_id = alarm_id;
    alarm->ts_ms = ts_ms;
    copy_string(alarm->device_id, sizeof(alarm->device_id), device_id);
    copy_string(alarm->code, sizeof(alarm->code), code);
    alarm->level = level;
    alarm->state = ALARM_STATE_ACTIVE;
    copy_string(alarm->message, sizeof(alarm->message), message);
    /* recovered_ts_ms 在告警恢复时由 State Engine（M5）填写 */
    return 0;
}
