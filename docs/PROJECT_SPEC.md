# Project Spec

## 项目名称

EdgeFlow Industrial Controller

## 项目本质

EdgeFlow Industrial Controller 是一个运行在 ARM Linux（RK3568 / RK3588）上的工业边缘控制平台。

该项目聚焦的工程能力：

- Linux 系统软件。
- ARM 嵌入式 Linux 开发。
- 工业设备接入与协议适配。
- 多线程并发设计。
- 控制器运行时与边缘本地自治。

目标不是实现真实商业 EMS。EMS/储能仅作为内置示例场景，用于验证平台的设备接入、状态机、策略、指令调度和断网自治能力。

## 设计原则

必须优先考虑：

1. 工程真实性（文档不超前于代码实现）。
2. Linux 系统能力。
3. ARM 部署能力。
4. 代码质量与线程/资源安全。
5. 长期可维护性。

不允许为了炫技而引入复杂架构。

## 技术栈约束

语言：

- C11。
- C++17。

构建系统：

- CMake。

数据库：

- SQLite WAL。

操作系统：

- Linux。

部署平台：

- RK3568。
- RK3588。

并发模型：

- epoll。
- Reactor。
- Thread Pool。

日志：

- 自研轻量日志。
- 后续可选 spdlog。

配置：

- JSON。

通信协议：

- TCP。
- UDP。
- Modbus RTU。
- Modbus TCP。
- MQTT。

## 禁止引入

不要引入：

- Kubernetes。
- Docker。
- 微服务。
- Java。
- Go。
- Python 服务端。
- Redis。
- Kafka。
- Qt GUI。
- Web 前端框架。

项目定位是 ARM Linux 边缘控制器，不是云平台。

## 核心架构

系统必须保持以下结构：

```text
Device Adapter Layer
    -> Unified Device Model
    -> State And Alarm Engine
    -> Strategy Engine
    -> Command Scheduler
    -> Local Storage
    -> MQTT / HTTP Uploader
```

Platform Layer 为横向基础设施，向各层提供配置、日志、metrics、watchdog、CLI、thread heartbeat 和 systemd 支持。

## 模块边界

### Device Adapter Layer

负责：

- Modbus RTU 设备。
- Modbus TCP 设备。
- 模拟 BMS。
- 模拟 PCS。
- 模拟 Meter。

必须采用插件化设计。禁止业务逻辑进入通信层。

### Unified Device Model

统一抽象：

- `Device`。
- `Point`。
- `Alarm`。
- `Command`。
- `Telemetry`。

所有业务层禁止直接访问协议层数据。

### State And Alarm Engine

负责：

- 设备在线离线。
- SOC 越界。
- 温度告警。
- 通信故障。
- 急停联锁。

必须采用状态机设计。

### Strategy Engine

仅实现：

- 分时电价。
- 削峰填谷。
- 最大需量限制。
- 恒功率充放电。

不实现复杂电力算法，不实现潮流计算，不实现并网控制。

### Command Scheduler

负责：

- 指令下发。
- 超时。
- 重试。
- 回读校验。
- 审计日志。

必须保证命令状态可追踪。

### Storage Layer

采用 SQLite WAL。

支持：

- 断网缓存。
- 补传。
- 数据清理。

禁止引入其他数据库。

### Platform Layer

必须包含：

- Config Manager。
- Metrics。
- Watchdog。
- CLI。
- Thread Heartbeat。
- systemd 支持。

## 开发流程

每次输出代码前，必须先输出：

1. 模块职责。
2. 输入输出。
3. 数据流。
4. 线程模型。
5. 异常处理。

然后再输出代码。

不要一次生成整个项目。必须按照模块逐步开发。

每个模块按以下流程推进：

```text
设计 -> 代码 -> 单元测试 -> 集成测试
```
