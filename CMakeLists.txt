# EdgeFlow Industrial Controller — CMake 构建
#
# 目标平台：Linux（x86_64 开发机 / aarch64 板端交叉编译）
# 本工程仅支持 Linux，使用 POSIX API，不提供其他平台兼容分支。

cmake_minimum_required(VERSION 3.16)

project(edgeflow_energy_gateway C)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "EdgeFlow 仅支持 Linux；当前 CMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}。")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(EDGEFLOW_POSIX_DEFS _POSIX_C_SOURCE=200809L)

function(edgeflow_target_common name)
    target_include_directories(${name} PRIVATE src)
    target_compile_definitions(${name} PRIVATE ${EDGEFLOW_POSIX_DEFS})
    target_compile_options(${name} PRIVATE -Wall -Wextra -Wpedantic)
endfunction()

find_package(SQLite3 REQUIRED)

add_subdirectory(third_party)

set(EDGEFLOW_CORE_SOURCES
    src/model/device_model.c
    src/platform/config.c
    src/platform/logger.c
    src/platform/storage.c
    src/platform/metrics.c
    src/platform/watchdog.c
    src/platform/heartbeat.c
    src/common/time_util.c
    src/core/ring.c
    src/core/reactor.c
    src/ingress/adapter.c
    src/ingress/stub_adapter.c
    src/ingress/modbus_rtu.c
    src/ingress/modbus_adapter.c
    src/ingress/modbus_libmodbus.c
    src/runtime/state_engine.c
    src/runtime/command_scheduler.c
    src/runtime/app.c
    src/egress/mqtt_client.c
    src/egress/mqtt_session.c
)

add_executable(edgeflow
    src/main.c
    ${EDGEFLOW_CORE_SOURCES}
)
edgeflow_target_common(edgeflow)
target_link_libraries(edgeflow PRIVATE pthread SQLite::SQLite3 cjson modbus)

add_executable(edgeflow-cli
    src/cli/main.c
    src/platform/config.c
)
edgeflow_target_common(edgeflow-cli)
target_link_libraries(edgeflow-cli PRIVATE SQLite::SQLite3 cjson)

include(CTest)
if(BUILD_TESTING)
    add_executable(test_device_model tests/test_device_model.c src/model/device_model.c)
    edgeflow_target_common(test_device_model)
    add_test(NAME test_device_model COMMAND test_device_model)

    add_executable(test_config tests/test_config.c src/platform/config.c)
    edgeflow_target_common(test_config)
    target_link_libraries(test_config PRIVATE cjson)
    add_test(NAME test_config COMMAND test_config WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

    add_executable(test_adapter
        tests/test_adapter.c
        src/ingress/adapter.c
        src/ingress/stub_adapter.c
        src/model/device_model.c
        src/platform/config.c
        src/platform/logger.c
        src/common/time_util.c
    )
    edgeflow_target_common(test_adapter)
    target_link_libraries(test_adapter PRIVATE cjson)
    add_test(NAME test_adapter COMMAND test_adapter WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

    add_executable(test_ring tests/test_ring.c src/core/ring.c src/model/device_model.c)
    edgeflow_target_common(test_ring)
    add_test(NAME test_ring COMMAND test_ring)

    add_executable(test_modbus_crc
        tests/test_modbus_crc.c
        src/ingress/modbus_rtu.c
        src/common/time_util.c
        src/model/device_model.c
        src/platform/logger.c
    )
    edgeflow_target_common(test_modbus_crc)
    add_test(NAME test_modbus_crc COMMAND test_modbus_crc)

    add_executable(test_state_engine
        tests/test_state_engine.c
        src/runtime/state_engine.c
        src/model/device_model.c
        src/platform/config.c
    )
    edgeflow_target_common(test_state_engine)
    target_link_libraries(test_state_engine PRIVATE cjson)
    add_test(NAME test_state_engine COMMAND test_state_engine)

    add_executable(test_command_scheduler
        tests/test_command_scheduler.c
        src/runtime/command_scheduler.c
        src/ingress/modbus_adapter.c
        src/ingress/modbus_rtu.c
        src/ingress/modbus_libmodbus.c
        src/ingress/adapter.c
        src/model/device_model.c
        src/platform/config.c
        src/platform/logger.c
        src/platform/storage.c
        src/common/time_util.c
    )
    edgeflow_target_common(test_command_scheduler)
    target_link_libraries(test_command_scheduler PRIVATE SQLite::SQLite3 cjson modbus)
    add_test(NAME test_command_scheduler COMMAND test_command_scheduler)

    add_executable(test_storage
        tests/test_storage.c
        src/platform/storage.c
        src/model/device_model.c
        src/platform/config.c
    )
    edgeflow_target_common(test_storage)
    target_link_libraries(test_storage PRIVATE SQLite::SQLite3 cjson)
    add_test(NAME test_storage COMMAND test_storage)
endif()
