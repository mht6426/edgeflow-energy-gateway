#include "model/device_model.h"

#include <stdio.h>
#include <string.h>

static void copy_string(char *dst, size_t dst_len, const char *src)
{
    if (dst == NULL || dst_len == 0U) {
        return;
    }
    snprintf(dst, dst_len, "%s", src != NULL ? src : "");
}

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
    command->state = COMMAND_STATE_PENDING;
    copy_string(command->point_id, sizeof(command->point_id), point_id);
    command->target_value = target_value;
    command->max_retries = max_retries;
    return 0;
}

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
    return 0;
}
