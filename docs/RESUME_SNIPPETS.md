# 简历材料

## 通用版

项目名称：EdgeFlow Industrial Controller | ARM Linux 工业边缘控制平台

项目描述：

基于 RK3568/RK3588 ARM Linux 设计并实现工业边缘控制平台，支持插件化工业设备接入、统一设备模型、状态机告警联锁、策略引擎、可靠指令调度、SQLite WAL 断网缓存、MQTT 上报、epoll/Reactor + Thread Pool、Prometheus metrics、CLI、systemd watchdog 和板端长期运行验证。以内置 BMS/PCS/Meter 模拟器验证分时电价、削峰填谷、最大需量限制和恒功率控制闭环。

技术栈：

C11、C++17、Linux、CMake、ARM64、epoll、Reactor、Thread Pool、Modbus RTU/TCP、TCP、UDP、MQTT、SQLite WAL、systemd、Prometheus metrics。

## 项目职责

- 设计 Device Adapter、Unified Device Model、State And Alarm Engine、Strategy Engine、Command Scheduler、Storage、Platform 七层架构，将协议通信、设备模型、状态机、策略决策和控制指令解耦。
- 设计 `Device`、`Point`、`Telemetry`、`Alarm`、`Command` 统一模型，避免业务层直接依赖 Modbus 寄存器或 TCP frame。
- 基于状态机实现设备在线/离线、通信故障、温度告警、急停联锁和告警恢复，避免复杂 if/else 导致状态不可控。
- 实现示例 Strategy Engine，支持分时电价、削峰填谷、最大需量限制和恒功率充放电，输出可解释的控制命令。
- 设计 Command Scheduler，支持 command_id、超时、重试、回读校验和控制动作审计，保证命令状态可追踪。
- 设计 SQLite WAL 本地缓存、upload cursor 和 MQTT 批量上报，支持断网本地自治和网络恢复补传。
- 构建 epoll/Reactor + Thread Pool 并发模型，区分 socket 事件、阻塞式设备轮询、SQLite 写入和策略计算。
- 构建 metrics、结构化日志、thread heartbeat、CLI、systemd watchdog 和故障排查文档，完成 ARM Linux 板端验证。

## 储能 EMS 岗强化版

突出描述：

以内置 BMS/PCS/Meter 模拟器作为示例 profile，验证工业边缘控制平台的状态机、策略引擎和命令调度能力，覆盖分时电价、削峰填谷、最大需量限制、故障联锁和断网自治等链路。

## 充电桩 / 光储充岗强化版

突出描述：

项目可扩展为光储充站端边缘控制器，通过 Modbus/MQTT 对接电表和 PCS，支持站端本地策略、充放电控制审计、告警上报和断网缓存补传。

## 面试 60 秒

我做的是一个 ARM Linux 工业边缘控制平台，运行在 RK3568 工控板上。它不是云平台，也不是单一 EMS，而是一个控制器运行时：底层通过插件化 Adapter 接入 Modbus RTU/TCP 和模拟设备，中间统一成 Device/Point/Telemetry/Alarm/Command 模型，上层用状态机处理离线、告警和联锁，用策略引擎生成控制命令，再通过 Command Scheduler 做超时、重试和回读校验。平台侧我设计了 SQLite WAL 断网缓存、MQTT 上报、epoll/Reactor + Thread Pool、metrics、CLI、thread heartbeat 和 systemd watchdog。

## 不要写

- 不写“精通储能调度算法”。
- 不写“完整 BMS/PCS 协议栈”。
- 不写“IEC61850 全量实现”。
- 不写“生产级并网控制”。
- 不写“云平台”或“微服务架构”。
- 没有真实接入 SQLite 前，不写“已实现 SQLite WAL 断网缓存”。
- 没有上板和实测数据前，不写“通过 24h 稳定性测试”。
