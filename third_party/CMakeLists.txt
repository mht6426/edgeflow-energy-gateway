# third_party —  vendored cJSON 与 libmodbus（静态链接，便于交叉编译）

# --- cJSON ---
add_library(cjson STATIC
    cJSON/cJSON.c
)
target_include_directories(cjson PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/cJSON")

# --- libmodbus（Linux 嵌入式网关常用协议库）---
set(LIBMODBUS_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libmodbus/src")
set(LIBMODBUS_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/libmodbus-gen")

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/libmodbus/cmake/linux/modbus-version.h"
    "${LIBMODBUS_GEN_DIR}/modbus-version.h"
    COPYONLY
)
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/libmodbus/cmake/linux/config.h"
    "${LIBMODBUS_GEN_DIR}/config.h"
    COPYONLY
)

add_library(modbus STATIC
    ${LIBMODBUS_SRC_DIR}/modbus.c
    ${LIBMODBUS_SRC_DIR}/modbus-data.c
    ${LIBMODBUS_SRC_DIR}/modbus-rtu.c
    ${LIBMODBUS_SRC_DIR}/modbus-tcp.c
)
target_include_directories(modbus PUBLIC
    ${LIBMODBUS_SRC_DIR}
    ${LIBMODBUS_GEN_DIR}
)
