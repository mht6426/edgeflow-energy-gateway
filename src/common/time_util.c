/*
 * time_util.c — POSIX 单调时钟与 sleep
 *
 * 平台：Linux（_POSIX_C_SOURCE=200809L）
 */

#include "common/time_util.h"

#include <time.h>

uint64_t edgeflow_now_ms(void)
{
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return 0U;
    }
    return (uint64_t)ts.tv_sec * 1000U + (uint64_t)ts.tv_nsec / 1000000U;
}

void edgeflow_sleep_ms(uint32_t ms)
{
    struct timespec req;
    struct timespec rem;

    req.tv_sec = (time_t)(ms / 1000U);
    req.tv_nsec = (long)(ms % 1000U) * 1000000L;
    while (nanosleep(&req, &rem) != 0) {
        req = rem;
    }
}
