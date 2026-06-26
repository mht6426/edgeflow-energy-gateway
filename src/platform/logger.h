#ifndef EDGEFLOW_LOGGER_H
#define EDGEFLOW_LOGGER_H

/*
 * Logger（M2）— 轻量文件日志
 *
 * 平台：Linux（日志目录使用 POSIX mkdir；路径如 /tmp/edgeflow、/var/log/edgeflow）
 *
 * 职责：
 *   - 将 INFO/WARN/ERROR 写入 <log_dir>/edgeflow.log
 *   - logger_init 失败时自动回退 stderr，不阻断进程启动
 *
 * 不负责：
 *   - 日志轮转（logrotate 由部署层处理，见 deploy/）
 *   - 远程 syslog、结构化 JSON 日志（M10 可扩展）
 *   - 多线程并发写（M8 引入 pthread 后需 Mutex 保护 g_log_fp）
 *
 * 日志格式（当前）：
 *   [LEVEL] message\n
 *   示例：[INFO] edgeflow M2 startup: device_id=gw-001 ...
 *
 * 典型用法：
 *   logger_init(cfg.log_dir);
 *   log_info("...");
 *   logger_shutdown();
 */

#include <stdarg.h>

/*
 * 初始化日志模块，创建目录并追加打开 edgeflow.log。
 * @param log_dir Linux 目录路径，不可为 NULL 或空串
 * @return 0 成功，-1 失败（调用方可用 stderr 继续）
 */
int logger_init(const char *log_dir);

/* 关闭日志文件句柄，进程退出前调用 */
void logger_shutdown(void);

/* 格式化日志输出，用法同 printf */
void log_info(const char *fmt, ...);
void log_warn(const char *fmt, ...);
void log_error(const char *fmt, ...);

#endif
