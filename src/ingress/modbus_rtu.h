#ifndef EDGEFLOW_MODBUS_RTU_H
#define EDGEFLOW_MODBUS_RTU_H

#include "common/datapoint.h"
#include "platform/config.h"

#include <stddef.h>
#include <stdint.h>

uint16_t modbus_crc16(const uint8_t *buf, size_t len);
int modbus_rtu_poll(const gateway_config_t *cfg, data_point_t *out, size_t max_points);

#endif
