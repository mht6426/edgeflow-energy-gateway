#ifndef EDGEFLOW_COMMAND_SCHEDULER_H
#define EDGEFLOW_COMMAND_SCHEDULER_H

/*
 * Command Scheduler（M6）
 *
 * 职责：命令队列、PENDING→SENT→VERIFIED/FAILED、模拟回读校验。
 * 策略/状态机只 submit，不直接写设备。
 */

#include "ingress/adapter.h"
#include "model/device_model.h"
#include "platform/config.h"

#include <stddef.h>
#include <stdint.h>

#define COMMAND_QUEUE_MAX 32U

typedef struct {
    command_t queue[COMMAND_QUEUE_MAX];
    size_t count;
    uint64_t next_command_id;
} command_scheduler_t;

void command_scheduler_init(command_scheduler_t *scheduler);

/* 策略产出命令入队；分配 command_id */
int command_scheduler_submit(command_scheduler_t *scheduler, command_t *command);

/*
 * 处理队列中的 PENDING 命令：经 Adapter 下发并模拟回读校验。
 * @return 本轮处理的命令数
 */
int command_scheduler_tick(command_scheduler_t *scheduler,
                           device_adapter_t *adapter,
                           const gateway_config_t *cfg,
                           storage_ctx_t *storage);

#endif
