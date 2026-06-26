# 实现状态

**最后更新**：2026-06-25

## 当前运行时模型

```text
main
  ├── gateway_config_load()        # M2
  ├── logger_init()                # M2
  ├── adapter_registry_register()  # M3
  ├── adapter_registry_init_all()
  ├── adapter_registry_poll_all()  # → telemetry_t[]
  └── logger_shutdown()
```

## 模块实现对照

| 模块 | 状态 | 说明 |
| --- | --- | --- |
| 统一设备模型（M1） | ✅ | `device_model.*` |
| Config + Logger（M2） | ✅ | `config.*`、`logger.*` |
| Adapter 插件接口（M3） | ✅ | `adapter.*`、`stub_adapter.*` |
| Modbus RTU（M4） | ⏳ | — |
| 状态机 / Scheduler（M5-M6） | ⏳ | — |
| 运行时 / MQTT（M7-M10） | ⏳ | — |

## 测试覆盖

| 测试 | 状态 |
| --- | --- |
| `test_device_model` | ✅ |
| `test_config` | ✅ |
| `test_adapter` | ✅ |
