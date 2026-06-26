/*
 * test_device_model.c — 统一设备模型单元测试（M1）
 *
 * 平台：与目标运行时一致（Linux），纯用户态逻辑，无 I/O 依赖。
 *
 * 覆盖范围：
 *   - device_t：init、mark_online、mark_failure（含 3 次失败 → OFFLINE）
 *   - point_t：init、raw → engineering 换算
 *   - telemetry_t：init、quality 字符串化
 *   - system_state_t / command_state_t：枚举转字符串
 *   - command_t：setpoint 初始化、mark_verified
 *   - alarm_event_t：alarm_make
 *
 * 运行：ctest --test-dir build -R test_device_model
 */

#include "model/device_model.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    device_t device;
    point_t point;
    telemetry_t telemetry;
    command_t command;
    alarm_event_t alarm;

    /* --- device_t：初始化默认 UNKNOWN；成功采集后 ONLINE --- */
    assert(device_init(&device, "bms_001", "BMS", DEVICE_PROTOCOL_SIMULATOR) == 0);
    assert(strcmp(device.id, "bms_001") == 0);
    assert(device.online_state == DEVICE_ONLINE_UNKNOWN);

    device_mark_online(&device, 1000U);
    assert(device.online_state == DEVICE_ONLINE_ONLINE);
    assert(device.consecutive_failures == 0U);

    /*
     * 渐进降级：第 1 次失败 → DEGRADED；第 3 次连续失败 → OFFLINE
     * 对应现场 Modbus 偶发超时不应立即判死。
     */
    device_mark_failure(&device, 2000U, "timeout");
    assert(device.online_state == DEVICE_ONLINE_DEGRADED);
    device_mark_failure(&device, 3000U, "timeout");
    device_mark_failure(&device, 4000U, "timeout");
    assert(device.online_state == DEVICE_ONLINE_OFFLINE);

    /* --- point_t：Modbus 原始 785 × scale 0.1 = 78.5% --- */
    assert(point_init(&point, "soc_percent", "bms_001", "SOC", POINT_TYPE_ANALOG, "%", 0.1, 0.0) == 0);
    assert(point_raw_to_engineering(&point, 785.0) == 78.5);

    /* --- telemetry_t：默认 quality 为 GOOD --- */
    telemetry_init(&telemetry, 100U, "bms_001", "soc_percent", 78.5, "%", 1U);
    assert(strcmp(telemetry.device_id, "bms_001") == 0);
    assert(strcmp(telemetry.point_id, "soc_percent") == 0);
    assert(telemetry.quality == TELEMETRY_QUALITY_GOOD);
    assert(strcmp(telemetry_quality_to_string(telemetry.quality), "good") == 0);
    assert(strcmp(system_state_to_string(SYSTEM_STATE_RUNNING), "RUNNING") == 0);

    /* --- command_t：策略产出 PENDING；Scheduler 校验后 VERIFIED（此处直接测 API） --- */
    assert(command_init_setpoint(&command, 1U, 200U, "pcs_001", "target_power_kw", -25.0, 3) == 0);
    assert(command.state == COMMAND_STATE_PENDING);
    assert(command_mark_verified(&command, "ok") == 0);
    assert(command.state == COMMAND_STATE_VERIFIED);
    assert(strcmp(command_state_to_string(command.state), "VERIFIED") == 0);

    /* --- alarm_event_t：新告警默认 ACTIVE --- */
    assert(alarm_make(&alarm, 1U, 300U, "bms_001", "BMS_TEMP_HIGH", ALARM_LEVEL_MAJOR, "temp high") == 0);
    assert(alarm.state == ALARM_STATE_ACTIVE);
    assert(strcmp(alarm.code, "BMS_TEMP_HIGH") == 0);

    printf("test_device_model passed\n");
    return 0;
}
