#include "ingress/modbus_rtu.h"

#include <assert.h>
#include <stdio.h>

int main(void)
{
    const uint8_t frame[] = {0x01U, 0x03U, 0x00U, 0x00U, 0x00U, 0x03U};
    uint16_t crc;

    crc = modbus_crc16(frame, sizeof(frame));
    assert(crc == 0x45CBU);

    printf("test_modbus_crc passed\n");
    return 0;
}
