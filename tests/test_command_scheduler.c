/*
 * test_command_scheduler.c — 指令调度单元测试（M6）
 */

#include "runtime/command_scheduler.h"
#include "ingress/modbus_adapter.h"
#include "platform/config.h"

#include <assert.h>
#include <stdio.h>

int main(void)
{
    command_scheduler_t scheduler;
    device_adapter_t modbus;
    gateway_config_t cfg;
    command_t cmd;

    gateway_config_defaults(&cfg);
    command_scheduler_init(&scheduler);
    modbus_adapter_fill(&modbus);
    assert(modbus.init(&modbus, &cfg) == 0);

    assert(command_init_setpoint(&cmd, 0U, 1U, "pcs_001", "target_power_kw", -10.0, 3) == 0);
    assert(command_scheduler_submit(&scheduler, &cmd) == 0);
    assert(command_scheduler_tick(&scheduler, &modbus, &cfg, NULL) == 1);
    assert(scheduler.count == 0U);

    modbus.shutdown(&modbus);
    printf("test_command_scheduler passed\n");
    return 0;
}
