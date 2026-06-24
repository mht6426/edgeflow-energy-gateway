#include "ingress/modbus_rtu.h"

#include "common/time_util.h"
#include "platform/logger.h"

#include <stdio.h>

uint16_t modbus_crc16(const uint8_t *buf, size_t len)
{
    uint16_t crc = 0xFFFFU;

    for (size_t i = 0; i < len; i++) {
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

int modbus_rtu_poll(const gateway_config_t *cfg, data_point_t *out, size_t max_points)
{
    static uint32_t seq;
    static bool serial_warned;
    uint8_t req[] = {0x01U, 0x03U, 0x00U, 0x00U, 0x00U, 0x03U, 0x00U, 0x00U};
    uint16_t crc;

    if (cfg == NULL || out == NULL || max_points < 3U) {
        return -1;
    }

    if (!cfg->simulate) {
        if (!serial_warned) {
            log_warn("serial modbus not implemented yet, device=%s", cfg->serial_device);
            serial_warned = true;
        }
        return -1;
    }

    crc = modbus_crc16(req, 6U);
    req[6] = (uint8_t)(crc & 0xFFU);
    req[7] = (uint8_t)(crc >> 8U);

    /* 首版使用可重复的工业控制器示例点位，保留 Modbus RTU 请求与 CRC 路径用于串口接入。 */
    telemetry_init(&out[0],
                   edgeflow_now_ms(),
                   "bms_001",
                   "soc_percent",
                   80.0 - (double)(seq % 30U) * 0.2,
                   "%",
                   1U);
    telemetry_init(&out[1],
                   edgeflow_now_ms(),
                   "bms_001",
                   "max_cell_temp_c",
                   28.0 + (double)(seq % 20U) * 0.2,
                   "C",
                   1U);
    telemetry_init(&out[2],
                   edgeflow_now_ms(),
                   "meter_001",
                   "grid_power_kw",
                   90.0 + (double)(seq % 80U),
                   "kW",
                   2U);
    seq++;
    return 3;
}
