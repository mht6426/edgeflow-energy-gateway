#ifndef EDGEFLOW_DEVICE_MODEL_H
#define EDGEFLOW_DEVICE_MODEL_H

#include <stdint.h>

#define EDGEFLOW_ID_MAX 64
#define EDGEFLOW_NAME_MAX 64
#define EDGEFLOW_UNIT_MAX 16
#define EDGEFLOW_MESSAGE_MAX 128

typedef enum {
    DEVICE_PROTOCOL_MODBUS_RTU = 0,
    DEVICE_PROTOCOL_MODBUS_TCP,
    DEVICE_PROTOCOL_TCP,
    DEVICE_PROTOCOL_UDP,
    DEVICE_PROTOCOL_SIMULATOR,
} device_protocol_t;

typedef enum {
    DEVICE_ONLINE_UNKNOWN = 0,
    DEVICE_ONLINE_ONLINE,
    DEVICE_ONLINE_OFFLINE,
    DEVICE_ONLINE_DEGRADED,
} device_online_state_t;

typedef enum {
    POINT_TYPE_ANALOG = 0,
    POINT_TYPE_DIGITAL,
    POINT_TYPE_ENUM,
} point_type_t;

typedef enum {
    TELEMETRY_QUALITY_GOOD = 0,
    TELEMETRY_QUALITY_BAD,
    TELEMETRY_QUALITY_UNCERTAIN,
} telemetry_quality_t;

typedef enum {
    ALARM_LEVEL_INFO = 0,
    ALARM_LEVEL_MINOR,
    ALARM_LEVEL_MAJOR,
    ALARM_LEVEL_CRITICAL,
} alarm_level_t;

typedef enum {
    ALARM_STATE_ACTIVE = 0,
    ALARM_STATE_RECOVERED,
} alarm_state_t;

typedef enum {
    COMMAND_TYPE_SET_POINT_VALUE = 0,
    COMMAND_TYPE_SET_DEVICE_MODE,
    COMMAND_TYPE_STOP_DEVICE,
} command_type_t;

typedef enum {
    COMMAND_STATE_PENDING = 0,
    COMMAND_STATE_SENT,
    COMMAND_STATE_ACKED,
    COMMAND_STATE_VERIFIED,
    COMMAND_STATE_TIMEOUT,
    COMMAND_STATE_RETRYING,
    COMMAND_STATE_FAILED,
} command_state_t;

typedef enum {
    SYSTEM_STATE_INIT = 0,
    SYSTEM_STATE_STANDBY,
    SYSTEM_STATE_RUNNING,
    SYSTEM_STATE_DEGRADED,
    SYSTEM_STATE_FAULT,
    SYSTEM_STATE_STOPPED,
} system_state_t;

typedef struct {
    char id[EDGEFLOW_ID_MAX];
    char name[EDGEFLOW_NAME_MAX];
    device_protocol_t protocol;
    device_online_state_t online_state;
    uint64_t last_seen_ms;
    uint32_t consecutive_failures;
    char last_error[EDGEFLOW_MESSAGE_MAX];
} device_t;

typedef struct {
    char id[EDGEFLOW_ID_MAX];
    char device_id[EDGEFLOW_ID_MAX];
    char name[EDGEFLOW_NAME_MAX];
    point_type_t type;
    char unit[EDGEFLOW_UNIT_MAX];
    double scale;
    double offset;
    double low_limit;
    double high_limit;
} point_t;

typedef struct {
    uint64_t ts_ms;
    char device_id[EDGEFLOW_ID_MAX];
    char point_id[EDGEFLOW_ID_MAX];
    double value;
    char unit[EDGEFLOW_UNIT_MAX];
    telemetry_quality_t quality;
    uint16_t source_id;
} telemetry_t;

typedef struct {
    uint64_t alarm_id;
    uint64_t ts_ms;
    char device_id[EDGEFLOW_ID_MAX];
    char code[EDGEFLOW_ID_MAX];
    alarm_level_t level;
    alarm_state_t state;
    char message[EDGEFLOW_MESSAGE_MAX];
    uint64_t recovered_ts_ms;
} alarm_event_t;

typedef struct {
    uint64_t command_id;
    uint64_t created_ts_ms;
    char target_device_id[EDGEFLOW_ID_MAX];
    command_type_t type;
    command_state_t state;
    char point_id[EDGEFLOW_ID_MAX];
    double target_value;
    int retry_count;
    int max_retries;
    char result_message[EDGEFLOW_MESSAGE_MAX];
} command_t;

const char *telemetry_quality_to_string(telemetry_quality_t quality);
const char *system_state_to_string(system_state_t state);
const char *command_state_to_string(command_state_t state);

void telemetry_init(telemetry_t *telemetry,
                    uint64_t ts_ms,
                    const char *device_id,
                    const char *point_id,
                    double value,
                    const char *unit,
                    uint16_t source_id);
int command_init_setpoint(command_t *command,
                          uint64_t command_id,
                          uint64_t created_ts_ms,
                          const char *target_device_id,
                          const char *point_id,
                          double target_value,
                          int max_retries);
int command_mark_verified(command_t *command, const char *message);
int alarm_make(alarm_event_t *alarm,
               uint64_t alarm_id,
               uint64_t ts_ms,
               const char *device_id,
               const char *code,
               alarm_level_t level,
               const char *message);

#endif
