/*
 * test_state_engine.c — 状态机与削峰策略单元测试（M5）
 */

#include "runtime/state_engine.h"
#include "platform/config.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    state_engine_t engine;
    gateway_config_t cfg;
    telemetry_t telemetry;
    alarm_event_t alarm;
    command_t command;

    gateway_config_defaults(&cfg);
    cfg.peak_shaving_threshold_kw = 100.0;
    cfg.max_discharge_kw = 50.0;
    cfg.min_discharge_soc = 20.0;
    cfg.temp_high = 60.0;

    state_engine_init(&engine);

    telemetry_init(&telemetry, 100U, "bms_001", "grid_power_kw", 150.0, "kW", 1U);
    engine.soc_percent = 80.0;
    memset(&command, 0, sizeof(command));
    assert(state_engine_process_telemetry(&engine, &cfg, &telemetry, NULL, &command, 1U, 100U));
    assert(command.target_value < 0.0);

    telemetry_init(&telemetry, 200U, "bms_001", "max_cell_temp_c", 65.0, "C", 1U);
    memset(&alarm, 0, sizeof(alarm));
    assert(state_engine_process_telemetry(&engine, &cfg, &telemetry, &alarm, NULL, 2U, 200U));
    assert(strcmp(alarm.code, "BMS_TEMP_HIGH") == 0);

    printf("test_state_engine passed\n");
    return 0;
}
