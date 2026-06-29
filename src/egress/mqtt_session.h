#ifndef EDGEFLOW_MQTT_SESSION_H
#define EDGEFLOW_MQTT_SESSION_H

/*
 * MQTT 长连接会话（进阶版 M9）
 *
 * 保持 TCP 连接，支持实时 PUBLISH 与 SQLite 离线补传。
 */

#include "model/device_model.h"
#include "platform/config.h"
#include "platform/metrics.h"
#include "platform/storage.h"

#include <pthread.h>

typedef struct mqtt_session mqtt_session_t;

struct mqtt_session {
    gateway_config_t cfg;
    int fd;
    bool connected;
    /*
     * 保护 fd 与 connected：worker 线程（publish_telemetry）与 monitor 线程
     * （replay_pending / ping / publish_heartbeat）会并发访问同一 socket，
     * 必须串行化，避免 MQTT 报文交错以及一方关闭 fd 时另一方写入的 race。
     */
    pthread_mutex_t lock;
};

void mqtt_session_init(mqtt_session_t *session, const gateway_config_t *cfg);
void mqtt_session_shutdown(mqtt_session_t *session);

int mqtt_session_ensure_connected(mqtt_session_t *session);
int mqtt_session_publish_telemetry(mqtt_session_t *session, const telemetry_t *point);
int mqtt_session_publish_heartbeat(mqtt_session_t *session, const char *payload);
int mqtt_session_replay_pending(mqtt_session_t *session,
                                storage_ctx_t *storage,
                                gateway_metrics_t *metrics);
int mqtt_session_ping(mqtt_session_t *session);

#endif
