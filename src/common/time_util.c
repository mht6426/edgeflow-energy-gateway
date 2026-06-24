#include "common/time_util.h"

#include <errno.h>
#include <time.h>

uint64_t edgeflow_now_ms(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) != 0) {
        return 0;
    }
    return (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)ts.tv_nsec / 1000000ULL;
}

void edgeflow_sleep_ms(uint32_t ms)
{
    struct timespec req;
    req.tv_sec = ms / 1000U;
    req.tv_nsec = (long)(ms % 1000U) * 1000000L;

    while (nanosleep(&req, &req) != 0 && errno == EINTR) {
    }
}
