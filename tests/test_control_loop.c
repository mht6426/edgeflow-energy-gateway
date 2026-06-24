#include "common/time_util.h"
#include "runtime/control_loop.h"

#include <assert.h>
#include <stdio.h>

int main(void)
{
    gateway_config_t cfg;
    control_loop_t loop;
    telemetry_t telemetry;
    alarm_event_t alarm = {0};
    command_t command = {0};

    gateway_config_defaults(&cfg);
    control_loop_init(&loop);

    telemetry_init(&telemetry, edgeflow_now_ms(), "bms_001", "soc_percent", 75.0, "%", 1U);
    assert(!control_loop_process_telemetry(&loop, &cfg, &telemetry, &alarm, &command));

    telemetry_init(&telemetry, edgeflow_now_ms(), "meter_001", "grid_power_kw", 150.0, "kW", 2U);
    assert(control_loop_process_telemetry(&loop, &cfg, &telemetry, &alarm, &command));
    assert(command.command_id == 1U);
    assert(command.state == COMMAND_STATE_VERIFIED);
    assert(command.target_value < 0.0);
    assert(loop.state == SYSTEM_STATE_RUNNING);

    telemetry_init(&telemetry, edgeflow_now_ms(), "bms_001", "max_cell_temp_c", 90.0, "C", 1U);
    alarm = (alarm_event_t){0};
    command = (command_t){0};
    assert(control_loop_process_telemetry(&loop, &cfg, &telemetry, &alarm, &command));
    assert(alarm.alarm_id == 1U);
    assert(loop.state == SYSTEM_STATE_FAULT);

    printf("test_control_loop passed\n");
    return 0;
}
