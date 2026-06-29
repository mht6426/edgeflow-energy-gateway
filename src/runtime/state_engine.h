#ifndef EDGEFLOW_STATE_ENGINE_H
#define EDGEFLOW_STATE_ENGINE_H

/*
 * State And Alarm Engine（M5）
 *
 * 职责：消费 telemetry，维护整机状态与设备快照，产出告警与待调度命令。
 * 不负责：协议采集、命令下发（M6 Scheduler）、MQTT。
 */

#include "model/device_model.h"
#include "platform/config.h"

#include <stdbool.h>

typedef struct {
    system_state_t state;
    double soc_percent;
    double max_cell_temp_c;
    double grid_power_kw;
    uint64_t next_alarm_id;
} state_engine_t;

void state_engine_init(state_engine_t *engine);

/*
 * 处理一条 telemetry，更新快照；必要时填充 alarm 或待提交 command。
 * @return true 表示 alarm 或 command 有效
 */
bool state_engine_process_telemetry(state_engine_t *engine,
                                  const gateway_config_t *cfg,
                                  const telemetry_t *telemetry,
                                  alarm_event_t *alarm,
                                  command_t *command,
                                  uint64_t command_id,
                                  uint64_t ts_ms);

#endif
