#ifndef EDGEFLOW_CONTROL_LOOP_H
#define EDGEFLOW_CONTROL_LOOP_H

#include "model/device_model.h"
#include "platform/config.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    system_state_t state;
    double soc_percent;
    double max_cell_temp_c;
    double grid_power_kw;
    uint64_t next_alarm_id;
    uint64_t next_command_id;
} control_loop_t;

void control_loop_init(control_loop_t *loop);
bool control_loop_process_telemetry(control_loop_t *loop,
                                    const gateway_config_t *cfg,
                                    const telemetry_t *telemetry,
                                    alarm_event_t *alarm,
                                    command_t *command);

#endif
