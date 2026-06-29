/*
 * reactor.c — epoll + timerfd 周期定时器
 */

#include "core/reactor.h"

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <unistd.h>

#define REACTOR_MAX_EVENTS 8

typedef struct {
    reactor_timer_cb cb;
    void *user_data;
} reactor_timer_ctx_t;

struct reactor {
    int epfd;
    int timer_fd;
    reactor_timer_ctx_t timer_ctx;
    bool running;
};

int reactor_init(reactor_t **out)
{
    reactor_t *r;

    if (out == NULL) {
        return -1;
    }

    r = calloc(1, sizeof(*r));
    if (r == NULL) {
        return -1;
    }

    r->epfd = epoll_create1(EPOLL_CLOEXEC);
    if (r->epfd < 0) {
        free(r);
        return -1;
    }

    r->timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (r->timer_fd < 0) {
        close(r->epfd);
        free(r);
        return -1;
    }

    {
        struct epoll_event ev;
        memset(&ev, 0, sizeof(ev));
        ev.events = EPOLLIN;
        ev.data.fd = r->timer_fd;
        if (epoll_ctl(r->epfd, EPOLL_CTL_ADD, r->timer_fd, &ev) != 0) {
            close(r->timer_fd);
            close(r->epfd);
            free(r);
            return -1;
        }
    }

    r->running = true;
    *out = r;
    return 0;
}

void reactor_destroy(reactor_t *reactor)
{
    if (reactor == NULL) {
        return;
    }
    if (reactor->timer_fd >= 0) {
        close(reactor->timer_fd);
    }
    if (reactor->epfd >= 0) {
        close(reactor->epfd);
    }
    free(reactor);
}

int reactor_add_timer_ms(reactor_t *reactor, uint32_t interval_ms, reactor_timer_cb cb, void *user_data)
{
    struct itimerspec its;

    if (reactor == NULL || cb == NULL || interval_ms == 0U) {
        return -1;
    }

    reactor->timer_ctx.cb = cb;
    reactor->timer_ctx.user_data = user_data;

    memset(&its, 0, sizeof(its));
    its.it_value.tv_sec = (time_t)(interval_ms / 1000U);
    its.it_value.tv_nsec = (long)((interval_ms % 1000U) * 1000000L);
    its.it_interval = its.it_value;

    return timerfd_settime(reactor->timer_fd, 0, &its, NULL) == 0 ? 0 : -1;
}

int reactor_run_once(reactor_t *reactor, int timeout_ms)
{
    struct epoll_event events[REACTOR_MAX_EVENTS];
    int n;

    if (reactor == NULL || !reactor->running) {
        return -1;
    }

    n = epoll_wait(reactor->epfd, events, REACTOR_MAX_EVENTS, timeout_ms);
    if (n < 0) {
        return (errno == EINTR) ? 0 : -1;
    }

    for (int i = 0; i < n; i++) {
        if (events[i].data.fd == reactor->timer_fd) {
            uint64_t expirations;
            if (read(reactor->timer_fd, &expirations, sizeof(expirations)) > 0) {
                if (reactor->timer_ctx.cb != NULL) {
                    reactor->timer_ctx.cb(reactor->timer_ctx.user_data);
                }
            }
        }
    }
    return 0;
}

void reactor_stop(reactor_t *reactor)
{
    if (reactor != NULL) {
        reactor->running = false;
    }
}
