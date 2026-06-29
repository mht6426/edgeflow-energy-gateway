/*
 * modbus_serial.c — termios 串口读写（Linux）
 */

#include "ingress/modbus_serial.h"

#include "ingress/modbus_rtu.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

int modbus_serial_open(const char *device_path, int baud)
{
    int fd;
    struct termios tty;
    speed_t speed = B9600;

    if (device_path == NULL) {
        return -1;
    }

    switch (baud) {
    case 19200:
        speed = B19200;
        break;
    case 38400:
        speed = B38400;
        break;
    case 115200:
        speed = B115200;
        break;
    default:
        speed = B9600;
        break;
    }

    fd = open(device_path, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        return -1;
    }

    if (tcgetattr(fd, &tty) != 0) {
        close(fd);
        return -1;
    }

    cfmakeraw(&tty);
    cfsetispeed(&tty, speed);
    cfsetospeed(&tty, speed);
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 0;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        close(fd);
        return -1;
    }

    return fd;
}

void modbus_serial_close(int fd)
{
    if (fd >= 0) {
        close(fd);
    }
}

static int read_with_timeout(int fd, uint8_t *buf, size_t max, uint32_t timeout_ms)
{
    fd_set rfds;
    struct timeval tv;
    size_t total = 0U;

    while (total < max) {
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);
        tv.tv_sec = (time_t)(timeout_ms / 1000U);
        tv.tv_usec = (suseconds_t)((timeout_ms % 1000U) * 1000U);

        if (select(fd + 1, &rfds, NULL, NULL, &tv) <= 0) {
            break;
        }

        ssize_t n = read(fd, buf + total, max - total);
        if (n < 0) {
            if (errno == EAGAIN || errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (n == 0) {
            break;
        }
        total += (size_t)n;
        if (total >= 5U) {
            break;
        }
    }
    return (int)total;
}

int modbus_serial_exchange(int fd,
                           const uint8_t *request,
                           size_t request_len,
                           uint8_t *response,
                           size_t response_max,
                           uint32_t timeout_ms)
{
    uint16_t crc;
    uint16_t recv_crc;

    if (fd < 0 || request == NULL || request_len < 4U || response == NULL || response_max < 5U) {
        return -1;
    }

    if (write(fd, request, request_len) != (ssize_t)request_len) {
        return -1;
    }

    {
        int n = read_with_timeout(fd, response, response_max, timeout_ms);
        if (n < 5) {
            return -1;
        }

        crc = modbus_crc16(response, (size_t)(n - 2));
        recv_crc = (uint16_t)response[n - 2] | ((uint16_t)response[n - 1] << 8U);
        if (crc != recv_crc) {
            return -1;
        }
        return n;
    }
}
