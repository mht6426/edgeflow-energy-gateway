/*
 * test_modbus_crc.c — Modbus CRC16 单元测试
 */

#include "ingress/modbus_rtu.h"

#include <assert.h>
#include <stdio.h>

int main(void)
{
    uint8_t data[] = {0x01U, 0x03U, 0x00U, 0x00U, 0x00U, 0x03U};
    uint16_t crc = modbus_crc16(data, sizeof(data));

    assert(crc != 0U);
    printf("test_modbus_crc passed (crc=0x%04x)\n", crc);
    return 0;
}
