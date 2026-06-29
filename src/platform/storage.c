/*
 * storage.c — SQLite WAL + JSONL 审计（进阶版）
 */

#include "platform/storage.h"

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct storage_ctx {
    gateway_config_t cfg;
    sqlite3 *db;
};

static int append_jsonl_line(const char *path, const char *line)
{
    FILE *fp = fopen(path, "a");
    if (fp == NULL) {
        return -1;
    }
    fputs(line, fp);
    fputs("\n", fp);
    fclose(fp);
    return 0;
}

static int jsonl_append_telemetry(const char *path, const telemetry_t *point)
{
    char line[512];
    snprintf(line,
             sizeof(line),
             "{\"type\":\"telemetry\",\"ts_ms\":%llu,\"device_id\":\"%s\",\"point_id\":\"%s\","
             "\"value\":%.4f,\"unit\":\"%s\",\"quality\":\"%s\"}",
             (unsigned long long)point->ts_ms,
             point->device_id,
             point->point_id,
             point->value,
             point->unit,
             telemetry_quality_to_string(point->quality));
    return append_jsonl_line(path, line);
}

static int sqlite_exec_schema(sqlite3 *db)
{
    const char *sql =
        "PRAGMA journal_mode=WAL;"
        "CREATE TABLE IF NOT EXISTS telemetry ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  ts_ms INTEGER NOT NULL,"
        "  device_id TEXT NOT NULL,"
        "  point_id TEXT NOT NULL,"
        "  value REAL NOT NULL,"
        "  unit TEXT,"
        "  quality TEXT NOT NULL,"
        "  uploaded INTEGER NOT NULL DEFAULT 0"
        ");"
        "CREATE INDEX IF NOT EXISTS idx_telemetry_upload ON telemetry(uploaded, id);"
        "CREATE TABLE IF NOT EXISTS alarms ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  alarm_id INTEGER, ts_ms INTEGER, device_id TEXT, code TEXT, level INTEGER,"
        "  state TEXT, message TEXT, uploaded INTEGER DEFAULT 0"
        ");"
        "CREATE TABLE IF NOT EXISTS commands ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  command_id INTEGER, target TEXT, point_id TEXT, value REAL,"
        "  state TEXT, message TEXT, uploaded INTEGER DEFAULT 0"
        ");";

    char *err = NULL;
    if (sqlite3_exec(db, sql, NULL, NULL, &err) != SQLITE_OK) {
        if (err != NULL) {
            sqlite3_free(err);
        }
        return -1;
    }
    return 0;
}

int storage_open(storage_ctx_t **out, const gateway_config_t *cfg)
{
    storage_ctx_t *ctx;
    int rc;

    if (out == NULL || cfg == NULL) {
        return -1;
    }

    ctx = calloc(1, sizeof(*ctx));
    if (ctx == NULL) {
        return -1;
    }
    ctx->cfg = *cfg;

    if (!cfg->use_sqlite) {
        *out = ctx;
        return 0;
    }

    rc = sqlite3_open(cfg->sqlite_path, &ctx->db);
    if (rc != SQLITE_OK) {
        free(ctx);
        return -1;
    }
    if (sqlite_exec_schema(ctx->db) != 0) {
        sqlite3_close(ctx->db);
        free(ctx);
        return -1;
    }

    *out = ctx;
    return 0;
}

void storage_close(storage_ctx_t *ctx)
{
    if (ctx == NULL) {
        return;
    }
    if (ctx->db != NULL) {
        sqlite3_close(ctx->db);
    }
    free(ctx);
}

int storage_append_telemetry(storage_ctx_t *ctx, const telemetry_t *point, int64_t *row_id)
{
    sqlite3_stmt *stmt;
    int rc;

    if (ctx == NULL || point == NULL) {
        return -1;
    }

    (void)jsonl_append_telemetry(ctx->cfg.cache_path, point);

    if (!ctx->cfg.use_sqlite || ctx->db == NULL) {
        if (row_id != NULL) {
            *row_id = 0;
        }
        return 0;
    }

    const char *sql =
        "INSERT INTO telemetry(ts_ms,device_id,point_id,value,unit,quality,uploaded)"
        " VALUES(?,?,?,?,?,?,0);";
    rc = sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        return -1;
    }
    sqlite3_bind_int64(stmt, 1, (sqlite3_int64)point->ts_ms);
    sqlite3_bind_text(stmt, 2, point->device_id, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, point->point_id, -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 4, point->value);
    sqlite3_bind_text(stmt, 5, point->unit, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, telemetry_quality_to_string(point->quality), -1, SQLITE_TRANSIENT);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        return -1;
    }
    if (row_id != NULL) {
        *row_id = sqlite3_last_insert_rowid(ctx->db);
    }
    return 0;
}

int storage_append_point(storage_ctx_t *ctx, const telemetry_t *point)
{
    return storage_append_telemetry(ctx, point, NULL);
}

int storage_append_alarm(storage_ctx_t *ctx, const alarm_event_t *alarm)
{
    sqlite3_stmt *stmt;
    char line[640];

    if (ctx == NULL || alarm == NULL) {
        return -1;
    }

    snprintf(line,
             sizeof(line),
             "{\"type\":\"alarm\",\"code\":\"%s\",\"device_id\":\"%s\"}",
             alarm->code,
             alarm->device_id);
    (void)append_jsonl_line(ctx->cfg.cache_path, line);

    if (!ctx->cfg.use_sqlite || ctx->db == NULL) {
        return 0;
    }

    const char *sql =
        "INSERT INTO alarms(alarm_id,ts_ms,device_id,code,level,state,message,uploaded)"
        " VALUES(?,?,?,?,?,?,?,0);";
    if (sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return -1;
    }
    sqlite3_bind_int64(stmt, 1, (sqlite3_int64)alarm->alarm_id);
    sqlite3_bind_int64(stmt, 2, (sqlite3_int64)alarm->ts_ms);
    sqlite3_bind_text(stmt, 3, alarm->device_id, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, alarm->code, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, (int)alarm->level);
    sqlite3_bind_text(stmt, 6, alarm->state == ALARM_STATE_ACTIVE ? "ACTIVE" : "RECOVERED", -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, alarm->message, -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return -1;
    }
    sqlite3_finalize(stmt);
    return 0;
}

int storage_append_command(storage_ctx_t *ctx, const command_t *command)
{
    sqlite3_stmt *stmt;
    char line[640];

    if (ctx == NULL || command == NULL) {
        return -1;
    }

    snprintf(line,
             sizeof(line),
             "{\"type\":\"command\",\"id\":%llu,\"state\":\"%s\"}",
             (unsigned long long)command->command_id,
             command_state_to_string(command->state));
    (void)append_jsonl_line(ctx->cfg.cache_path, line);

    if (!ctx->cfg.use_sqlite || ctx->db == NULL) {
        return 0;
    }

    const char *sql =
        "INSERT INTO commands(command_id,target,point_id,value,state,message,uploaded)"
        " VALUES(?,?,?,?,?,?,0);";
    if (sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return -1;
    }
    sqlite3_bind_int64(stmt, 1, (sqlite3_int64)command->command_id);
    sqlite3_bind_text(stmt, 2, command->target_device_id, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, command->point_id, -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 4, command->target_value);
    sqlite3_bind_text(stmt, 5, command_state_to_string(command->state), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, command->result_message, -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return -1;
    }
    sqlite3_finalize(stmt);
    return 0;
}

int storage_fetch_pending_telemetry(storage_ctx_t *ctx,
                                    storage_telemetry_row_t *rows,
                                    size_t max_rows,
                                    size_t *out_count)
{
    sqlite3_stmt *stmt;
    size_t n = 0U;

    if (out_count != NULL) {
        *out_count = 0U;
    }
    if (ctx == NULL || rows == NULL || max_rows == 0U || !ctx->cfg.use_sqlite || ctx->db == NULL) {
        return 0;
    }

    const char *sql =
        "SELECT id,ts_ms,device_id,point_id,value,unit,quality FROM telemetry "
        "WHERE uploaded=0 ORDER BY id ASC LIMIT ?;";
    if (sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return -1;
    }
    sqlite3_bind_int64(stmt, 1, (sqlite3_int64)max_rows);

    while (sqlite3_step(stmt) == SQLITE_ROW && n < max_rows) {
        rows[n].row_id = sqlite3_column_int64(stmt, 0);
        telemetry_init(&rows[n].point,
                       (uint64_t)sqlite3_column_int64(stmt, 1),
                       (const char *)sqlite3_column_text(stmt, 2),
                       (const char *)sqlite3_column_text(stmt, 3),
                       sqlite3_column_double(stmt, 4),
                       (const char *)sqlite3_column_text(stmt, 5),
                       0U);
        rows[n].point.quality = TELEMETRY_QUALITY_GOOD;
        n++;
    }
    sqlite3_finalize(stmt);
    if (out_count != NULL) {
        *out_count = n;
    }
    return (int)n;
}

int storage_mark_telemetry_uploaded(storage_ctx_t *ctx, int64_t row_id)
{
    sqlite3_stmt *stmt;

    if (ctx == NULL || row_id <= 0 || ctx->db == NULL) {
        return -1;
    }

    const char *sql = "UPDATE telemetry SET uploaded=1 WHERE id=?;";
    if (sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return -1;
    }
    sqlite3_bind_int64(stmt, 1, row_id);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return -1;
    }
    sqlite3_finalize(stmt);
    return 0;
}

uint64_t storage_pending_telemetry_count(storage_ctx_t *ctx)
{
    sqlite3_stmt *stmt;
    uint64_t count = 0U;

    if (ctx == NULL || ctx->db == NULL) {
        return 0U;
    }

    const char *sql = "SELECT COUNT(*) FROM telemetry WHERE uploaded=0;";
    if (sqlite3_prepare_v2(ctx->db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0U;
    }
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = (uint64_t)sqlite3_column_int64(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return count;
}
