#ifndef EDGEFLOW_STORAGE_H
#define EDGEFLOW_STORAGE_H

/*
 * 存储层（进阶版 M7）
 *
 * 支持：
 *   - SQLite WAL（主路径，uploaded 标记 + 补传游标）
 *   - JSONL 审计追加（cache_path，与 SQLite 并行可选）
 */

#include "model/device_model.h"
#include "platform/config.h"

#include <stddef.h>
#include <stdint.h>

typedef struct storage_ctx storage_ctx_t;

typedef struct {
    int64_t row_id;
    telemetry_t point;
} storage_telemetry_row_t;

int storage_open(storage_ctx_t **out, const gateway_config_t *cfg);
void storage_close(storage_ctx_t *ctx);

int storage_append_telemetry(storage_ctx_t *ctx, const telemetry_t *point, int64_t *row_id);
int storage_append_alarm(storage_ctx_t *ctx, const alarm_event_t *alarm);
int storage_append_command(storage_ctx_t *ctx, const command_t *command);

int storage_fetch_pending_telemetry(storage_ctx_t *ctx,
                                    storage_telemetry_row_t *rows,
                                    size_t max_rows,
                                    size_t *out_count);
int storage_mark_telemetry_uploaded(storage_ctx_t *ctx, int64_t row_id);
uint64_t storage_pending_telemetry_count(storage_ctx_t *ctx);

/* 兼容旧接口名 */
int storage_append_point(storage_ctx_t *ctx, const telemetry_t *point);

#endif
