/*
 * modbus_libmodbus.c — 基于 libmodbus 的 RTU/TCP 采集
 */

#include "ingress/modbus_libmodbus.h"

#include "common/time_util.h"
#include "platform/logger.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <modbus.h>
#include <modbus-rtu.h>
#include <modbus-tcp.h>

static modbus_t *g_ctx;
static char g_transport[8];

static void copy_transport_label(const char *src, char *dst, size_t len)
{
    if (len == 0U) {
        return;
    }
    snprintf(dst, len, "%s", src != NULL ? src : "rtu");
}

static int ensure_context(const gateway_config_t *cfg)
{
    const char *want = cfg->modbus_transport;

    if (g_ctx != NULL && strcmp(g_transport, want) == 0) {
        return 0;
    }

    modbus_libmodbus_shutdown();

    if (strcmp(want, "tcp") == 0) {
        g_ctx = modbus_new_tcp(cfg->modbus_tcp_host, (int)cfg->modbus_tcp_port);
    } else {
        g_ctx = modbus_new_rtu(cfg->serial_device,
                               (int)cfg->modbus_baud_rate,
                               cfg->modbus_parity,
                               cfg->modbus_data_bits,
                               cfg->modbus_stop_bits);
    }

    if (g_ctx == NULL) {
        log_error("libmodbus: create context failed transport=%s", want);
        return -1;
    }

    modbus_set_slave(g_ctx, (int)cfg->modbus_slave_id);
    modbus_set_response_timeout(g_ctx, 0, (int)cfg->modbus_timeout_ms * 1000);

    if (modbus_connect(g_ctx) != 0) {
        log_warn("libmodbus connect failed: %s", modbus_strerror(errno));
        modbus_free(g_ctx);
        g_ctx = NULL;
        return -1;
    }

    copy_transport_label(want, g_transport, sizeof(g_transport));
    log_info("libmodbus connected: transport=%s slave=%u",
             want,
             (unsigned)cfg->modbus_slave_id);
    return 0;
}

void modbus_libmodbus_shutdown(void)
{
    if (g_ctx == NULL) {
        return;
    }
    modbus_close(g_ctx);
    modbus_free(g_ctx);
    g_ctx = NULL;
    g_transport[0] = '\0';
}

static int map_registers_to_telemetry(const uint16_t *regs, telemetry_t *out, size_t max_out)
{
    uint64_t now = edgeflow_now_ms();

    if (max_out < 3U) {
        return -1;
    }

    telemetry_init(&out[0], now, "bms_001", "soc_percent", (double)regs[0] * 0.1, "%", 2U);
    telemetry_init(&out[1], now, "bms_001", "max_cell_temp_c", (double)regs[1] * 0.1, "C", 2U);
    telemetry_init(&out[2], now, "meter_001", "grid_power_kw", (double)regs[2] * 0.1, "kW", 2U);
    return 3;
}

int modbus_libmodbus_poll(const gateway_config_t *cfg, telemetry_t *out, size_t max_out)
{
    uint16_t regs[3];
    int rc;

    if (cfg == NULL || out == NULL || max_out < 3U) {
        return -1;
    }

    if (ensure_context(cfg) != 0) {
        return -1;
    }

    rc = modbus_read_registers(g_ctx, 0, 3, regs);
    if (rc < 0) {
        log_warn("libmodbus read failed: %s", modbus_strerror(errno));
        modbus_libmodbus_shutdown();
        return -1;
    }

    return map_registers_to_telemetry(regs, out, max_out);
}
