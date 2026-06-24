# 实现状态

本文档说明**当前代码已实现**与**文档规划但未实现**的边界，避免对外过度承诺。

最后更新：2026-06-24

## 当前运行时模型

```text
main
  └── gateway_app
        ├── ingress_thread  (Modbus RTU 模拟采集 → SPSC ring)
        └── worker_thread   (规则过滤 → 控制环 → JSONL 缓存 → MQTT 发布 → metrics)
```

> 架构文档中的 epoll/Reactor、7 worker 线程模型为**规划目标**，当前为双线程最小闭环。

## 模块实现对照

| 模块 | 状态 | 说明 |
| --- | --- | --- |
| 统一设备模型类型 | ✅ 已实现 | `Device`/`Point`/`Telemetry`/`Alarm`/`Command` 类型与辅助函数 |
| Device/Point 运行时注册 | ⏳ 未实现 | 类型已定义，运行时未使用 |
| Modbus RTU CRC16 | ✅ 已实现 | 标准 CRC 算法 |
| Modbus RTU 串口采集 | ⏳ 未实现 | `simulate:true` 使用内置模拟器；`simulate:false` 返回错误 |
| SPSC 无锁队列 | ✅ 已实现 | ingress/worker 解耦 |
| 状态机（基础） | ✅ 部分 | INIT/RUNNING/FAULT；STANDBY/DEGRADED/STOPPED 未完整 |
| 削峰填谷策略 | ✅ 原型 | 可配置阈值，策略输出经模拟回读标记 VERIFIED |
| 分时电价 / 需量限制 / 防逆流 | ⏳ 未实现 | 见 ROADMAP 第 2 月 |
| Command Scheduler | ⏳ 未实现 | 无超时/重试/真实回读 |
| JSONL 本地缓存 | ✅ 已实现 | 追加写 telemetry/alarm/command |
| SQLite WAL | ⏳ 未实现 | 规划替代 JSONL |
| MQTT 上报 | ✅ 基础 | 每点短连接 PUBLISH，校验 CONNACK |
| MQTT 断网补传 | ⏳ 未实现 | 无 upload_cursor |
| Prometheus metrics 文件 | ✅ 已实现 | 周期性写 metrics.prom |
| 结构化日志 | ✅ 已实现 | 写文件或 stderr |
| JSON 配置加载 | ✅ 基础 | 手写解析，无 schema 校验 |
| CLI | ⏳ 未实现 | |
| systemd 部署 | ✅ 脚本 | 无 watchdog 喂狗（已移除 WatchdogSec） |
| 24h soak test | ⏳ 模板 | 见 SOAK_TEST_RECORD.md |

## 配置字段使用情况

| 字段 | 状态 |
| --- | --- |
| `device_id` | ✅ 使用 |
| `simulate` | ✅ 使用（false 时禁用模拟采集） |
| `serial_device` | ⏳ 预留（串口采集未实现） |
| `poll_interval_ms` | ✅ 使用 |
| `queue_size` | ✅ 使用 |
| `deadband` | ⏳ 预留 |
| `temp_high` | ✅ 使用 |
| `peak_shaving_threshold_kw` | ✅ 使用 |
| `max_discharge_kw` | ✅ 使用 |
| `min_discharge_soc` | ✅ 使用 |
| `broker_*` / `mqtt_topic` | ✅ 使用 |
| `log_dir` / `metrics_path` / `cache_path` | ✅ 使用 |

## 简历与面试边界

**可以写（需与代码一致）：**

- ARM Linux 工业边缘控制平台原型
- 插件化 Adapter 设计（Modbus RTU 模拟 + CRC 路径）
- 统一设备模型、基础状态机、削峰填谷策略原型
- SPSC 队列双线程运行时、JSONL 缓存、MQTT 上报、metrics

**不要写（尚未实现）：**

- SQLite WAL 断网缓存与补传
- epoll/Reactor + Thread Pool 完整并发模型
- 指令调度超时/重试/真实回读
- 24h 稳定性测试通过（除非实测完成）
- 生产级 BMS/PCS 协议栈

## 演进路线

- 里程碑进度与续开发指引：[PROGRESS.md](PROGRESS.md)
- 三个月阶段目标：[ROADMAP.md](ROADMAP.md)
