# EdgeFlow Industrial Controller — CMake 构建
#
# 默认目标平台：Linux（x86_64 开发机 / aarch64 板端交叉编译）
# 依赖 POSIX API（pthread、socket 等将在后续学习步骤引入）
# 非 Linux 环境请使用 WSL、虚拟机或 Docker，见 README.md

cmake_minimum_required(VERSION 3.16)

project(edgeflow_energy_gateway C)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(WARNING
        "EdgeFlow 默认执行环境为 Linux；当前为 ${CMAKE_SYSTEM_NAME}。"
        "请在 WSL/Ubuntu、ARM 板端或 Linux 容器中构建与运行。")
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

# 学习阶段 M3：Model + Config + Logger + Adapter
set(EDGEFLOW_ADAPTER_SOURCES
    src/ingress/adapter.c
    src/ingress/stub_adapter.c
    src/common/time_util.c
)

add_executable(edgeflow
    src/main.c
    src/model/device_model.c
    src/platform/config.c
    src/platform/logger.c
    ${EDGEFLOW_ADAPTER_SOURCES}
)
edgeflow_target_common(edgeflow)

include(CTest)
if(BUILD_TESTING)
    add_executable(test_device_model
        tests/test_device_model.c
        src/model/device_model.c
    )
    edgeflow_target_common(test_device_model)
    add_test(NAME test_device_model COMMAND test_device_model)

    add_executable(test_config
        tests/test_config.c
        src/platform/config.c
    )
    edgeflow_target_common(test_config)
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
    add_test(NAME test_adapter COMMAND test_adapter WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif()
