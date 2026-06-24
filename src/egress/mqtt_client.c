#include "egress/mqtt_client.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static int send_all(int fd, const unsigned char *buf, size_t len)
{
    size_t sent = 0U;
    while (sent < len) {
        ssize_t n = send(fd, buf + sent, len - sent, 0);
        if (n <= 0) {
            return -1;
        }
        sent += (size_t)n;
    }
    return 0;
}

static int put_string(unsigned char *buf, size_t cap, const char *s)
{
    const size_t len = strlen(s);
    if (len > 65535U || cap < len + 2U) {
        return -1;
    }
    buf[0] = (unsigned char)(len >> 8U);
    buf[1] = (unsigned char)(len & 0xFFU);
    memcpy(buf + 2U, s, len);
    return (int)(len + 2U);
}

static int tcp_connect(const char *host, uint16_t port)
{
    struct addrinfo hints;
    struct addrinfo *res = NULL;
    struct addrinfo *it;
    char port_str[16];
    int fd = -1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    snprintf(port_str, sizeof(port_str), "%u", port);

    if (getaddrinfo(host, port_str, &hints, &res) != 0) {
        return -1;
    }
    for (it = res; it != NULL; it = it->ai_next) {
        fd = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
        if (fd < 0) {
            continue;
        }
        if (connect(fd, it->ai_addr, it->ai_addrlen) == 0) {
            break;
        }
        close(fd);
        fd = -1;
    }
    freeaddrinfo(res);
    return fd;
}

static int mqtt_connect_packet(unsigned char *buf, size_t cap, const char *client_id)
{
    unsigned char payload[128];
    int id_len = put_string(payload, sizeof(payload), client_id);
    size_t pos = 0U;
    size_t remain;

    if (id_len < 0 || cap < 32U) {
        return -1;
    }
    buf[pos++] = 0x10U;
    remain = 10U + (size_t)id_len;
    if (remain > 127U) {
        return -1;
    }
    buf[pos++] = (unsigned char)remain;
    buf[pos++] = 0x00U;
    buf[pos++] = 0x04U;
    memcpy(buf + pos, "MQTT", 4U);
    pos += 4U;
    buf[pos++] = 0x04U;
    buf[pos++] = 0x02U;
    buf[pos++] = 0x00U;
    buf[pos++] = 0x3CU;
    memcpy(buf + pos, payload, (size_t)id_len);
    pos += (size_t)id_len;
    return (int)pos;
}

static int mqtt_publish_packet(unsigned char *buf, size_t cap, const char *topic, const char *payload)
{
    unsigned char topic_buf[160];
    int topic_len = put_string(topic_buf, sizeof(topic_buf), topic);
    size_t payload_len = strlen(payload);
    size_t remain;
    size_t pos = 0U;

    if (topic_len < 0) {
        return -1;
    }
    remain = (size_t)topic_len + payload_len;
    if (remain > 127U || cap < remain + 2U) {
        return -1;
    }
    buf[pos++] = 0x30U;
    buf[pos++] = (unsigned char)remain;
    memcpy(buf + pos, topic_buf, (size_t)topic_len);
    pos += (size_t)topic_len;
    memcpy(buf + pos, payload, payload_len);
    pos += payload_len;
    return (int)pos;
}

static int recv_connack(int fd)
{
    unsigned char buf[4];

    if (recv(fd, buf, sizeof(buf), 0) != (ssize_t)sizeof(buf)) {
        return -1;
    }
    /* CONNACK: 0x20 0x02 0x00 return_code */
    if (buf[0] != 0x20U || buf[1] != 0x02U || buf[3] != 0x00U) {
        return -1;
    }
    return 0;
}

int mqtt_publish_point(const gateway_config_t *cfg, const data_point_t *point)
{
    int fd;
    unsigned char packet[512];
    char payload[512];
    int len;

    fd = tcp_connect(cfg->broker_host, cfg->broker_port);
    if (fd < 0) {
        return -1;
    }

    len = mqtt_connect_packet(packet, sizeof(packet), cfg->device_id);
    if (len < 0 || send_all(fd, packet, (size_t)len) != 0) {
        close(fd);
        return -1;
    }
    if (recv_connack(fd) != 0) {
        close(fd);
        return -1;
    }

    len = snprintf(payload, sizeof(payload),
                   "{\"device_id\":\"%s\",\"ts\":%llu,\"telemetry\":{\"device_id\":\"%s\",\"point_id\":\"%s\",\"value\":%.3f,\"unit\":\"%s\",\"quality\":\"%s\",\"src\":%u}}",
                   cfg->device_id,
                   (unsigned long long)point->ts_ms,
                   point->device_id,
                   point->point_id,
                   point->value,
                   point->unit,
                   telemetry_quality_to_string(point->quality),
                   point->source_id);
    if (len < 0 || (size_t)len >= sizeof(payload)) {
        close(fd);
        return -1;
    }
    len = mqtt_publish_packet(packet, sizeof(packet), cfg->mqtt_topic, payload);
    if (len < 0 || send_all(fd, packet, (size_t)len) != 0) {
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}
