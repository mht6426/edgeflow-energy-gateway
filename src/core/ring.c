#include "core/ring.h"

#include <stdlib.h>

static bool is_power_of_two(uint32_t value)
{
    return value != 0U && (value & (value - 1U)) == 0U;
}

int spsc_ring_init(spsc_ring_t *ring, uint32_t size)
{
    if (ring == NULL || !is_power_of_two(size)) {
        return -1;
    }

    ring->buf = calloc(size, sizeof(*ring->buf));
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

bool spsc_ring_push(spsc_ring_t *ring, const data_point_t *point)
{
    const uint32_t head = atomic_load_explicit(&ring->head, memory_order_relaxed);
    const uint32_t next = (head + 1U) & (ring->size - 1U);
    const uint32_t tail = atomic_load_explicit(&ring->tail, memory_order_acquire);

    if (next == tail) {
        atomic_fetch_add_explicit(&ring->dropped, 1U, memory_order_relaxed);
        return false;
    }

    ring->buf[head] = *point;
    atomic_store_explicit(&ring->head, next, memory_order_release);
    return true;
}

bool spsc_ring_pop(spsc_ring_t *ring, data_point_t *point)
{
    const uint32_t tail = atomic_load_explicit(&ring->tail, memory_order_relaxed);
    const uint32_t head = atomic_load_explicit(&ring->head, memory_order_acquire);

    if (tail == head) {
        return false;
    }

    *point = ring->buf[tail];
    atomic_store_explicit(&ring->tail, (tail + 1U) & (ring->size - 1U), memory_order_release);
    return true;
}

uint32_t spsc_ring_count(const spsc_ring_t *ring)
{
    const uint32_t head = atomic_load_explicit(&ring->head, memory_order_acquire);
    const uint32_t tail = atomic_load_explicit(&ring->tail, memory_order_acquire);
    return (head - tail) & (ring->size - 1U);
}

uint32_t spsc_ring_dropped(const spsc_ring_t *ring)
{
    return atomic_load_explicit(&ring->dropped, memory_order_relaxed);
}
