/*
 * edgeflow-cli — 运维命令行工具（进阶版）
 *
 * 子命令：
 *   validate-config -c <path>
 *   status --metrics <path>
 *   storage-stats --sqlite <path>
 */

#include "platform/config.h"

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_usage(const char *prog)
{
    fprintf(stderr,
            "Usage:\n"
            "  %s validate-config -c <gateway.json>\n"
            "  %s status --metrics <metrics.prom>\n"
            "  %s storage-stats --sqlite <edgeflow.db>\n",
            prog,
            prog,
            prog);
}

static int cmd_validate_config(int argc, char **argv)
{
    const char *path = "configs/gateway.json";
    gateway_config_t cfg;

    for (int i = 2; i + 1 < argc; i++) {
        if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--config") == 0) {
            path = argv[i + 1];
        }
    }

    if (gateway_config_load(path, &cfg) != 0) {
        fprintf(stderr, "validate-config: load failed: %s\n", path);
        return 1;
    }

    printf("validate-config OK: %s\n", path);
    printf("  device_id=%s simulate=%s\n", cfg.device_id, cfg.simulate ? "true" : "false");
    printf("  broker=%s:%u topic=%s\n", cfg.broker_host, cfg.broker_port, cfg.mqtt_topic);
    printf("  sqlite=%s use_sqlite=%s\n", cfg.sqlite_path, cfg.use_sqlite ? "true" : "false");
    return 0;
}

static int cmd_status(int argc, char **argv)
{
    const char *metrics_path = NULL;
    char line[512];

    for (int i = 2; i + 1 < argc; i++) {
        if (strcmp(argv[i], "--metrics") == 0) {
            metrics_path = argv[i + 1];
        }
    }
    if (metrics_path == NULL) {
        fprintf(stderr, "status: missing --metrics\n");
        return 1;
    }

    FILE *fp = fopen(metrics_path, "r");
    if (fp == NULL) {
        fprintf(stderr, "status: cannot open %s\n", metrics_path);
        return 1;
    }

    printf("=== edgeflow status (%s) ===\n", metrics_path);
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (line[0] != '#') {
            fputs(line, stdout);
        }
    }
    fclose(fp);
    return 0;
}

static int cmd_storage_stats(int argc, char **argv)
{
    const char *db_path = NULL;
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;

    for (int i = 2; i + 1 < argc; i++) {
        if (strcmp(argv[i], "--sqlite") == 0) {
            db_path = argv[i + 1];
        }
    }
    if (db_path == NULL) {
        fprintf(stderr, "storage-stats: missing --sqlite\n");
        return 1;
    }

    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        fprintf(stderr, "storage-stats: open failed: %s\n", db_path);
        return 1;
    }

    printf("=== storage stats (%s) ===\n", db_path);

    const char *queries[] = {
        "SELECT COUNT(*) FROM telemetry;",
        "SELECT COUNT(*) FROM telemetry WHERE uploaded=0;",
        "SELECT COUNT(*) FROM alarms;",
        "SELECT COUNT(*) FROM commands;",
    };
    const char *labels[] = {
        "telemetry_total",
        "telemetry_pending",
        "alarms_total",
        "commands_total",
    };

    for (size_t i = 0U; i < sizeof(queries) / sizeof(queries[0]); i++) {
        if (sqlite3_prepare_v2(db, queries[i], -1, &stmt, NULL) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                printf("%s %lld\n", labels[i], (long long)sqlite3_column_int64(stmt, 0));
            }
            sqlite3_finalize(stmt);
            stmt = NULL;
        }
    }

    sqlite3_close(db);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "validate-config") == 0) {
        return cmd_validate_config(argc, argv);
    }
    if (strcmp(argv[1], "status") == 0) {
        return cmd_status(argc, argv);
    }
    if (strcmp(argv[1], "storage-stats") == 0) {
        return cmd_storage_stats(argc, argv);
    }

    print_usage(argv[0]);
    return 1;
}
