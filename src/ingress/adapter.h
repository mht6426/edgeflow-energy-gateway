#ifndef EDGEFLOW_ADAPTER_H
#define EDGEFLOW_ADAPTER_H

/*
 * Device Adapter 插件接口（M3）
 *
 * 平台：Linux
 *
 * 设计目标：
 *   将 Modbus/串口/TCP 等协议细节封装在 Adapter 内部；
 *   上层（运行时、状态机）只通过统一 poll / write_command 与设备交互。
 *
 * 插件模型：
 *   每个 Adapter 实现 device_adapter_t 函数表；
 *   adapter_registry 负责注册、按配置初始化、聚合 poll 结果。
 *
 * 不负责：
 *   - 具体 Modbus 帧解析（M4 modbus_rtu.c）
 *   - 命令调度与重试（M6）
 *   - 线程与队列（M8）
 *
 * 数据流：
 *   Adapter.poll() → telemetry_t[] → （M8）SPSC → State Engine
 *   Command Scheduler → Adapter.write_command() → 协议写操作（M4/M6）
 */

#include "model/device_model.h"
#include "platform/config.h"

#include <stddef.h>
#include <stdint.h>

#define ADAPTER_REGISTRY_MAX 8U

typedef enum {
    ADAPTER_OK = 0,
    ADAPTER_ERR = -1,
    ADAPTER_ERR_FULL = -2,
    ADAPTER_ERR_NOT_FOUND = -3,
} adapter_status_t;

/*
 * 单个设备接入插件（类似 vtable + 实例上下文）。
 *
 * 生命周期：
 *   填充回调 → registry_register → init_all(cfg) → poll / write_command → shutdown_all
 */
typedef struct device_adapter {
    char name[EDGEFLOW_ID_MAX];       /* 插件名，如 "stub" / "modbus_rtu" */
    device_protocol_t protocol;       /* 协议类型，用于查找与日志 */
    uint16_t source_id;               /* 写入 telemetry.source_id，区分多 Adapter */
    void *ctx;                        /* 插件私有上下文，由具体 Adapter 管理 */

    /*
     * 初始化：打开串口、加载点表等。
     * @return 0 成功，<0 失败（调用方记录日志，M8 可标记设备离线）
     */
    int (*init)(struct device_adapter *self, const gateway_config_t *cfg);

    /* 释放 fd、关闭连接 */
    void (*shutdown)(struct device_adapter *self);

    /*
     * 采集：将协议数据转换为 telemetry_t，写入 out[0..max_out-1]。
     * @return 写入条数（>=0），<0 表示本轮采集失败
     */
    int (*poll)(struct device_adapter *self, telemetry_t *out, size_t max_out);

    /*
     * 下行：将 command_t 转为协议写操作。
     * M3 stub 可返回 -1；M4/M6 由 Modbus 等实现。
     * @return 0 已发送，<0 失败
     */
    int (*write_command)(struct device_adapter *self, const command_t *cmd);
} device_adapter_t;

/* 已注册 Adapter 列表（M3 单线程使用，M8 需考虑并发） */
typedef struct {
    device_adapter_t slots[ADAPTER_REGISTRY_MAX];
    size_t count;
} adapter_registry_t;

void adapter_registry_init(adapter_registry_t *reg);

/*
 * 注册插件（拷贝 device_adapter_t 到 registry）。
 * 同一 protocol 重复注册由调用方避免；当前不强制去重。
 */
adapter_status_t adapter_registry_register(adapter_registry_t *reg, const device_adapter_t *adapter);

/* 按协议查找已注册插件；未找到返回 NULL */
device_adapter_t *adapter_registry_find(adapter_registry_t *reg, device_protocol_t protocol);

/* 对所有已注册插件调用 init */
int adapter_registry_init_all(adapter_registry_t *reg, const gateway_config_t *cfg);

/*
 * 依次调用各插件 poll，将 telemetry 追加到 out，总数不超过 max_out。
 * @return 写入 out 的总条数
 */
int adapter_registry_poll_all(adapter_registry_t *reg, telemetry_t *out, size_t max_out);

/* 对所有插件调用 shutdown */
void adapter_registry_shutdown_all(adapter_registry_t *reg);

#endif
