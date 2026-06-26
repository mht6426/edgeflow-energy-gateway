/*
 * adapter.c — Device Adapter 注册表与聚合 poll
 *
 * 平台：Linux
 * 学习阶段：M3
 *
 * 线程安全：M3 假定单线程调用；M8 ingress 线程独占 registry 时需加锁或传递所有权。
 */

#include "ingress/adapter.h"

#include <string.h>

void adapter_registry_init(adapter_registry_t *reg)
{
    if (reg == NULL) {
        return;
    }
    memset(reg, 0, sizeof(*reg));
}

adapter_status_t adapter_registry_register(adapter_registry_t *reg, const device_adapter_t *adapter)
{
    if (reg == NULL || adapter == NULL) {
        return ADAPTER_ERR;
    }
    if (reg->count >= ADAPTER_REGISTRY_MAX) {
        return ADAPTER_ERR_FULL;
    }
    if (adapter->poll == NULL || adapter->init == NULL || adapter->shutdown == NULL) {
        return ADAPTER_ERR;
    }

    reg->slots[reg->count] = *adapter;
    reg->count++;
    return ADAPTER_OK;
}

device_adapter_t *adapter_registry_find(adapter_registry_t *reg, device_protocol_t protocol)
{
    if (reg == NULL) {
        return NULL;
    }
    for (size_t i = 0U; i < reg->count; i++) {
        if (reg->slots[i].protocol == protocol) {
            return &reg->slots[i];
        }
    }
    return NULL;
}

int adapter_registry_init_all(adapter_registry_t *reg, const gateway_config_t *cfg)
{
    int failures = 0;

    if (reg == NULL || cfg == NULL) {
        return -1;
    }

    for (size_t i = 0U; i < reg->count; i++) {
        device_adapter_t *slot = &reg->slots[i];
        if (slot->init != NULL && slot->init(slot, cfg) != 0) {
            failures++;
        }
    }
    return failures > 0 ? -1 : 0;
}

int adapter_registry_poll_all(adapter_registry_t *reg, telemetry_t *out, size_t max_out)
{
    size_t total = 0U;

    if (reg == NULL || out == NULL || max_out == 0U) {
        return 0;
    }

    for (size_t i = 0U; i < reg->count; i++) {
        device_adapter_t *slot = &reg->slots[i];
        int n;

        if (slot->poll == NULL || total >= max_out) {
            break;
        }

        n = slot->poll(slot, &out[total], max_out - total);
        if (n > 0) {
            total += (size_t)n;
        }
    }
    return (int)total;
}

void adapter_registry_shutdown_all(adapter_registry_t *reg)
{
    if (reg == NULL) {
        return;
    }
    for (size_t i = 0U; i < reg->count; i++) {
        device_adapter_t *slot = &reg->slots[i];
        if (slot->shutdown != NULL) {
            slot->shutdown(slot);
        }
    }
}
