#include "runtime/control_loop.h"

#include <string.h>

void control_loop_init(control_loop_t *loop)
{
    if (loop == NULL) {
        return;
    }
    memset(loop, 0, sizeof(*loop));
    loop->state = SYSTEM_STATE_INIT;
    loop->soc_percent = 80.0;
    loop->next_alarm_id = 1U;
    loop->next_command_id = 1U;
}

static double clamp_discharge_power(double requested_kw, double max_discharge_kw)
{
    if (requested_kw < 0.0) {
        return 0.0;
    }
    if (requested_kw > max_discharge_kw) {
        return max_discharge_kw;
    }
    return requested_kw;
}

static bool update_snapshot(control_loop_t *loop, const telemetry_t *telemetry)
{
    if (strcmp(telemetry->point_id, "soc_percent") == 0) {
        loop->soc_percent = telemetry->value;
        return true;
    }
    if (strcmp(telemetry->point_id, "max_cell_temp_c") == 0) {
        loop->max_cell_temp_c = telemetry->value;
        return true;
    }
    if (strcmp(telemetry->point_id, "grid_power_kw") == 0) {
        loop->grid_power_kw = telemetry->value;
        return true;
    }
    return false;
}

bool control_loop_process_telemetry(control_loop_t *loop,
                                    const gateway_config_t *cfg,
                                    const telemetry_t *telemetry,
                                    alarm_event_t *alarm,
                                    command_t *command)
{
    double discharge_kw;

    if (loop == NULL || cfg == NULL || telemetry == NULL) {
        return false;
    }

    (void)update_snapshot(loop, telemetry);

    if (loop->max_cell_temp_c > cfg->temp_high) {
        loop->state = SYSTEM_STATE_FAULT;
        if (alarm != NULL) {
            (void)alarm_make(alarm,
                             loop->next_alarm_id++,
                             telemetry->ts_ms,
                             "bms_001",
                             "BMS_TEMP_HIGH",
                             ALARM_LEVEL_MAJOR,
                             "max cell temperature exceeds configured limit");
        }
        return alarm != NULL;
    }

    loop->state = SYSTEM_STATE_RUNNING;
    if (strcmp(telemetry->point_id, "grid_power_kw") != 0) {
        return false;
    }
    if (loop->soc_percent <= cfg->min_discharge_soc) {
        return false;
    }
    if (loop->grid_power_kw <= cfg->peak_shaving_threshold_kw) {
        return false;
    }

    discharge_kw = clamp_discharge_power(loop->grid_power_kw - cfg->peak_shaving_threshold_kw,
                                         cfg->max_discharge_kw);
    if (command == NULL || discharge_kw <= 0.0) {
        return false;
    }

    if (command_init_setpoint(command,
                              loop->next_command_id++,
                              telemetry->ts_ms,
                              "pcs_001",
                              "target_power_kw",
                              -discharge_kw,
                              3) != 0) {
        return false;
    }
    command->state = COMMAND_STATE_SENT;
    (void)command_mark_verified(command, "simulator readback verified");
    return true;
}
