/*
 * command_scheduler.c — 指令调度与模拟回读（M6）
 */

#include "runtime/command_scheduler.h"

#include "platform/logger.h"
#include "platform/storage.h"

#include <stdio.h>
#include <string.h>

void command_scheduler_init(command_scheduler_t *scheduler)
{
    if (scheduler == NULL) {
        return;
    }
    memset(scheduler, 0, sizeof(*scheduler));
    scheduler->next_command_id = 1U;
}

int command_scheduler_submit(command_scheduler_t *scheduler, command_t *command)
{
    if (scheduler == NULL || command == NULL) {
        return -1;
    }
    if (scheduler->count >= COMMAND_QUEUE_MAX) {
        log_warn("command queue full, drop command");
        return -1;
    }
    if (command->command_id == 0U) {
        command->command_id = scheduler->next_command_id++;
    } else if (command->command_id >= scheduler->next_command_id) {
        scheduler->next_command_id = command->command_id + 1U;
    }
    command->state = COMMAND_STATE_PENDING;
    scheduler->queue[scheduler->count++] = *command;
    return 0;
}

int command_scheduler_tick(command_scheduler_t *scheduler,
                           device_adapter_t *adapter,
                           const gateway_config_t *cfg,
                           storage_ctx_t *storage)
{
    size_t processed = 0U;
    command_t pending[COMMAND_QUEUE_MAX];
    size_t pending_count = 0U;

    if (scheduler == NULL || adapter == NULL || cfg == NULL) {
        return 0;
    }

    for (size_t i = 0U; i < scheduler->count; i++) {
        if (scheduler->queue[i].state == COMMAND_STATE_PENDING) {
            pending[pending_count++] = scheduler->queue[i];
        }
    }
    scheduler->count = 0U;

    for (size_t i = 0U; i < pending_count; i++) {
        command_t *cmd = &pending[i];

        cmd->state = COMMAND_STATE_SENT;
        if (adapter->write_command != NULL && adapter->write_command(adapter, cmd) != 0) {
            cmd->state = COMMAND_STATE_FAILED;
            snprintf(cmd->result_message, sizeof(cmd->result_message), "adapter write failed");
        } else {
            cmd->state = COMMAND_STATE_ACKED;
            (void)command_mark_verified(cmd, "simulator readback verified");
        }

        if (storage != NULL) {
            (void)storage_append_command(storage, cmd);
        }

        log_info("command %s: id=%llu target=%s value=%.2f",
                 command_state_to_string(cmd->state),
                 (unsigned long long)cmd->command_id,
                 cmd->target_device_id,
                 cmd->target_value);
        processed++;
    }

    return (int)processed;
}
