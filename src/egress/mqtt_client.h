#ifndef EDGEFLOW_MQTT_CLIENT_H
#define EDGEFLOW_MQTT_CLIENT_H

#include "common/datapoint.h"
#include "platform/config.h"

int mqtt_publish_point(const gateway_config_t *cfg, const data_point_t *point);

#endif
