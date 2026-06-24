#ifndef EDGEFLOW_STORAGE_H
#define EDGEFLOW_STORAGE_H

#include "common/datapoint.h"

int storage_append_point(const char *path, const data_point_t *point);
int storage_append_alarm(const char *path, const alarm_event_t *alarm);
int storage_append_command(const char *path, const command_t *command);

#endif
