/*
 * mqtt_client.c — 兼容短连接 API，内部委托 mqtt_session
 */

#include "egress/mqtt_client.h"

#include "egress/mqtt_session.h"

int mqtt_publish_point(const gateway_config_t *cfg, const telemetry_t *point)
{
    mqtt_session_t session;
    int rc;

    if (cfg == NULL || point == NULL) {
        return -1;
    }

    mqtt_session_init(&session, cfg);
    rc = mqtt_session_publish_telemetry(&session, point);
    mqtt_session_shutdown(&session);
    return rc;
}
