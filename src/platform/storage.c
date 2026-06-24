#include "platform/storage.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

static void ensure_parent_dir(const char *path)
{
    char buf[256];
    char *slash;

    snprintf(buf, sizeof(buf), "%s", path);
    slash = strrchr(buf, '/');
    if (slash == NULL || slash == buf) {
        return;
    }
    *slash = '\0';
    (void)mkdir(buf, 0755);
}

int storage_append_point(const char *path, const data_point_t *point)
{
    FILE *fp;

    if (path == NULL || point == NULL) {
        return -1;
    }
    ensure_parent_dir(path);
    fp = fopen(path, "a");
    if (fp == NULL) {
        return -1;
    }
    fprintf(fp,
            "{\"type\":\"telemetry\",\"ts\":%llu,\"device_id\":\"%s\",\"point_id\":\"%s\",\"value\":%.3f,\"unit\":\"%s\",\"quality\":\"%s\",\"src\":%u}\n",
            (unsigned long long)point->ts_ms,
            point->device_id,
            point->point_id,
            point->value,
            point->unit,
            telemetry_quality_to_string(point->quality),
            point->source_id);
    fclose(fp);
    return 0;
}

int storage_append_alarm(const char *path, const alarm_event_t *alarm)
{
    FILE *fp;

    if (path == NULL || alarm == NULL) {
        return -1;
    }
    ensure_parent_dir(path);
    fp = fopen(path, "a");
    if (fp == NULL) {
        return -1;
    }
    fprintf(fp,
            "{\"type\":\"alarm\",\"alarm_id\":%llu,\"ts\":%llu,\"device_id\":\"%s\",\"code\":\"%s\",\"level\":%d,\"state\":%d,\"message\":\"%s\"}\n",
            (unsigned long long)alarm->alarm_id,
            (unsigned long long)alarm->ts_ms,
            alarm->device_id,
            alarm->code,
            (int)alarm->level,
            (int)alarm->state,
            alarm->message);
    fclose(fp);
    return 0;
}

int storage_append_command(const char *path, const command_t *command)
{
    FILE *fp;

    if (path == NULL || command == NULL) {
        return -1;
    }
    ensure_parent_dir(path);
    fp = fopen(path, "a");
    if (fp == NULL) {
        return -1;
    }
    fprintf(fp,
            "{\"type\":\"command\",\"command_id\":%llu,\"ts\":%llu,\"target_device_id\":\"%s\",\"point_id\":\"%s\",\"target_value\":%.3f,\"state\":\"%s\",\"retry_count\":%d,\"result\":\"%s\"}\n",
            (unsigned long long)command->command_id,
            (unsigned long long)command->created_ts_ms,
            command->target_device_id,
            command->point_id,
            command->target_value,
            command_state_to_string(command->state),
            command->retry_count,
            command->result_message);
    fclose(fp);
    return 0;
}
