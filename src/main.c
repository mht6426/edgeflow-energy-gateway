/*
 * main.c — EdgeFlow 进程入口（完整运行时 M3–M10 集成）
 *
 * 平台：Linux
 *
 * 流程：配置 → 日志 → gateway_app 双线程运行 → SIGINT 优雅退出
 */

#include "common/time_util.h"
#include "platform/config.h"
#include "platform/logger.h"
#include "runtime/app.h"

#include <signal.h>
#include <stdio.h>
#include <stdatomic.h>
#include <string.h>

static atomic_bool g_exit_requested;

static void on_signal(int signo)
{
    (void)signo;
    atomic_store(&g_exit_requested, true);
}

static const char *parse_config_path(int argc, char **argv)
{
    for (int i = 1; i + 1 < argc; i++) {
        if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--config") == 0) {
            return argv[i + 1];
        }
    }
    return "configs/gateway.json";
}

int main(int argc, char **argv)
{
    const char *config_path = parse_config_path(argc, argv);
    gateway_config_t cfg;
    gateway_app_t app;

    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);
    signal(SIGHUP, on_signal);
    atomic_init(&g_exit_requested, false);

    if (gateway_config_load(config_path, &cfg) != 0) {
        fprintf(stderr, "failed to load config: %s\n", config_path);
        return 1;
    }

    if (logger_init(cfg.log_dir) != 0) {
        fprintf(stderr, "failed to init logger, fallback stderr (dir=%s)\n", cfg.log_dir);
    }

    log_info("edgeflow startup: config=%s device_id=%s", config_path, cfg.device_id);

    if (gateway_app_init(&app, &cfg) != 0) {
        log_error("gateway_app_init failed");
        logger_shutdown();
        return 1;
    }
    if (gateway_app_run(&app) != 0) {
        log_error("gateway_app_run failed");
        gateway_app_destroy(&app);
        logger_shutdown();
        return 1;
    }

    printf("EdgeFlow running — Ctrl+C to stop\n");
    printf("  log:      %s/edgeflow.log\n", cfg.log_dir);
    printf("  metrics:  %s\n", cfg.metrics_path);
    printf("  cache:    %s\n", cfg.cache_path);
    printf("  sqlite:   %s (use_sqlite=%s)\n", cfg.sqlite_path, cfg.use_sqlite ? "true" : "false");

    while (!atomic_load(&g_exit_requested)) {
        edgeflow_sleep_ms(200U);
    }

    gateway_app_stop(&app);
    gateway_app_destroy(&app);
    logger_shutdown();
    return 0;
}
