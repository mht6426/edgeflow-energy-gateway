#ifndef EDGEFLOW_RING_H
#define EDGEFLOW_RING_H

/*
 * SPSC 无锁环形队列（M8）
 *
 * 平台：Linux + C11 atomic
 * 职责：ingress 线程 push、worker 线程 pop，解耦采集与处理。
 * 不负责：多生产者/多消费者（后续扩展需换队列类型）。
 */

#include "model/device_model.h"

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    telemetry_t *buf;
    uint32_t size;
    atomic_uint head;
    atomic_uint tail;
    atomic_uint dropped;
} spsc_ring_t;

int spsc_ring_init(spsc_ring_t *ring, uint32_t size);
void spsc_ring_destroy(spsc_ring_t *ring);
bool spsc_ring_push(spsc_ring_t *ring, const telemetry_t *point);
bool spsc_ring_pop(spsc_ring_t *ring, telemetry_t *point);
uint32_t spsc_ring_count(const spsc_ring_t *ring);
uint32_t spsc_ring_dropped(const spsc_ring_t *ring);

#endif
