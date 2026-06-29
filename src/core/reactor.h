#ifndef EDGEFLOW_REACTOR_H
#define EDGEFLOW_REACTOR_H

/*
 * 轻量 epoll reactor（进阶版）
 *
 * monitor 线程用 timerfd 周期唤醒，演示 Linux I/O 多路复用模式。
 */

#include <stdbool.h>
#include <stdint.h>

typedef struct reactor reactor_t;

typedef void (*reactor_timer_cb)(void *user_data);

int reactor_init(reactor_t **out);
void reactor_destroy(reactor_t *reactor);

int reactor_add_timer_ms(reactor_t *reactor, uint32_t interval_ms, reactor_timer_cb cb, void *user_data);
int reactor_run_once(reactor_t *reactor, int timeout_ms);
void reactor_stop(reactor_t *reactor);

#endif
