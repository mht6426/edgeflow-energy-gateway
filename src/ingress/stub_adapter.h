#ifndef EDGEFLOW_STUB_ADAPTER_H
#define EDGEFLOW_STUB_ADAPTER_H

/*
 * Stub Adapter（M3 演示插件）
 *
 * 职责：在无 Modbus 实现前，提供可重复的模拟 telemetry，验证 Adapter 插件接口。
 * M4 将新增 modbus_rtu Adapter；simulate:true 时可与 stub 二选一或并存。
 *
 * 使用：
 *   device_adapter_t stub;
 *   stub_adapter_fill(&stub);
 *   adapter_registry_register(&reg, &stub);
 */

#include "ingress/adapter.h"

/* 填充 stub 插件的 name/protocol/回调，ctx 置 NULL */
void stub_adapter_fill(device_adapter_t *out);

#endif
