/*
 * watchdog.c — systemd sd_notify 兼容实现（NOTIFY_SOCKET）
 */

#include "platform/watchdog.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

static int g_notify_fd = -1;
static bool g_enabled;

static int open_notify_socket(void)
{
    const char *path = getenv("NOTIFY_SOCKET");
    struct sockaddr_un addr;

    if (path == NULL || path[0] == '\0') {
        return -1;
    }

    g_notify_fd = socket(AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    if (g_notify_fd < 0) {
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", path);
    if (connect(g_notify_fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        close(g_notify_fd);
        g_notify_fd = -1;
        return -1;
    }
    return 0;
}

static void send_notify(const char *msg)
{
    if (g_notify_fd < 0 || msg == NULL) {
        return;
    }
    (void)write(g_notify_fd, msg, strlen(msg));
}

void watchdog_init(uint32_t interval_ms)
{
    (void)interval_ms;
    g_enabled = (open_notify_socket() == 0);
}

void watchdog_notify_ready(void)
{
    if (!g_enabled) {
        return;
    }
    send_notify("READY=1\n");
}

void watchdog_notify_alive(void)
{
    if (!g_enabled) {
        return;
    }
    send_notify("WATCHDOG=1\n");
}

void watchdog_shutdown(void)
{
    if (g_notify_fd >= 0) {
        close(g_notify_fd);
        g_notify_fd = -1;
    }
    g_enabled = false;
}

bool watchdog_enabled(void)
{
    return g_enabled;
}
