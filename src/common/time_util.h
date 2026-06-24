#ifndef EDGEFLOW_TIME_UTIL_H
#define EDGEFLOW_TIME_UTIL_H

#include <stdint.h>

uint64_t edgeflow_now_ms(void);
void edgeflow_sleep_ms(uint32_t ms);

#endif
