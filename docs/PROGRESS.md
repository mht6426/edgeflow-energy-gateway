# EdgeFlow 开发进度总览

> **用途**：换设备 `git clone` 后，先看 [LEARNING_PATH.md](LEARNING_PATH.md)，再对照本文档。

**最后更新**：2026-06-25  
**当前步骤**：**M3** — Device Adapter 插件接口（进行中）  
**下一任务**：完成 M3 验收 → **M4** Modbus 模拟设备接入

---

## 一、30 秒速览

| 项目 | 内容 |
| --- | --- |
| 产品名 | EdgeFlow Industrial Controller |
| 学习方式 | 按 [LEARNING_PATH.md](LEARNING_PATH.md) 10 步推进 |
| 当前代码量 | M1–M3：模型 + 配置日志 + Adapter 注册与 stub 采集 |
| 架构文档 | [ARCHITECTURE.md](ARCHITECTURE.md) 为**目标架构** |

### 学习路径进度

```text
M1  统一设备模型           [████████████████████] 100%  ✅
M2  Config + Logger        [████████████████████] 100%  ✅
M3  Adapter 接口            [██████████████░░░░░░]  75%  ← 当前
M4  Modbus 模拟接入         [░░░░░░░░░░░░░░░░░░░░]   0%
M5  状态机与告警            [░░░░░░░░░░░░░░░░░░░░]   0%
M6  Command Scheduler       [░░░░░░░░░░░░░░░░░░░░]   0%
M7  SQLite WAL              [░░░░░░░░░░░░░░░░░░░░]   0%
M8  Reactor + 运行时        [░░░░░░░░░░░░░░░░░░░░]   0%
M9  MQTT Uploader           [░░░░░░░░░░░░░░░░░░░░]   0%
M10 CLI / Metrics / Watchdog [░░░░░░░░░░░░░░░░░░░░]   0%
```

---

## 二、当前代码地图

```text
edgeflow-energy-gateway/
├── src/
│   ├── main.c                 # M3：配置 + 日志 + Adapter poll
│   ├── model/device_model.*   # M1 ✅
│   ├── platform/config.*      # M2 ✅
│   ├── platform/logger.*      # M2 ✅
│   ├── common/time_util.*     # M3 单调时钟
│   ├── ingress/adapter.*      # M3 ✅ 插件注册表
│   ├── ingress/stub_adapter.* # M3 ✅ 演示采集
│   ├── egress/README.md       # M9 占位
│   ├── platform/README.md     # M2/M7/M10 占位
│   └── runtime/README.md      # M5-M6/M8 占位
├── tests/test_device_model.c  # M1 ✅
├── tests/test_config.c        # M2 ✅
├── tests/test_adapter.c       # M3 ✅
├── configs/                   # M2 起使用（当前保留示例 JSON）
└── docs/LEARNING_PATH.md      # ★ 边学边做主文档
```

---

## 三、验证当前能力（M3）

```bash
cmake -S . -B build && cmake --build build
ctest --test-dir build --output-on-failure
./build/edgeflow -c configs/gateway.json
tail -5 /tmp/edgeflow/edgeflow.log
```

**预期：**

- `test_device_model` / `test_config` / `test_adapter` 均 passed
- 终端：`EdgeFlow M3 OK — polled 3 telemetry point(s)`
- 日志含 `adapter poll` 与 `bms_001.soc_percent` 等行

---

## 四、下一步：M4 Modbus 模拟设备接入

| 任务 | 文件 | 验收 |
| --- | --- | --- |
| Modbus CRC16 | `ingress/modbus_rtu.c` | `test_modbus_crc` |
| 替换/补充 stub | `modbus_rtu` Adapter | poll 路径与 CRC 单测通过 |

---

## 五、文档索引

| 文档 | 何时阅读 |
| --- | --- |
| [LEARNING_PATH.md](LEARNING_PATH.md) | **每次开发第一件事** |
| [DEVICE_MODEL.md](DEVICE_MODEL.md) | M1 |
| [MODULE_DESIGN.md](MODULE_DESIGN.md) | 每步动手写代码前 |
| [ARCHITECTURE.md](ARCHITECTURE.md) | M5 之前浏览即可 |
| [IMPLEMENTATION_STATUS.md](IMPLEMENTATION_STATUS.md) | 确认能否写进简历 |
| [ROADMAP.md](ROADMAP.md) | 三个月目标 |

---

## 六、变更记录

| 日期 | 说明 |
| --- | --- |
| 2026-06-25 | M3：adapter 注册表、stub_adapter、test_adapter |
| 2026-06-25 | M2：config/logger |
