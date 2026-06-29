#ifndef EDGEFLOW_MODBUS_RTU_H
#define EDGEFLOW_MODBUS_RTU_H

/*
 * Modbus RTU 采集（M4）
 *
 * 平台：Linux 串口 /dev/ttyUSB*（simulate 模式走内置模拟器）
 * 职责：CRC16、模拟 BMS/电表点位、封装为 telemetry_t。
 */

#include "model/device_model.h"
#include "platform/config.h"

#include <stddef.h>
#include <stdint.h>

uint16_t modbus_crc16(const uint8_t *buf, size_t len);

/*
 * 执行一轮 Modbus 采集（当前 simulate 路径）。
 * @return 写入 out 的条数，<0 错误
 */
int modbus_rtu_poll(const gateway_config_t *cfg, telemetry_t *out, size_t max_out);

#endif
