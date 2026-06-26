/*
 * main.c — EdgeFlow 进程入口
 *
 * 平台：Linux（默认执行环境）
 *
 * 学习阶段：M3（Device Adapter 插件接口）
 *
 * 启动流程：
 *   1. 解析 -c / --config
 *   2. gateway_config_load
 *   3. logger_init
 *   4. 注册 stub Adapter → init_all → poll_all → 日志打印 telemetry
 *   5. adapter shutdown + logger_shutdown
 *
 * 尚未实现：M8 轮询线程、M5 状态机、M9 MQTT
 */

#include "ingress/adapter.h"
#include "ingress/stub_adapter.h"
#include "platform/config.h"
#include "platform/logger.h"

#include <stdio.h>
#include <string.h>

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
    adapter_registry_t registry;
    device_adapter_t stub;
    telemetry_t points[16];
    int n;

    if (gateway_config_load(config_path, &cfg) != 0) {
        fprintf(stderr, "failed to load config: %s\n", config_path);
        return 1;
    }

    if (logger_init(cfg.log_dir) != 0) {
        fprintf(stderr, "failed to init logger, fallback to stderr (dir=%s)\n", cfg.log_dir);
    }

    log_info("edgeflow M3 startup: device_id=%s config=%s simulate=%s",
             cfg.device_id,
             config_path,
             cfg.simulate ? "true" : "false");

    /* --- M3：注册并初始化 Adapter 插件 --- */
    adapter_registry_init(&registry);
    stub_adapter_fill(&stub);
    if (adapter_registry_register(&registry, &stub) != ADAPTER_OK) {
        log_error("failed to register stub adapter");
        logger_shutdown();
        return 1;
    }
    if (adapter_registry_init_all(&registry, &cfg) != 0) {
        log_warn("one or more adapters failed init");
    }

    /* 单轮采集演示；M8 将在 ingress 线程中按 poll_interval_ms 循环调用 */
    n = adapter_registry_poll_all(&registry, points, 16U);
    log_info("adapter poll: telemetry_count=%d", n);
    for (int i = 0; i < n; i++) {
        log_info("  %s.%s=%.2f %s quality=%s ts_ms=%llu",
                 points[i].device_id,
                 points[i].point_id,
                 points[i].value,
                 points[i].unit,
                 points[i].quality == TELEMETRY_QUALITY_GOOD ? "good" : "bad",
                 (unsigned long long)points[i].ts_ms);
    }

    printf("EdgeFlow M3 OK — polled %d telemetry point(s), log_dir=%s\n", n, cfg.log_dir);

    adapter_registry_shutdown_all(&registry);
    logger_shutdown();
    return 0;
}
