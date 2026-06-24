#include "common/time_util.h"
#include "platform/config.h"
#include "platform/logger.h"
#include "runtime/app.h"

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdatomic.h>

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
    atomic_init(&g_exit_requested, false);

    if (gateway_config_load(config_path, &cfg) != 0) {
        fprintf(stderr, "failed to load config: %s\n", config_path);
        return 1;
    }
    if (logger_init(cfg.log_dir) != 0) {
        fprintf(stderr, "failed to init logger, fallback to stderr\n");
    }
    if (gateway_app_init(&app, &cfg) != 0) {
        log_error("failed to init edgeflow");
        logger_shutdown();
        return 1;
    }
    if (gateway_app_run(&app) != 0) {
        log_error("failed to run edgeflow");
        gateway_app_destroy(&app);
        logger_shutdown();
        return 1;
    }

    while (!atomic_load(&g_exit_requested)) {
        edgeflow_sleep_ms(200U);
    }

    gateway_app_stop(&app);
    gateway_app_destroy(&app);
    logger_shutdown();
    return 0;
}
