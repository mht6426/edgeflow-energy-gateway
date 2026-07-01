/*
 * modbus_adapter.c — Modbus 插件适配 device_adapter_t
 */

#include "ingress/modbus_adapter.h"

#include "ingress/modbus_libmodbus.h"
#include "ingress/modbus_rtu.h"
#include "platform/logger.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    const gateway_config_t *cfg;
} modbus_adapter_ctx_t;

static modbus_adapter_ctx_t g_modbus_ctx;

static int modbus_init(device_adapter_t *self, const gateway_config_t *cfg)
{
    modbus_adapter_ctx_t *ctx;

    if (self == NULL || cfg == NULL) {
        return -1;
    }
    ctx = (modbus_adapter_ctx_t *)self->ctx;
    if (ctx == NULL) {
        return -1;
    }
    ctx->cfg = cfg;
    self->protocol = (strcmp(cfg->modbus_transport, "tcp") == 0) ? DEVICE_PROTOCOL_MODBUS_TCP
                                                                 : DEVICE_PROTOCOL_MODBUS_RTU;
    log_info("modbus adapter init: simulate=%s transport=%s serial=%s",
             cfg->simulate ? "true" : "false",
             cfg->modbus_transport,
             cfg->serial_device);
    return 0;
}

static void modbus_shutdown(device_adapter_t *self)
{
    (void)self;
    modbus_libmodbus_shutdown();
    log_info("modbus adapter shutdown");
}

static int modbus_poll(device_adapter_t *self, telemetry_t *out, size_t max_out)
{
    modbus_adapter_ctx_t *ctx;

    if (self == NULL) {
        return -1;
    }
    ctx = (modbus_adapter_ctx_t *)self->ctx;
    if (ctx == NULL || ctx->cfg == NULL) {
        return -1;
    }
    return modbus_rtu_poll(ctx->cfg, out, max_out);
}

static int modbus_write_command(device_adapter_t *self, const command_t *cmd)
{
    (void)self;
    if (cmd == NULL) {
        return -1;
    }
    log_info("modbus write_command simulate: target=%s point=%s value=%.2f",
             cmd->target_device_id,
             cmd->point_id,
             cmd->target_value);
    return 0;
}

void modbus_adapter_fill(device_adapter_t *out)
{
    if (out == NULL) {
        return;
    }
    memset(out, 0, sizeof(*out));
    snprintf(out->name, sizeof(out->name), "modbus");
    out->protocol = DEVICE_PROTOCOL_MODBUS_RTU;
    out->source_id = 2U;
    out->ctx = &g_modbus_ctx;
    out->init = modbus_init;
    out->shutdown = modbus_shutdown;
    out->poll = modbus_poll;
    out->write_command = modbus_write_command;
}
