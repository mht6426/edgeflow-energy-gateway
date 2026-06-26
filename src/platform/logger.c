/*
 * logger.c — 轻量文件日志实现（M2）
 *
 * 平台：Linux only
 *   - 使用 POSIX mkdir(2) 创建日志目录
 *   - 日志文件：<log_dir>/edgeflow.log，追加模式 "a"
 *
 * 全局状态：
 *   g_log_fp — 日志文件句柄；NULL 表示回退 stderr
 *
 * 线程安全：
 *   M2 仅 main 单线程调用；M8 多线程运行时必须加 Mutex。
 */

#include "platform/logger.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

/* 进程内唯一日志文件句柄；NULL 时所有日志写 stderr */
static FILE *g_log_fp;

/*
 * 内部统一输出：带级别前缀，立即 fflush 便于 tail -f 实时查看。
 */
static void log_vprint(const char *level, const char *fmt, va_list ap)
{
    FILE *out = g_log_fp != NULL ? g_log_fp : stderr;

    fprintf(out, "[%s] ", level);
    vfprintf(out, fmt, ap);
    fprintf(out, "\n");
    fflush(out);
}

int logger_init(const char *log_dir)
{
    char path[256];

    if (log_dir == NULL || log_dir[0] == '\0') {
        return -1;
    }

    /*
     * mode 0755：所有者 rwx，组/其他 rx。
     * 目录已存在时 mkdir 返回 -1 可忽略，后续 fopen 仍可能成功。
     */
    (void)mkdir(log_dir, 0755);
    snprintf(path, sizeof(path), "%s/edgeflow.log", log_dir);

    g_log_fp = fopen(path, "a");
    if (g_log_fp == NULL) {
        return -1;
    }
    return 0;
}

void logger_shutdown(void)
{
    if (g_log_fp != NULL) {
        fclose(g_log_fp);
        g_log_fp = NULL;
    }
}

void log_info(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    log_vprint("INFO", fmt, ap);
    va_end(ap);
}

void log_warn(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    log_vprint("WARN", fmt, ap);
    va_end(ap);
}

void log_error(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    log_vprint("ERROR", fmt, ap);
    va_end(ap);
}
