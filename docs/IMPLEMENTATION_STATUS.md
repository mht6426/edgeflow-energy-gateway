# 实现状态

**最后更新**：2026-06-25

## 当前运行时模型（进阶版）

```text
main (SIGINT/SIGTERM 优雅退出)
  └── gateway_app
        ├── ingress_thread  → modbus_adapter.poll → SPSC ring
        ├── worker_thread   → SQLite WAL + JSONL → state_engine → command_scheduler → MQTT 长连接
        └── monitor_thread  → epoll/timerfd → watchdog / 心跳 / MQTT 补传 / PING
```

## 模块实现对照

| 模块 | 状态 | 说明 |
| --- | --- | --- |
| M1 统一设备模型 | ✅ | `device_model.*` |
| M2 Config + Logger | ✅ | `config.*`、`logger.*`（含进阶配置项） |
| M3 Adapter 注册表 | ✅ | `adapter.*`、`stub_adapter.*` |
| M4 Modbus | ✅ | 模拟 + `modbus_serial.*` 真实串口 |
| M5 状态机 / 策略 | ✅ | `state_engine.*` |
| M6 Command Scheduler | ✅ | `command_scheduler.*` |
| M7 存储 | ✅ | SQLite WAL + JSONL 审计 `storage.*` |
| M8 运行时 | ✅ | 三线程 `ring.*`、`app.*`、`reactor.*` |
| M9 MQTT | ✅ | 长连接 `mqtt_session.*` + 批量补传 |
| M10 Metrics | ✅ | 含心跳/补传/待上传计数 |
| CLI | ✅ | `edgeflow-cli` validate-config / status / storage-stats |
| Watchdog | ✅ | `watchdog.*` NOTIFY_SOCKET |
| Heartbeat | ✅ | `heartbeat.*` + metrics |

## 测试

| 测试 | 状态 |
| --- | --- |
| test_device_model | ✅ |
| test_config | ✅ |
| test_adapter | ✅ |
| test_ring | ✅ |
| test_modbus_crc | ✅ |
| test_state_engine | ✅ |
| test_command_scheduler | ✅ |
| test_storage | ✅ |

详见 [RUNTIME_FLOW.md](RUNTIME_FLOW.md)。

## 配置文件

| 文件 | 用途 |
| --- | --- |
| `configs/gateway.json` | 开发机默认（simulate=true） |
| `configs/gateway.prod.json` | 板端模板（simulate=false） |

## CLI 示例

```bash
./build/edgeflow-cli validate-config -c configs/gateway.json
./build/edgeflow-cli status --metrics /tmp/edgeflow/metrics.prom
./build/edgeflow-cli storage-stats --sqlite /tmp/edgeflow/edgeflow.db
```
