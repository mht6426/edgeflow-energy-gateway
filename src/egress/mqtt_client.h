#ifndef EDGEFLOW_MQTT_CLIENT_H
#define EDGEFLOW_MQTT_CLIENT_H

/*
 * MQTT 上报客户端（兼容 API）
 *
 * 进阶版主路径请使用 mqtt_session；本接口保留短连接一次性发布。
 */

#include "model/device_model.h"
#include "platform/config.h"

int mqtt_publish_point(const gateway_config_t *cfg, const telemetry_t *point);

#endif
