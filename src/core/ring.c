/*
 * ring.c — SPSC 无锁队列实现
 *
 * 单生产者单消费者；满时 push 失败并递增 dropped 计数。
 */

#include "core/ring.h"

#include <stdlib.h>
#include <string.h>

int spsc_ring_init(spsc_ring_t *ring, uint32_t size)
{
    if (ring == NULL || size < 2U) {
        return -1;
    }
    ring->buf = calloc(size, sizeof(telemetry_t));
    if (ring->buf == NULL) {
        return -1;
    }
    ring->size = size;
    atomic_init(&ring->head, 0U);
    atomic_init(&ring->tail, 0U);
    atomic_init(&ring->dropped, 0U);
    return 0;
}

void spsc_ring_destroy(spsc_ring_t *ring)
{
    if (ring == NULL) {
        return;
    }
    free(ring->buf);
    ring->buf = NULL;
    ring->size = 0U;
}

bool spsc_ring_push(spsc_ring_t *ring, const telemetry_t *point)
{
    uint32_t head;
    uint32_t next;
    uint32_t tail;

    if (ring == NULL || point == NULL || ring->buf == NULL) {
        return false;
    }

    head = atomic_load(&ring->head);
    next = (head + 1U) % ring->size;
    tail = atomic_load(&ring->tail);
    if (next == tail) {
        atomic_fetch_add(&ring->dropped, 1U);
        return false;
    }

    ring->buf[head] = *point;
    atomic_store(&ring->head, next);
    return true;
}

bool spsc_ring_pop(spsc_ring_t *ring, telemetry_t *point)
{
    uint32_t tail;
    uint32_t head;

    if (ring == NULL || point == NULL || ring->buf == NULL) {
        return false;
    }

    tail = atomic_load(&ring->tail);
    head = atomic_load(&ring->head);
    if (tail == head) {
        return false;
    }

    *point = ring->buf[tail];
    atomic_store(&ring->tail, (tail + 1U) % ring->size);
    return true;
}

uint32_t spsc_ring_count(const spsc_ring_t *ring)
{
    uint32_t head;
    uint32_t tail;

    if (ring == NULL || ring->buf == NULL) {
        return 0U;
    }
    head = atomic_load(&ring->head);
    tail = atomic_load(&ring->tail);
    if (head >= tail) {
        return head - tail;
    }
    return ring->size - tail + head;
}

uint32_t spsc_ring_dropped(const spsc_ring_t *ring)
{
    if (ring == NULL) {
        return 0U;
    }
    return atomic_load(&ring->dropped);
}
