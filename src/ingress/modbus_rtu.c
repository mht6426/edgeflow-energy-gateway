/*
 * modbus_rtu.c — Modbus RTU CRC、模拟采集与真实串口路径
 */

#include "ingress/modbus_rtu.h"

#include "common/time_util.h"
#include "ingress/modbus_libmodbus.h"
#include "platform/logger.h"

#include <stdio.h>

uint16_t modbus_crc16(const uint8_t *buf, size_t len)
{
    uint16_t crc = 0xFFFFU;

    for (size_t i = 0U; i < len; i++) {
        crc ^= buf[i];
        for (int bit = 0; bit < 8; bit++) {
            if ((crc & 0x0001U) != 0U) {
                crc = (uint16_t)((crc >> 1U) ^ 0xA001U);
            } else {
                crc >>= 1U;
            }
        }
    }
    return crc;
}

static int modbus_poll_simulate(const gateway_config_t *cfg, telemetry_t *out, size_t max_out)
{
    static uint32_t seq;
    uint8_t req[] = {0x01U, 0x03U, 0x00U, 0x00U, 0x00U, 0x03U, 0x00U, 0x00U};
    uint16_t crc;
    uint64_t now;

    (void)cfg;
    if (out == NULL || max_out < 3U) {
        return -1;
    }

    crc = modbus_crc16(req, 6U);
    req[6] = (uint8_t)(crc & 0xFFU);
    req[7] = (uint8_t)(crc >> 8U);
    (void)req;

    now = edgeflow_now_ms();
    telemetry_init(&out[0], now, "bms_001", "soc_percent", 80.0 - (double)(seq % 30U) * 0.2, "%", 2U);
    telemetry_init(&out[1], now, "bms_001", "max_cell_temp_c", 28.0 + (double)(seq % 20U) * 0.2, "C", 2U);
    telemetry_init(&out[2], now, "meter_001", "grid_power_kw", 90.0 + (double)(seq % 80U), "kW", 2U);
    seq++;
    return 3;
}

int modbus_rtu_poll(const gateway_config_t *cfg, telemetry_t *out, size_t max_out)
{
    if (cfg == NULL || out == NULL || max_out < 3U) {
        return -1;
    }

    if (cfg->simulate) {
        return modbus_poll_simulate(cfg, out, max_out);
    }
    return modbus_libmodbus_poll(cfg, out, max_out);
}
