#include "platform/logger.h"

#include "common/time_util.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

static FILE *g_log;

static void log_write(const char *level, const char *fmt, va_list ap)
{
    FILE *out = g_log != NULL ? g_log : stderr;
    fprintf(out, "[%llu] %-5s ", (unsigned long long)edgeflow_now_ms(), level);
    vfprintf(out, fmt, ap);
    fputc('\n', out);
    fflush(out);
}

int logger_init(const char *dir)
{
    char path[256];

    if (dir == NULL || dir[0] == '\0') {
        return -1;
    }
    (void)mkdir(dir, 0755);
    snprintf(path, sizeof(path), "%s/edgeflow.log", dir);
    g_log = fopen(path, "a");
    if (g_log == NULL) {
        return -1;
    }
    return 0;
}

void logger_shutdown(void)
{
    if (g_log != NULL) {
        fclose(g_log);
        g_log = NULL;
    }
}

void log_info(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    log_write("INFO", fmt, ap);
    va_end(ap);
}

void log_warn(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    log_write("WARN", fmt, ap);
    va_end(ap);
}

void log_error(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    log_write("ERROR", fmt, ap);
    va_end(ap);
}
