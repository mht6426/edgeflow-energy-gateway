#ifndef EDGEFLOW_RING_H
#define EDGEFLOW_RING_H

#include "common/datapoint.h"

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    data_point_t *buf;
    uint32_t size;
    atomic_uint head;
    atomic_uint tail;
    atomic_uint dropped;
} spsc_ring_t;

int spsc_ring_init(spsc_ring_t *ring, uint32_t size);
void spsc_ring_destroy(spsc_ring_t *ring);
bool spsc_ring_push(spsc_ring_t *ring, const data_point_t *point);
bool spsc_ring_pop(spsc_ring_t *ring, data_point_t *point);
uint32_t spsc_ring_count(const spsc_ring_t *ring);
uint32_t spsc_ring_dropped(const spsc_ring_t *ring);

#endif
