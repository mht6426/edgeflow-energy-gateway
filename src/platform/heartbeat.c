/*
 * heartbeat.c — 线程存活时间戳
 */

#include "platform/heartbeat.h"

#include "common/time_util.h"

void heartbeat_registry_init(heartbeat_registry_t *hb)
{
    uint64_t now;

    if (hb == NULL) {
        return;
    }
    now = edgeflow_now_ms();
    for (int i = 0; i < HEARTBEAT_THREAD_COUNT; i++) {
        atomic_init(&hb->last_ms[i], now);
    }
}

void heartbeat_touch(heartbeat_registry_t *hb, heartbeat_thread_t thread)
{
    if (hb == NULL || thread < 0 || thread >= HEARTBEAT_THREAD_COUNT) {
        return;
    }
    atomic_store(&hb->last_ms[thread], edgeflow_now_ms());
}

uint64_t heartbeat_age_ms(const heartbeat_registry_t *hb, heartbeat_thread_t thread)
{
    uint64_t now;
    uint64_t last;

    if (hb == NULL || thread < 0 || thread >= HEARTBEAT_THREAD_COUNT) {
        return UINT64_MAX;
    }
    now = edgeflow_now_ms();
    last = atomic_load(&hb->last_ms[thread]);
    return now >= last ? now - last : 0U;
}
