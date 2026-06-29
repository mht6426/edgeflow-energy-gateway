#ifndef EDGEFLOW_MODBUS_SERIAL_H
#define EDGEFLOW_MODBUS_SERIAL_H

/*
 * Linux 串口 termios 封装（进阶版 M4）
 */

#include <stddef.h>
#include <stdint.h>

int modbus_serial_open(const char *device_path, int baud);
void modbus_serial_close(int fd);

/*
 * 发送 RTU 帧并读取响应（含 CRC 校验）。
 * @return 响应有效字节数，<0 失败
 */
int modbus_serial_exchange(int fd,
                           const uint8_t *request,
                           size_t request_len,
                           uint8_t *response,
                           size_t response_max,
                           uint32_t timeout_ms);

#endif
