# Unified Device Model

## 设计原则

业务层禁止直接访问协议层数据。Modbus 寄存器、TCP frame、MQTT topic、串口设备节点都只能存在于 Device Adapter Layer。上层模块只处理统一模型：

- `Device`
- `Point`
- `Telemetry`
- `Alarm`
- `Command`

这样做的目的是让新增协议或设备时不改状态机、策略引擎、存储和上报模块。

## Device

表示一个工业设备实例。

```cpp
enum class DeviceProtocol {
    ModbusRtu,
    ModbusTcp,
    Tcp,
    Udp,
    Simulator,
};

enum class DeviceOnlineState {
    Unknown,
    Online,
    Offline,
    Degraded,
};

struct Device {
    std::string id;
    std::string name;
    DeviceProtocol protocol;
    DeviceOnlineState online_state;
    uint64_t last_seen_ms;
    uint32_t consecutive_failures;
    std::string last_error;
};
```

## Point

表示设备测点定义。

```cpp
enum class PointType {
    Analog,
    Digital,
    Enum,
};

struct Point {
    std::string id;
    std::string device_id;
    std::string name;
    PointType type;
    std::string unit;
    double scale;
    double offset;
    double low_limit;
    double high_limit;
};
```

## Telemetry

表示一次采集结果。

```cpp
enum class Quality {
    Good,
    Bad,
    Uncertain,
};

struct Telemetry {
    uint64_t ts_ms;
    std::string device_id;
    std::string point_id;
    double value;
    Quality quality;
};
```

## Alarm

表示告警产生、持续和恢复。

```cpp
enum class AlarmLevel {
    Info,
    Minor,
    Major,
    Critical,
};

enum class AlarmState {
    Active,
    Recovered,
};

struct Alarm {
    uint64_t alarm_id;
    uint64_t ts_ms;
    std::string device_id;
    std::string code;
    AlarmLevel level;
    AlarmState state;
    std::string message;
    uint64_t recovered_ts_ms;
};
```

## Command

表示控制命令。

```cpp
enum class CommandType {
    SetPointValue,
    SetDeviceMode,
    StopDevice,
};

enum class CommandState {
    Pending,
    Sent,
    Acked,
    Verified,
    Timeout,
    Retrying,
    Failed,
};

struct Command {
    uint64_t command_id;
    uint64_t created_ts_ms;
    std::string target_device_id;
    CommandType type;
    CommandState state;
    std::string point_id;
    double target_value;
    int retry_count;
    int max_retries;
    std::string result_message;
};
```

## EMS 示例 Profile

EMS/储能只是示例业务场景，可以由通用模型表达：

- BMS SOC：`Device = bms_001`，`Point = soc_percent`
- BMS 温度：`Point = max_cell_temp_c`
- PCS 实际功率：`Device = pcs_001`，`Point = actual_power_kw`
- PCS 目标功率命令：`CommandType::SetPointValue`，`point_id = target_power_kw`
- 电表电网功率：`Device = meter_001`，`Point = grid_power_kw`
- 急停告警：`Alarm.code = EMERGENCY_STOP`

设计重点：项目不是为某个固定 EMS 设备写死模型，而是通过统一模型承载不同工业设备和示例策略。
