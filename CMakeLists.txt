cmake_minimum_required(VERSION 3.16)

project(edgeflow_energy_gateway C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(edgeflow
    src/main.c
    src/common/time_util.c
    src/core/ring.c
    src/model/device_model.c
    src/ingress/modbus_rtu.c
    src/egress/mqtt_client.c
    src/platform/config.c
    src/platform/logger.c
    src/platform/metrics.c
    src/platform/storage.c
    src/runtime/control_loop.c
    src/runtime/app.c
)

target_include_directories(edgeflow PRIVATE src)
target_compile_definitions(edgeflow PRIVATE _POSIX_C_SOURCE=200809L)
target_compile_options(edgeflow PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(edgeflow PRIVATE pthread)

include(CTest)
if(BUILD_TESTING)
    add_executable(test_ring tests/test_ring.c src/core/ring.c src/model/device_model.c)
    target_include_directories(test_ring PRIVATE src)
    target_compile_definitions(test_ring PRIVATE _POSIX_C_SOURCE=200809L)
    target_compile_options(test_ring PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME test_ring COMMAND test_ring)

    add_executable(test_device_model tests/test_device_model.c src/model/device_model.c)
    target_include_directories(test_device_model PRIVATE src)
    target_compile_definitions(test_device_model PRIVATE _POSIX_C_SOURCE=200809L)
    target_compile_options(test_device_model PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME test_device_model COMMAND test_device_model)

    add_executable(test_control_loop
        tests/test_control_loop.c
        src/runtime/control_loop.c
        src/model/device_model.c
        src/platform/config.c
        src/common/time_util.c
    )
    target_include_directories(test_control_loop PRIVATE src)
    target_compile_definitions(test_control_loop PRIVATE _POSIX_C_SOURCE=200809L)
    target_compile_options(test_control_loop PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME test_control_loop COMMAND test_control_loop)

    add_executable(test_modbus_crc
        tests/test_modbus_crc.c
        src/ingress/modbus_rtu.c
        src/platform/logger.c
        src/common/time_util.c
        src/model/device_model.c
    )
    target_include_directories(test_modbus_crc PRIVATE src)
    target_compile_definitions(test_modbus_crc PRIVATE _POSIX_C_SOURCE=200809L)
    target_compile_options(test_modbus_crc PRIVATE -Wall -Wextra -Wpedantic)
    target_link_libraries(test_modbus_crc PRIVATE pthread)
    add_test(NAME test_modbus_crc COMMAND test_modbus_crc)

    add_executable(test_config tests/test_config.c src/platform/config.c)
    target_include_directories(test_config PRIVATE src)
    target_compile_definitions(test_config PRIVATE _POSIX_C_SOURCE=200809L)
    target_compile_options(test_config PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME test_config COMMAND test_config WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif()
