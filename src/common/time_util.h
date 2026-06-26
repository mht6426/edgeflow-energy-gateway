#ifndef EDGEFLOW_TIME_UTIL_H
#define EDGEFLOW_TIME_UTIL_H

/*
 * 时间工具（common）
 *
 * 平台：Linux POSIX clock_gettime(CLOCK_MONOTONIC)
 *
 * 职责：提供单调毫秒时钟，供 Adapter 打 telemetry 时间戳、后续超时判断（M6）。
 * 不负责：墙钟/NTP 同步、时区转换。
 */

#include <stdint.h>

/* 单调时钟毫秒，适合间隔与排序，不受系统时间调整影响 */
uint64_t edgeflow_now_ms(void);

/* 线程休眠，后续 ingress 轮询间隔使用（M4/M8） */
void edgeflow_sleep_ms(uint32_t ms);

#endif
