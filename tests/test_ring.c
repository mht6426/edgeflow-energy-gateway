#include "core/ring.h"

#include <assert.h>
#include <stdio.h>

int main(void)
{
    spsc_ring_t ring;
    data_point_t in = {0};
    data_point_t out = {0};

    assert(spsc_ring_init(&ring, 8U) == 0);
    snprintf(in.device_id, sizeof(in.device_id), "%s", "bms_001");
    snprintf(in.point_id, sizeof(in.point_id), "%s", "soc_percent");
    in.value = 42.0;
    assert(spsc_ring_push(&ring, &in));
    assert(spsc_ring_count(&ring) == 1U);
    assert(spsc_ring_pop(&ring, &out));
    assert(out.value == 42.0);
    assert(out.point_id[0] == 's');
    assert(spsc_ring_count(&ring) == 0U);
    spsc_ring_destroy(&ring);

    printf("test_ring passed\n");
    return 0;
}
