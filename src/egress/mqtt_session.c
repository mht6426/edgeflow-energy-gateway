/*
 * mqtt_session.c — MQTT 3.1.1 长连接 + 批量补传
 */

#include "egress/mqtt_session.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

/*
 * 循环写直到全部写出，处理部分写（socket 可能一次写不完）与 EINTR。
 * 任一不可恢复错误返回 -1。
 */
static int mqtt_write_all(int fd, const uint8_t *buf, size_t len)
{
    size_t off = 0U;

    while (off < len) {
        ssize_t n = write(fd, buf + off, len - off);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        off += (size_t)n;
    }
    return 0;
}

/*
 * MQTT Remaining Length 变长字节编码（1–4 字节）。
 * 每字节低 7 位有效，最高位为续传标志；> 127 的长度必须多字节，
 * 否则真实 broker 会把单字节高位误判为续传而读取失败。
 * 返回写入 out 的字节数（1–4）。
 */
static size_t mqtt_encode_remaining_length(uint8_t *out, size_t remaining)
{
    size_t i = 0U;

    do {
        uint8_t byte = (uint8_t)(remaining % 128U);
        remaining /= 128U;
        if (remaining > 0U) {
            byte |= 0x80U;
        }
        out[i++] = byte;
    } while (remaining > 0U && i < 4U);

    return i;
}

static int mqtt_send_connect(int fd, const char *client_id, uint16_t keepalive_sec)
{
    uint8_t buf[160];
    uint8_t len_buf[4];
    size_t pos = 0U;
    size_t len_bytes;
    size_t id_len = strlen(client_id);
    size_t remaining = 10U + 2U + id_len; /* 可变报头 10 + 客户端 ID 长度域 2 + ID */

    len_bytes = mqtt_encode_remaining_length(len_buf, remaining);
    if (1U + len_bytes + remaining > sizeof(buf)) {
        return -1;
    }

    buf[pos++] = 0x10U;
    memcpy(&buf[pos], len_buf, len_bytes);
    pos += len_bytes;
    buf[pos++] = 0x00U;
    buf[pos++] = 0x04U;
    memcpy(&buf[pos], "MQTT", 4U);
    pos += 4U;
    buf[pos++] = 0x04U;
    buf[pos++] = 0x02U;
    buf[pos++] = (uint8_t)(keepalive_sec >> 8U);
    buf[pos++] = (uint8_t)(keepalive_sec & 0xFFU);
    buf[pos++] = (uint8_t)(id_len >> 8U);
    buf[pos++] = (uint8_t)(id_len & 0xFFU);
    memcpy(&buf[pos], client_id, id_len);
    pos += id_len;

    if (mqtt_write_all(fd, buf, pos) != 0) {
        return -1;
    }
    if (read(fd, buf, sizeof(buf)) < 4) {
        return -1;
    }
    return (buf[0] == 0x20U && buf[3] == 0x00U) ? 0 : -1;
}

static int mqtt_send_publish(int fd, const char *topic, const char *payload)
{
    uint8_t buf[1024];
    uint8_t len_buf[4];
    size_t pos = 0U;
    size_t len_bytes;
    size_t topic_len = strlen(topic);
    size_t payload_len = strlen(payload);
    size_t remaining = 2U + topic_len + payload_len; /* QoS0：主题长度域 2 + 主题 + 载荷 */

    len_bytes = mqtt_encode_remaining_length(len_buf, remaining);
    if (1U + len_bytes + remaining > sizeof(buf)) {
        return -1;
    }

    buf[pos++] = 0x30U;
    memcpy(&buf[pos], len_buf, len_bytes);
    pos += len_bytes;
    buf[pos++] = (uint8_t)(topic_len >> 8U);
    buf[pos++] = (uint8_t)(topic_len & 0xFFU);
    memcpy(&buf[pos], topic, topic_len);
    pos += topic_len;
    memcpy(&buf[pos], payload, payload_len);
    pos += payload_len;

    return mqtt_write_all(fd, buf, pos);
}

static int mqtt_send_ping(int fd)
{
    uint8_t ping[] = {0xC0U, 0x00U};
    return mqtt_write_all(fd, ping, sizeof(ping));
}

static int mqtt_open_tcp(const gateway_config_t *cfg)
{
    int fd;
    struct sockaddr_in addr;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(cfg->broker_port);
    if (inet_pton(AF_INET, cfg->broker_host, &addr.sin_addr) != 1) {
        close(fd);
        return -1;
    }
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        close(fd);
        return -1;
    }
    return fd;
}

static void telemetry_to_json(const telemetry_t *point, char *out, size_t out_len)
{
    snprintf(out,
             out_len,
             "{\"device_id\":\"%s\",\"point_id\":\"%s\",\"value\":%.4f,\"unit\":\"%s\","
             "\"ts_ms\":%llu,\"quality\":\"%s\"}",
             point->device_id,
             point->point_id,
             point->value,
             point->unit,
             (unsigned long long)point->ts_ms,
             telemetry_quality_to_string(point->quality));
}

void mqtt_session_init(mqtt_session_t *session, const gateway_config_t *cfg)
{
    if (session == NULL) {
        return;
    }
    memset(session, 0, sizeof(*session));
    if (cfg != NULL) {
        session->cfg = *cfg;
    }
    session->fd = -1;
    session->connected = false;
    pthread_mutex_init(&session->lock, NULL);
}

/* 关闭连接（调用方须已持有 session->lock）。 */
static void mqtt_shutdown_locked(mqtt_session_t *session)
{
    if (session->fd >= 0) {
        close(session->fd);
        session->fd = -1;
    }
    session->connected = false;
}

/* 建立/复用连接（调用方须已持有 session->lock）。 */
static int mqtt_ensure_connected_locked(mqtt_session_t *session)
{
    char client_id[64];

    if (session->connected && session->fd >= 0) {
        return 0;
    }

    mqtt_shutdown_locked(session);

    session->fd = mqtt_open_tcp(&session->cfg);
    if (session->fd < 0) {
        session->connected = false;
        return -1;
    }

    snprintf(client_id, sizeof(client_id), "edgeflow-%s", session->cfg.device_id);
    if (mqtt_send_connect(session->fd, client_id, (uint16_t)session->cfg.mqtt_keepalive_sec) != 0) {
        mqtt_shutdown_locked(session);
        return -1;
    }

    session->connected = true;
    return 0;
}

void mqtt_session_shutdown(mqtt_session_t *session)
{
    if (session == NULL) {
        return;
    }
    pthread_mutex_lock(&session->lock);
    mqtt_shutdown_locked(session);
    pthread_mutex_unlock(&session->lock);
}

int mqtt_session_ensure_connected(mqtt_session_t *session)
{
    int rc;

    if (session == NULL) {
        return -1;
    }
    pthread_mutex_lock(&session->lock);
    rc = mqtt_ensure_connected_locked(session);
    pthread_mutex_unlock(&session->lock);
    return rc;
}

int mqtt_session_publish_telemetry(mqtt_session_t *session, const telemetry_t *point)
{
    char payload[384];
    int rc = -1;

    if (session == NULL || point == NULL) {
        return -1;
    }

    telemetry_to_json(point, payload, sizeof(payload));

    pthread_mutex_lock(&session->lock);
    if (mqtt_ensure_connected_locked(session) == 0) {
        if (mqtt_send_publish(session->fd, session->cfg.mqtt_topic, payload) == 0) {
            rc = 0;
        } else {
            mqtt_shutdown_locked(session);
        }
    }
    pthread_mutex_unlock(&session->lock);
    return rc;
}

int mqtt_session_publish_heartbeat(mqtt_session_t *session, const char *payload)
{
    int rc = -1;

    if (session == NULL || payload == NULL) {
        return -1;
    }

    pthread_mutex_lock(&session->lock);
    if (mqtt_ensure_connected_locked(session) == 0) {
        if (mqtt_send_publish(session->fd, session->cfg.mqtt_heartbeat_topic, payload) == 0) {
            rc = 0;
        } else {
            mqtt_shutdown_locked(session);
        }
    }
    pthread_mutex_unlock(&session->lock);
    return rc;
}

int mqtt_session_replay_pending(mqtt_session_t *session,
                                storage_ctx_t *storage,
                                gateway_metrics_t *metrics)
{
    storage_telemetry_row_t rows[64];
    size_t count = 0U;
    size_t batch;
    int replayed = 0;

    if (session == NULL || storage == NULL) {
        return 0;
    }

    batch = session->cfg.mqtt_replay_batch;
    if (batch == 0U) {
        batch = 50U;
    }
    if (batch > 64U) {
        batch = 64U;
    }

    /* 先取 SQLite 待补传数据，再进入临界区，缩短持有 MQTT 锁的时间。 */
    if (storage_fetch_pending_telemetry(storage, rows, batch, &count) < 0) {
        return 0;
    }

    pthread_mutex_lock(&session->lock);
    if (mqtt_ensure_connected_locked(session) == 0) {
        for (size_t i = 0U; i < count; i++) {
            char payload[384];
            telemetry_to_json(&rows[i].point, payload, sizeof(payload));
            if (mqtt_send_publish(session->fd, session->cfg.mqtt_topic, payload) != 0) {
                mqtt_shutdown_locked(session);
                break;
            }
            if (storage_mark_telemetry_uploaded(storage, rows[i].row_id) == 0) {
                replayed++;
                if (metrics != NULL) {
                    atomic_fetch_add(&metrics->mqtt_replay_total, 1ULL);
                }
            }
        }
    }
    pthread_mutex_unlock(&session->lock);

    if (metrics != NULL) {
        atomic_store(&metrics->storage_pending_count,
                     (unsigned int)storage_pending_telemetry_count(storage));
    }
    return replayed;
}

int mqtt_session_ping(mqtt_session_t *session)
{
    int rc = -1;

    if (session == NULL) {
        return -1;
    }

    pthread_mutex_lock(&session->lock);
    if (session->connected && session->fd >= 0) {
        if (mqtt_send_ping(session->fd) == 0) {
            rc = 0;
        } else {
            mqtt_shutdown_locked(session);
        }
    }
    pthread_mutex_unlock(&session->lock);
    return rc;
}
