#ifndef EDGEFLOW_DEVICE_MODEL_H
#define EDGEFLOW_DEVICE_MODEL_H

/*
 * EdgeFlow 统一设备模型（M1）
 *
 * 平台：Linux 用户态（与目标 ARM 板端运行时一致；无 OS 特定 API，纯 C 数据结构）
 *
 * 设计目标：
 *   业务层（状态机、策略、存储、MQTT）只处理本文件定义的五类对象，
 *   禁止直接访问 Modbus 寄存器、串口 fd、TCP frame 等协议细节。
 *
 * 五类核心对象：
 *   device_t      — 设备实例（静态元数据 + 连接健康）
 *   point_t       — 测点定义（静态元数据，来自配置/点表）
 *   telemetry_t   — 一次采集结果（运行时上行数据）
 *   alarm_event_t — 告警事件（运行时，可持久化/上报）
 *   command_t     — 控制指令（运行时下行，带状态机）
 *
 * 实现约定：
 *   - 字符串字段使用固定长度 char[]，值语义，可整块 memcpy 进 SPSC 队列
 *   - 本模块只提供类型定义与构造辅助函数，不含注册表、协议解析、状态机逻辑
 *   - 详见 docs/架构与设计.md
 */

#include <stdint.h>

/* 字符串缓冲区上限（嵌入式场景避免堆分配） */
#define EDGEFLOW_ID_MAX 64       /* 设备/测点/告警码 ID */
#define EDGEFLOW_NAME_MAX 64     /* 人类可读名称 */
#define EDGEFLOW_UNIT_MAX 16       /* 工程单位，如 "%"、"kW"、"C" */
#define EDGEFLOW_MESSAGE_MAX 128   /* 告警描述、命令结果等文本 */

/*
 * 设备接入协议类型。
 * 仅 Adapter 层根据此字段选择驱动；业务层不应依赖 protocol 做分支判断。
 */
typedef enum {
    DEVICE_PROTOCOL_MODBUS_RTU = 0,
    DEVICE_PROTOCOL_MODBUS_TCP,
    DEVICE_PROTOCOL_TCP,
    DEVICE_PROTOCOL_UDP,
    DEVICE_PROTOCOL_SIMULATOR,   /* 开发/测试用内置模拟器 */
} device_protocol_t;

/*
 * 设备连接健康状态。
 * 由 Adapter 在采集成功/失败时通过 device_mark_online / device_mark_failure 更新。
 */
typedef enum {
    DEVICE_ONLINE_UNKNOWN = 0,   /* 尚未完成首次采集 */
    DEVICE_ONLINE_ONLINE,        /* 通信正常 */
    DEVICE_ONLINE_OFFLINE,       /* 连续失败达到阈值，判定离线 */
    DEVICE_ONLINE_DEGRADED,      /* 偶发失败，降级运行 */
} device_online_state_t;

/* 测点数据类型，影响 Adapter 解析方式与 Command 下发语义 */
typedef enum {
    POINT_TYPE_ANALOG = 0,       /* 连续量，如 SOC、温度、功率 */
    POINT_TYPE_DIGITAL,          /* 开关量，如急停、断路器状态 */
    POINT_TYPE_ENUM,             /* 枚举/discrete，如 PCS 运行模式 */
} point_type_t;

/*
 * 遥测质量位（IEC 61850 风格简化）。
 * 通信成功不等于值可信：传感器故障、人工 override、规则过滤均可标 BAD。
 * 策略引擎应拒绝使用 quality != GOOD 的 telemetry。
 */
typedef enum {
    TELEMETRY_QUALITY_GOOD = 0,
    TELEMETRY_QUALITY_BAD,
    TELEMETRY_QUALITY_UNCERTAIN,
} telemetry_quality_t;

/* 告警严重等级，用于联锁优先级（CRITICAL 可压制策略输出） */
typedef enum {
    ALARM_LEVEL_INFO = 0,
    ALARM_LEVEL_MINOR,
    ALARM_LEVEL_MAJOR,
    ALARM_LEVEL_CRITICAL,
} alarm_level_t;

/* 告警生命周期状态（恢复时写入 recovered_ts_ms） */
typedef enum {
    ALARM_STATE_ACTIVE = 0,
    ALARM_STATE_RECOVERED,
} alarm_state_t;

/*
 * 控制命令类型。
 * 策略层只构造 command_t；具体协议写操作由 Adapter + Command Scheduler 完成。
 */
typedef enum {
    COMMAND_TYPE_SET_POINT_VALUE = 0,  /* 写目标测点，如 PCS 目标功率 */
    COMMAND_TYPE_SET_DEVICE_MODE,      /* 切换设备模式 */
    COMMAND_TYPE_STOP_DEVICE,          /* 停机/急停类命令 */
} command_type_t;

/*
 * 命令执行状态机（完整生命周期由 M6 Command Scheduler 驱动）。
 *
 * 典型流转：
 *   PENDING → SENT → ACKED → VERIFIED
 *                    └→ TIMEOUT → RETRYING → ...
 *                    └→ FAILED
 */
typedef enum {
    COMMAND_STATE_PENDING = 0,
    COMMAND_STATE_SENT,
    COMMAND_STATE_ACKED,
    COMMAND_STATE_VERIFIED,
    COMMAND_STATE_TIMEOUT,
    COMMAND_STATE_RETRYING,
    COMMAND_STATE_FAILED,
} command_state_t;

/*
 * 控制器整机运行状态（M5 State Engine 维护，非单设备状态）。
 *
 * 典型流转：
 *   INIT → STANDBY → RUNNING
 *   RUNNING → FAULT（如 BMS 温度高）
 *   任意 → STOPPED（急停）
 *   RUNNING → DEGRADED（部分设备离线）
 */
typedef enum {
    SYSTEM_STATE_INIT = 0,
    SYSTEM_STATE_STANDBY,
    SYSTEM_STATE_RUNNING,
    SYSTEM_STATE_DEGRADED,
    SYSTEM_STATE_FAULT,
    SYSTEM_STATE_STOPPED,
} system_state_t;

/*
 * 设备实例。
 *
 * 表示「一个可采集/可控制的工业设备」，如 bms_001、pcs_001、meter_001。
 * 业务逻辑、MQTT topic、日志均引用 id，不引用 Modbus 从站地址。
 */
typedef struct {
    char id[EDGEFLOW_ID_MAX];              /* 全局唯一设备 ID */
    char name[EDGEFLOW_NAME_MAX];          /* 显示名称 */
    device_protocol_t protocol;            /* 接入协议，Adapter 使用 */
    device_online_state_t online_state;    /* 当前连接健康状态 */
    uint64_t last_seen_ms;                 /* 最近一次成功/失败通信时间戳 */
    uint32_t consecutive_failures;         /* 连续采集失败次数，用于离线判定 */
    char last_error[EDGEFLOW_MESSAGE_MAX]; /* 最近一次错误描述，便于排障 */
} device_t;

/*
 * 测点定义（静态元数据，不随时间变化）。
 *
 * 与 telemetry_t 的关系：
 *   point_t  回答「这个测点是什么、怎么换算、限值多少」
 *   telemetry_t 回答「此刻该测点的值是多少」
 *
 * 工程值换算：engineering = raw * scale + offset
 * 点表来源：configs/maps/*.json（M4 接入时加载）
 */
typedef struct {
    char id[EDGEFLOW_ID_MAX];       /* 测点 ID，如 soc_percent、grid_power_kw */
    char device_id[EDGEFLOW_ID_MAX]; /* 所属设备 ID */
    char name[EDGEFLOW_NAME_MAX];   /* 显示名称 */
    point_type_t type;              /* 模拟量/开关量/枚举 */
    char unit[EDGEFLOW_UNIT_MAX];   /* 工程单位 */
    double scale;                   /* 原始值 → 工程值 比例系数 */
    double offset;                  /* 原始值 → 工程值 偏移量 */
    double low_limit;               /* 合法下限，供告警联锁（M5） */
    double high_limit;              /* 合法上限，供告警联锁（M5） */
} point_t;

/*
 * 遥测数据（运行时上行，每条采集产生一条）。
 *
 * 设计为自包含结构体：进 SPSC 队列、写 JSONL、发 MQTT 时无需再查 point_t。
 * Adapter 负责将协议原始值换算为工程值后填入 value。
 */
typedef struct {
    uint64_t ts_ms;                      /* 采集时间戳（单调时钟，毫秒） */
    char device_id[EDGEFLOW_ID_MAX];     /* 来源设备 */
    char point_id[EDGEFLOW_ID_MAX];      /* 测点 ID */
    double value;                        /* 工程值（已完成 scale/offset 换算） */
    char unit[EDGEFLOW_UNIT_MAX];        /* 单位冗余一份，便于序列化 */
    telemetry_quality_t quality;         /* 数据质量位 */
    uint16_t source_id;                  /* Adapter 实例 ID，用于多源追踪与调试 */
} telemetry_t;

/*
 * 告警事件（可持久化、可上报、有生命周期）。
 *
 * code 使用字符串（如 BMS_TEMP_HIGH、EMERGENCY_STOP），
 * 避免 magic number，便于配置化与云端对齐。
 */
typedef struct {
    uint64_t alarm_id;                   /* 全局递增告警 ID */
    uint64_t ts_ms;                      /* 告警产生时间 */
    char device_id[EDGEFLOW_ID_MAX];     /* 关联设备 */
    char code[EDGEFLOW_ID_MAX];          /* 告警码，如 BMS_TEMP_HIGH */
    alarm_level_t level;                 /* 严重等级 */
    alarm_state_t state;                 /* ACTIVE / RECOVERED */
    char message[EDGEFLOW_MESSAGE_MAX];  /* 人类可读描述 */
    uint64_t recovered_ts_ms;            /* 恢复时间，ACTIVE 时为 0 */
} alarm_event_t;

/*
 * 控制命令（下行「工单」模型）。
 *
 * 职责分离：
 *   策略引擎  → 构造 command_t，state = PENDING
 *   Scheduler → 驱动 SENT/ACKED/VERIFIED/FAILED（M6）
 *   Adapter   → 将 command 翻译为 Modbus 写寄存器等操作（M4）
 *
 * 禁止策略模块直接写设备。
 */
typedef struct {
    uint64_t command_id;                     /* 全局递增命令 ID */
    uint64_t created_ts_ms;                  /* 命令创建时间 */
    char target_device_id[EDGEFLOW_ID_MAX];   /* 目标设备，如 pcs_001 */
    command_type_t type;                     /* 命令类型 */
    command_state_t state;                   /* 执行状态 */
    char point_id[EDGEFLOW_ID_MAX];          /* 目标测点，如 target_power_kw */
    double target_value;                     /* 目标工程值，如 -25.0 kW（放电） */
    int retry_count;                         /* 当前已重试次数 */
    int max_retries;                         /* 最大重试次数上限 */
    char result_message[EDGEFLOW_MESSAGE_MAX]; /* 执行结果/回读校验说明 */
} command_t;

/* ---------- 枚举 → 字符串（日志、调试、单测断言） ---------- */

const char *telemetry_quality_to_string(telemetry_quality_t quality);
const char *system_state_to_string(system_state_t state);
const char *command_state_to_string(command_state_t state);

/* ---------- 构造与维护辅助函数（实现见 device_model.c） ---------- */

/*
 * 初始化设备实例，online_state 置为 UNKNOWN。
 * 返回 0 成功，-1 参数非法。
 */
int device_init(device_t *device,
                const char *id,
                const char *name,
                device_protocol_t protocol);

/*
 * 初始化测点定义。low_limit/high_limit 由调用方按需赋值（默认 0）。
 * scale/offset 用于 Adapter 原始值换算，见 point_raw_to_engineering()。
 */
int point_init(point_t *point,
               const char *id,
               const char *device_id,
               const char *name,
               point_type_t type,
               const char *unit,
               double scale,
               double offset);

/* 原始值 → 工程值：raw * scale + offset */
double point_raw_to_engineering(const point_t *point, double raw_value);

/* 采集成功：置 ONLINE，清零 consecutive_failures */
void device_mark_online(device_t *device, uint64_t ts_ms);

/*
 * 采集失败：递增 consecutive_failures，记录 last_error。
 * 1~2 次失败 → DEGRADED；≥3 次 → OFFLINE。
 */
void device_mark_failure(device_t *device, uint64_t ts_ms, const char *error);

/*
 * 构造一条遥测，默认 quality = GOOD。
 * value 应为工程值（Adapter 已完成换算）。
 */
void telemetry_init(telemetry_t *telemetry,
                    uint64_t ts_ms,
                    const char *device_id,
                    const char *point_id,
                    double value,
                    const char *unit,
                    uint16_t source_id);

/*
 * 构造一条设点命令（COMMAND_TYPE_SET_POINT_VALUE），state = PENDING。
 * 示例：PCS 放电 target_value = -25.0（kW，负号表示放电方向）。
 */
int command_init_setpoint(command_t *command,
                          uint64_t command_id,
                          uint64_t created_ts_ms,
                          const char *target_device_id,
                          const char *point_id,
                          double target_value,
                          int max_retries);

/* 将命令标记为 VERIFIED，写入 result_message（M6 回读校验后调用） */
int command_mark_verified(command_t *command, const char *message);

/*
 * 构造一条 ACTIVE 告警。
 * 告警恢复由 State Engine 另建 RECOVERED 记录或更新 state（M5）。
 */
int alarm_make(alarm_event_t *alarm,
               uint64_t alarm_id,
               uint64_t ts_ms,
               const char *device_id,
               const char *code,
               alarm_level_t level,
               const char *message);

#endif
