#ifndef EDGEFLOW_HEARTBEAT_H
#define EDGEFLOW_HEARTBEAT_H

/*
 * 线程心跳注册表（进阶版）
 *
 * 各工作线程定期 touch，monitor 检测 stale 并写入 metrics。
 */

#include <stdatomic.h>
#include <stdint.h>

typedef enum {
    HEARTBEAT_THREAD_INGRESS = 0,
    HEARTBEAT_THREAD_WORKER,
    HEARTBEAT_THREAD_MONITOR,
    HEARTBEAT_THREAD_COUNT
} heartbeat_thread_t;

typedef struct {
    atomic_ullong last_ms[HEARTBEAT_THREAD_COUNT];
} heartbeat_registry_t;

void heartbeat_registry_init(heartbeat_registry_t *hb);
void heartbeat_touch(heartbeat_registry_t *hb, heartbeat_thread_t thread);
uint64_t heartbeat_age_ms(const heartbeat_registry_t *hb, heartbeat_thread_t thread);

#endif
