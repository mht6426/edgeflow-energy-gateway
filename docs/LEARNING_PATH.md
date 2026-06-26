# EdgeFlow 边学边做路径

> **用法**：按步骤推进，每步完成「设计 → 代码 → 单元测试 → 集成测试」后再进入下一步。  
> 与 [MODULE_DESIGN.md](MODULE_DESIGN.md) 模块顺序一致，与 [ROADMAP.md](ROADMAP.md) 三个月目标对齐。

**当前步骤**：**M3 — Device Adapter 插件接口**（进行中）

---

## 步骤总览

| 步骤 | 模块 | 关键产出 | 状态 |
| --- | --- | --- | --- |
| M1 | 统一设备模型 | `src/model/device_model.*` | ✅ |
| M2 | Config Manager + Logger | `platform/config.*`、`platform/logger.*` | ✅ |
| **M3** | Device Adapter 接口 | `ingress/adapter.*`、`stub_adapter.*` | 🔄 进行中 |
| M4 | Modbus 模拟设备接入 | `ingress/modbus_*` | ⏳ |
| M5 | State And Alarm Engine | `runtime/state_engine.*`、告警联锁 | ⏳ |
| M6 | Command Scheduler | `runtime/command_scheduler.*` | ⏳ |
| M7 | SQLite WAL Storage | `platform/storage.*` | ⏳ |
| M8 | Reactor + Thread Pool | `core/ring.*`、`runtime/app.*` | ⏳ |
| M9 | MQTT Uploader | `egress/mqtt_client.*` | ⏳ |
| M10 | CLI、Metrics、Watchdog | `platform/metrics.*`、CLI、systemd 喂狗 | ⏳ |

---

## M3 — Device Adapter 插件接口（当前）

### 你要理解什么

Adapter 是**协议层与统一模型之间的翻译器**。上层只调用：

- `poll()` → 产出 `telemetry_t[]`
- `write_command()` → 消费 `command_t`（M4/M6 实现）

`adapter_registry` 管理多个插件（Modbus、模拟器等），`poll_all` 聚合结果。

### 本步已实现

- `ingress/adapter.h/c` — `device_adapter_t` 函数表、注册表、`poll_all`
- `ingress/stub_adapter.h/c` — `simulate:true` 时 3 条 EMS 演示点位
- `common/time_util.*` — `edgeflow_now_ms()` 打时间戳
- `tests/test_adapter.c`
- `main.c` — 注册 stub → 单轮 poll → 写日志

### M3 验收清单

- [ ] 能画出 `registry → init → poll → telemetry_t` 数据流
- [ ] `ctest` 含 `test_adapter` 通过
- [ ] `./build/edgeflow` 日志有 3 条 telemetry
- [ ] 能说明 `write_command` 为何 M3 返回 -1

### M3 完成后 → M4 Modbus RTU

---

## M2 — Config Manager + Logger（已完成）

### 本步已实现

- `gateway_config_t` + `gateway_config_load()`：加载 `configs/gateway.json`
- `logger_init()` / `log_info|warn|error`：写 `<log_dir>/edgeflow.log`
- `main.c`：`-c` / `--config`，默认 `configs/gateway.json`
- `tests/test_config.c`

### M2 验收清单

- [ ] `ctest` 中 `test_config` 通过
- [ ] `./build/edgeflow -c configs/gateway.json` 成功
- [ ] `/tmp/edgeflow/edgeflow.log` 有 startup 日志（Linux）
- [ ] 能说出 `gateway_config_t` 各字段在后续哪一步使用

### M2 完成后 → M3 Device Adapter 接口

---

## M1 — 统一设备模型（已完成）

### 你要理解什么

工业网关里所有协议（Modbus、MQTT、TCP）最终都要变成**同一套数据结构**，上层状态机、策略、存储只认这些类型，不认寄存器地址。

五类核心对象：

| 类型 | 作用 |
| --- | --- |
| `device_t` | 设备身份、协议、在线状态 |
| `point_t` | 测点定义（单位、scale、限值） |
| `telemetry_t` | 一次采集结果（值 + 时间戳 + 质量位） |
| `alarm_event_t` | 告警事件 |
| `command_t` | 控制指令及状态机 |

### 本步已实现

- 类型定义与枚举：`src/model/device_model.h`
- 辅助函数：`device_init`、`point_init`、`telemetry_init`、`alarm_make`、`command_*`
- 工程值换算：`point_raw_to_engineering`
- 设备在线状态：`device_mark_online` / `device_mark_failure`
- 单测：`tests/test_device_model.c`
- 演示入口：`src/main.c`（打印模型示例，不启动运行时）

### M1 验收清单

- [ ] 阅读 `docs/DEVICE_MODEL.md` 与 `device_model.h`，能说出五类对象各自字段含义
- [ ] `cmake --build build && ctest --test-dir build` 通过
- [ ] `./build/edgeflow` 能打印 BMS 测点与告警/命令示例
- [ ] 能独立构造一条 `telemetry_t` 并解释 `quality` 字段用途
- [ ] 完成 M1 后更新 `docs/PROGRESS.md` 将 M1 标为 ✅

### M1 完成后 → 进入 M2

1. 在 `src/platform/` 新建 `config.c/h`：加载 `configs/gateway.json`
2. 新建 `logger.c/h`：写文件或 stderr
3. `main.c` 改为：解析 `-c` 路径 → 加载配置 → 初始化日志
4. 新建 `tests/test_config.c`

参考设计约束：[MODULE_DESIGN.md](MODULE_DESIGN.md) 中 Config Manager 模板。

---

## 推荐阅读顺序（仅 M1）

1. [LEARNING_PATH.md](LEARNING_PATH.md)（本文）
2. [DEVICE_MODEL.md](DEVICE_MODEL.md)
3. `src/model/device_model.h`
4. `src/model/device_model.c`
5. `tests/test_device_model.c`
6. `src/main.c`

架构全景可在 M5 之前浏览 [ARCHITECTURE.md](ARCHITECTURE.md)，**不必一次读完**。

---

## 代码注释约定（当前及后续模块均遵循）

> 完整规范见 [CODE_STYLE.md](CODE_STYLE.md)。**每个新建或修改的 `.c` / `.h` / 测试文件都必须带详细中文注释。**

### 默认执行环境：Linux

- 开发与运行以 **Linux** 为准；Windows 使用 WSL，不为 Windows 写兼容代码
- 路径、串口、日志目录使用 Linux 惯例（`/tmp`、`/dev/ttyUSB0`）
- CMake 启用 `_POSIX_C_SOURCE=200809L`

### 注释层次

新增或修改源码时，按以下层次写**简体中文**注释（保留 Mutex、RAII 等英文术语）：

| 位置 | 写什么 |
| --- | --- |
| `.c` 文件头 | 模块职责、不负责什么、所属学习阶段（如 M2） |
| `.h` 文件头 | 设计目标、五类对象/核心概念、与上下游模块边界 |
| `struct` / 枚举 | 用途、典型流转、由哪一层维护 |
| 字段 | 含义、单位、示例值（非显而易见时） |
| 函数 | 参数语义、返回值、调用时机（如「Adapter 采集后调用」） |
| 测试用例 | 文件头写覆盖范围；每个 `assert` 块前用分段注释 |

**原则：**

- 解释「为什么这样设计」，少复述代码字面意思
- 标出后续步骤才实现的能力（如 `/* M6 Scheduler 推进 */`）
- 不在注释里写与实现矛盾的承诺

**M1–M2 已注释文件（后续模块请对齐此标准）：**

- `src/model/device_model.h` / `.c`
- `src/platform/config.h` / `.c`
- `src/platform/logger.h` / `.c`
- `src/main.c`
- `tests/test_device_model.c` / `test_config.c`
- `CMakeLists.txt`

---

## 构建与验证

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/edgeflow
```

---

## 变更记录

| 日期 | 说明 |
| --- | --- |
| 2026-06-25 | 全量重构：移除未按学习路径实现的半成品模块，从 M1 重新开始 |
