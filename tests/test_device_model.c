#include "model/device_model.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    telemetry_t telemetry;
    command_t command;
    alarm_event_t alarm;

    telemetry_init(&telemetry, 100U, "bms_001", "soc_percent", 78.5, "%", 1U);
    assert(strcmp(telemetry.device_id, "bms_001") == 0);
    assert(strcmp(telemetry.point_id, "soc_percent") == 0);
    assert(telemetry.quality == TELEMETRY_QUALITY_GOOD);
    assert(strcmp(telemetry_quality_to_string(telemetry.quality), "good") == 0);

    assert(command_init_setpoint(&command, 1U, 200U, "pcs_001", "target_power_kw", -25.0, 3) == 0);
    assert(command.state == COMMAND_STATE_PENDING);
    assert(command_mark_verified(&command, "ok") == 0);
    assert(command.state == COMMAND_STATE_VERIFIED);
    assert(strcmp(command_state_to_string(command.state), "VERIFIED") == 0);

    assert(alarm_make(&alarm, 1U, 300U, "bms_001", "BMS_TEMP_HIGH", ALARM_LEVEL_MAJOR, "temp high") == 0);
    assert(alarm.state == ALARM_STATE_ACTIVE);
    assert(strcmp(alarm.code, "BMS_TEMP_HIGH") == 0);

    printf("test_device_model passed\n");
    return 0;
}
