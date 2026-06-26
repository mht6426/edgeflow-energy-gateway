#ifndef EDGEFLOW_CONFIG_H
#define EDGEFLOW_CONFIG_H

/*
 * Config Manager（M2）— 网关运行参数
 *
 * 平台：Linux（配置文件路径、串口节点均为 Linux 惯例）
 *
 * 职责：
 *   - 从 JSON 文件加载扁平键值，填充 gateway_config_t 快照
 *   - 提供 gateway_config_defaults() 作为缺省回退
 *
 * 不负责：
 *   - 配置热更新（M10）、JSON schema 校验
 *   - 设备点表 / 寄存器映射（M4 configs/maps/）
 *   - 解释业务策略（只存阈值，策略在 M5 读取）
 *
 * 数据流：
 *   configs/gateway.json → gateway_config_load() → gateway_config_t → 各运行时模块只读
 *
 * 调用时机：main 线程启动时加载一次；reload 留待 M10。
 */

#include <stdbool.h>
#include <stdint.h>

/*
 * 网关全局配置快照（值语义结构体，可整体拷贝传递）。
 *
 * 字段按「采集 / 策略 / 上报 / 平台路径」分组，便于后续模块按需读取。
 */
typedef struct {
    /* --- 网关身份 --- */
    char device_id[32];              /* 网关 ID，如 gw-001；用于 MQTT、日志标识 */

    /* --- 设备采集（M4 Adapter / M8 运行时消费） --- */
    bool simulate;                     /* true：使用内置模拟器；false：真实串口（M4 实现） */
    char serial_device[128];           /* Linux 串口路径，如 /dev/ttyUSB0 */
    uint32_t poll_interval_ms;         /* 轮询周期（毫秒） */
    uint32_t queue_size;               /* SPSC 队列容量（M8） */

    /* --- 策略阈值（M5 State Engine / 策略引擎消费） --- */
    double deadband;                   /* 死区，避免抖动反复触发（M5 使用） */
    double temp_high;                  /* BMS 最高单体温度上限（℃），超限告警 */
    double peak_shaving_threshold_kw;    /* 削峰：电网功率超过此值触发放电（kW） */
    double max_discharge_kw;           /* 最大放电功率上限（kW） */
    double min_discharge_soc;          /* 允许放电的最低 SOC（%） */

    /* --- MQTT 上报（M9 Egress 消费） --- */
    char broker_host[128];             /* Broker 地址，如 127.0.0.1 */
    uint16_t broker_port;              /* Broker 端口，默认 1883 */
    char mqtt_topic[128];              /* 遥测发布主题 */

    /* --- 平台路径（Linux 文件系统） --- */
    char log_dir[128];                 /* 日志目录，如 /tmp/edgeflow 或 /var/log/edgeflow */
    char metrics_path[128];            /* Prometheus 文本文件路径（M10） */
    char cache_path[128];              /* 离线缓存 JSONL 路径（M7） */
} gateway_config_t;

/*
 * 填充内置默认值（不读文件）。
 * 典型用途：单元测试、JSON 缺字段时 gateway_config_load 已先调用本函数。
 */
void gateway_config_defaults(gateway_config_t *cfg);

/*
 * 从 path 读取 JSON 并合并到 cfg。
 * 流程：defaults → read_file → 逐字段 json_get_* 覆盖。
 *
 * @param path 配置文件路径（相对或绝对，Linux 路径）
 * @param cfg  输出配置快照，不可为 NULL
 * @return 0 成功，-1 文件不存在/读取失败/参数非法
 */
int gateway_config_load(const char *path, gateway_config_t *cfg);

#endif
