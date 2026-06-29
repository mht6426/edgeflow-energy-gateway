#ifndef EDGEFLOW_WATCHDOG_H
#define EDGEFLOW_WATCHDOG_H

/*
 * systemd Watchdog（进阶版）
 *
 * 通过 NOTIFY_SOCKET 发送 READY / WATCHDOG=1，无需链接 libsystemd。
 */

#include <stdbool.h>
#include <stdint.h>

void watchdog_init(uint32_t interval_ms);
void watchdog_notify_ready(void);
void watchdog_notify_alive(void);
void watchdog_shutdown(void);
bool watchdog_enabled(void);

#endif
