#ifndef EDGEFLOW_MODBUS_ADAPTER_H
#define EDGEFLOW_MODBUS_ADAPTER_H

/*
 * Modbus RTU Device Adapter 插件封装（M4）
 * 将 modbus_rtu_poll 挂接到 device_adapter_t 函数表。
 */

#include "ingress/adapter.h"

void modbus_adapter_fill(device_adapter_t *out);

#endif
