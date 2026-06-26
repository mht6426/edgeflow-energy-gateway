# ingress — 设备接入（Device Adapter）

**平台**：Linux（串口 `/dev/ttyUSB*`、后续 socket）

**学习阶段**：

| 文件 | 阶段 | 状态 |
| --- | --- | --- |
| `adapter.h/c` | M3 插件接口与注册表 | ✅ |
| `stub_adapter.h/c` | M3 演示插件（simulate 时 3 点位） | ✅ |
| `modbus_rtu.*` | M4 Modbus + CRC + 真实/模拟串口 | ⏳ |

**数据流**：

```text
adapter_registry_poll_all() → telemetry_t[] → （M8 队列）→ State Engine
```

实现新 Adapter 请遵循 [CODE_STYLE.md](../../docs/CODE_STYLE.md)。
