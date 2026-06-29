/*
 * state_engine.c — 状态机、告警联锁、削峰填谷策略原型（M5）
 */

#include "runtime/state_engine.h"

#include <string.h>

void state_engine_init(state_engine_t *engine)
{
    if (engine == NULL) {
        return;
    }
    memset(engine, 0, sizeof(*engine));
    engine->state = SYSTEM_STATE_INIT;
    engine->soc_percent = 80.0;
    engine->next_alarm_id = 1U;
}

static bool update_snapshot(state_engine_t *engine, const telemetry_t *telemetry)
{
    if (strcmp(telemetry->point_id, "soc_percent") == 0) {
        engine->soc_percent = telemetry->value;
        return true;
    }
    if (strcmp(telemetry->point_id, "max_cell_temp_c") == 0) {
        engine->max_cell_temp_c = telemetry->value;
        return true;
    }
    if (strcmp(telemetry->point_id, "grid_power_kw") == 0) {
        engine->grid_power_kw = telemetry->value;
        return true;
    }
    return false;
}

static double clamp_discharge(double kw, double max_kw)
{
    if (kw < 0.0) {
        return 0.0;
    }
    if (kw > max_kw) {
        return max_kw;
    }
    return kw;
}

bool state_engine_process_telemetry(state_engine_t *engine,
                                    const gateway_config_t *cfg,
                                    const telemetry_t *telemetry,
                                    alarm_event_t *alarm,
                                    command_t *command,
                                    uint64_t command_id,
                                    uint64_t ts_ms)
{
    double discharge_kw;

    if (engine == NULL || cfg == NULL || telemetry == NULL) {
        return false;
    }

    if (telemetry->quality != TELEMETRY_QUALITY_GOOD) {
        return false;
    }

    (void)update_snapshot(engine, telemetry);

    if (engine->max_cell_temp_c > cfg->temp_high) {
        engine->state = SYSTEM_STATE_FAULT;
        if (alarm != NULL) {
            (void)alarm_make(alarm,
                             engine->next_alarm_id++,
                             ts_ms,
                             "bms_001",
                             "BMS_TEMP_HIGH",
                             ALARM_LEVEL_MAJOR,
                             "max cell temperature exceeds limit");
            return true;
        }
    }

    if (engine->soc_percent <= cfg->min_discharge_soc) {
        if (alarm != NULL && strcmp(telemetry->point_id, "soc_percent") == 0) {
            (void)alarm_make(alarm,
                             engine->next_alarm_id++,
                             ts_ms,
                             "bms_001",
                             "BMS_SOC_LOW",
                             ALARM_LEVEL_MAJOR,
                             "SOC below minimum discharge threshold");
            return true;
        }
    }

    engine->state = SYSTEM_STATE_RUNNING;

    if (strcmp(telemetry->point_id, "grid_power_kw") != 0) {
        return false;
    }
    if (engine->soc_percent <= cfg->min_discharge_soc) {
        return false;
    }
    if (engine->grid_power_kw <= cfg->peak_shaving_threshold_kw) {
        return false;
    }

    discharge_kw = clamp_discharge(engine->grid_power_kw - cfg->peak_shaving_threshold_kw,
                                   cfg->max_discharge_kw);
    if (command == NULL || discharge_kw <= 0.0) {
        return false;
    }

    if (command_init_setpoint(command,
                              command_id,
                              ts_ms,
                              "pcs_001",
                              "target_power_kw",
                              -discharge_kw,
                              3) != 0) {
        return false;
    }
    return true;
}
