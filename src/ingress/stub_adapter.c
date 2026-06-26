/*
 * stub_adapter.c — M3 内置模拟采集插件
 *
 * 平台：Linux
 *
 * poll 行为：
 *   cfg->simulate == true 时返回 BMS/电表共 3 条工程值 telemetry
 *   simulate == false 时返回 0（M4 真实串口由 modbus_rtu 接管）
 *
 * write_command：M3 未实现，返回 -1。
 */

#include "ingress/stub_adapter.h"

#include "common/time_util.h"
#include "model/device_model.h"
#include "platform/logger.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    uint32_t seq;
    const gateway_config_t *cfg; /* init 时绑定，poll 读取 simulate 等 */
} stub_adapter_ctx_t;

static stub_adapter_ctx_t g_stub_ctx;

static int stub_init(device_adapter_t *self, const gateway_config_t *cfg)
{
    stub_adapter_ctx_t *ctx;

    if (self == NULL || cfg == NULL) {
        return -1;
    }
    ctx = (stub_adapter_ctx_t *)self->ctx;
    if (ctx == NULL) {
        return -1;
    }
    ctx->seq = 0U;
    ctx->cfg = cfg;
    log_info("stub adapter init: name=%s simulate=%s source_id=%u",
             self->name,
             cfg->simulate ? "true" : "false",
             (unsigned)self->source_id);
    return 0;
}

static void stub_shutdown(device_adapter_t *self)
{
    if (self != NULL) {
        log_info("stub adapter shutdown: name=%s", self->name);
    }
}

static int stub_poll(device_adapter_t *self, telemetry_t *out, size_t max_out)
{
    stub_adapter_ctx_t *ctx;
    uint64_t now;
    uint32_t seq;

    if (self == NULL || out == NULL || max_out < 3U) {
        return -1;
    }

    ctx = (stub_adapter_ctx_t *)self->ctx;
    if (ctx == NULL || ctx->cfg == NULL) {
        return -1;
    }

    if (!ctx->cfg->simulate) {
        return 0;
    }

    seq = ctx->seq++;
    now = edgeflow_now_ms();

    telemetry_init(&out[0],
                   now,
                   "bms_001",
                   "soc_percent",
                   80.0 - (double)(seq % 30U) * 0.2,
                   "%",
                   self->source_id);
    telemetry_init(&out[1],
                   now,
                   "bms_001",
                   "max_cell_temp_c",
                   28.0 + (double)(seq % 20U) * 0.2,
                   "C",
                   self->source_id);
    telemetry_init(&out[2],
                   now,
                   "meter_001",
                   "grid_power_kw",
                   90.0 + (double)(seq % 80U),
                   "kW",
                   self->source_id);

    return 3;
}

static int stub_write_command(device_adapter_t *self, const command_t *cmd)
{
    (void)self;
    (void)cmd;
    return -1;
}

void stub_adapter_fill(device_adapter_t *out)
{
    if (out == NULL) {
        return;
    }
    memset(out, 0, sizeof(*out));
    snprintf(out->name, sizeof(out->name), "stub");
    out->protocol = DEVICE_PROTOCOL_SIMULATOR;
    out->source_id = 1U;
    out->ctx = &g_stub_ctx;
    out->init = stub_init;
    out->shutdown = stub_shutdown;
    out->poll = stub_poll;
    out->write_command = stub_write_command;
}
