#ifndef EDGEFLOW_MODBUS_LIBMODBUS_H
#define EDGEFLOW_MODBUS_LIBMODBUS_H

/*
 * libmodbus 后端 — Modbus RTU / TCP 真实采集
 *
 * simulate=false 时由 modbus_rtu_poll 委托调用。
 */

#include "model/device_model.h"
#include "platform/config.h"

#include <stddef.h>

void modbus_libmodbus_shutdown(void);
int modbus_libmodbus_poll(const gateway_config_t *cfg, telemetry_t *out, size_t max_out);

#endif
