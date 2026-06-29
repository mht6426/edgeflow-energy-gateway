/*
 * test_storage.c — SQLite WAL 存储单元测试
 */

#include "platform/storage.h"
#include "platform/config.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
    gateway_config_t cfg;
    storage_ctx_t *ctx = NULL;
    telemetry_t point;
    storage_telemetry_row_t rows[8];
    size_t count = 0U;
    int64_t row_id = 0;

    gateway_config_defaults(&cfg);
    snprintf(cfg.sqlite_path, sizeof(cfg.sqlite_path), "/tmp/edgeflow-test-%d.db", getpid());
    cfg.use_sqlite = true;

    assert(storage_open(&ctx, &cfg) == 0);

    telemetry_init(&point, 1000U, "bms_001", "soc_percent", 75.5, "%", 1U);
    assert(storage_append_telemetry(ctx, &point, &row_id) == 0);
    assert(row_id > 0);

    count = 99U;
    assert(storage_fetch_pending_telemetry(ctx, rows, 8U, &count) >= 0);
    assert(count == 1U);
    assert(rows[0].row_id == row_id);

    assert(storage_mark_telemetry_uploaded(ctx, row_id) == 0);
    assert(storage_pending_telemetry_count(ctx) == 0U);

    storage_close(ctx);
    unlink(cfg.sqlite_path);

    printf("test_storage passed\n");
    return 0;
}
