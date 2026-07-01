# ingress — 设备接入（Device Adapter）

**平台**：Linux（串口、TCP socket）

| 文件 | 说明 |
| --- | --- |
| `adapter.h/c` | 插件接口与注册表 |
| `stub_adapter.*` | 演示插件 |
| `modbus_adapter.*` | Modbus 插件封装 |
| `modbus_rtu.c` | 模拟器 + CRC16 |
| `modbus_libmodbus.c` | libmodbus RTU/TCP 真实采集 |
| `modbus_serial.*` | 历史自研串口实现（保留参考，主链路已用 libmodbus） |

**数据流**：`adapter_registry_poll_all()` → `telemetry_t[]` → SPSC → State Engine

实现新 Adapter 请遵循 [开发指南.md](../../docs/开发指南.md)。
