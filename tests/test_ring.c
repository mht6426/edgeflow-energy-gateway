/*
 * test_ring.c — SPSC 队列单元测试
 */

#include "core/ring.h"
#include "model/device_model.h"

#include <assert.h>
#include <stdio.h>

int main(void)
{
    spsc_ring_t ring;
    telemetry_t in;
    telemetry_t out;

    assert(spsc_ring_init(&ring, 4U) == 0);
    telemetry_init(&in, 1U, "d1", "p1", 1.0, "U", 1U);

    assert(spsc_ring_push(&ring, &in));
    assert(spsc_ring_pop(&ring, &out));
    assert(out.value == 1.0);

    spsc_ring_destroy(&ring);
    printf("test_ring passed\n");
    return 0;
}
