# 示例场景与功能映射

## 对标的工业网关常见能力

工业边缘网关/控制器类项目通常涉及以下能力，本项目以此为参照选择实现子集：

- 工业设备接入、协议适配和设备状态建模。
- Modbus RTU/TCP、TCP、UDP、MQTT 等通信协议。
- Linux 多线程、epoll、Socket、CLI、systemd 和 watchdog。
- 状态机、告警联锁、指令调度和边缘本地自治。
- 网络编程、现场调试和稳定性优化。
- 断网本地运行、日志、看门狗、长期稳定运行。

本项目只选择可落地、可验证、能体现 Linux 系统软件和工业控制器能力的子集。BMS/PCS/Meter、分时电价和削峰填谷作为内置示例场景，不代表项目要实现真实商业 EMS。下表中部分策略（分时电价、需量限制、防逆流）为目标场景，当前代码仅实现单一削峰，详见 [IMPLEMENTATION_STATUS.md](IMPLEMENTATION_STATUS.md)。

## 功能映射表

| 市场需求 | 项目功能 | 验证方式 |
| --- | --- | --- |
| 工业设备接入 | BMS/PCS/Meter 模拟器 + Modbus RTU/TCP adapter | 设备状态周期刷新，metrics 统计采集成功率 |
| 统一模型抽象 | Device/Point/Telemetry/Alarm/Command | 业务层不直接访问协议数据 |
| 状态机 | State And Alarm Engine | 设备离线、急停、告警恢复可复现 |
| 指令调度 | Command Scheduler | 指令状态可追踪，支持超时重试和回读 |
| 削峰填谷 | 根据负载功率和峰谷时段生成充放电功率 | 模拟负载超过阈值时输出放电指令 |
| 分时电价 | TOU tariff 表驱动策略 | 谷时充电、峰时放电、平时待机 |
| 最大需量限制 | 电网功率接近上限时限制充电或放电支撑 | grid_power_kw 超阈值触发策略 |
| 告警联锁 | SOC、温度、急停、消防、设备离线触发停机/降级 | 触发 alarm_event 并改变 SystemState |
| 断网本地自治 | 本地策略不依赖云端，缓存待补传 | broker 停止 30 分钟后策略仍运行 |
| 数据补传 | SQLite WAL + upload_cursor | 网络恢复后按游标补传 |
| 现场排障 | metrics、结构化日志、CLI、TROUBLESHOOTING | 输出断网、离线、指令失败闭环 |

## 削峰填谷策略

输入：

- `load_power_kw`
- `grid_power_kw`
- `soc_percent`
- `tou_period`
- `pcs_available`
- `active_alarm_level`

规则：

```text
if active_alarm_level >= MAJOR:
    target_power = 0
elif tou_period == VALLEY and soc < charge_soc_limit:
    target_power = +charge_power_kw
elif tou_period == PEAK and soc > discharge_soc_limit:
    target_power = -discharge_power_kw
elif load_power_kw > peak_shaving_threshold and soc > discharge_soc_limit:
    target_power = -min(discharge_power_kw, load_power_kw - threshold)
else:
    target_power = 0
```

说明：

- 正功率表示充电。
- 负功率表示放电。
- 首版不做复杂收益优化，只做可解释、可验证的控制逻辑。

## 最大需量限制

目标：避免电网侧功率超过合同需量或设定上限。

规则：

```text
if grid_power_kw >= demand_limit_kw:
    target_power = -min(discharge_power_kw, grid_power_kw - demand_limit_kw + reserve_kw)
elif grid_power_kw >= demand_warning_kw:
    forbid_charge = true
```

验证：

- 模拟 `grid_power_kw` 从 100kW 上升到 150kW。
- 当超过 `demand_limit_kw` 后，策略输出放电目标功率。
- 如果 SOC 不足，生成 `DEMAND_LIMIT_SOC_LOW` 告警。

## 防逆流控制

目标：避免储能向电网反送电。

规则：

```text
if grid_power_kw <= anti_backflow_margin_kw:
    target_power = min(target_power, 0)
    if target_power < 0:
        target_power = 0
```

验证：

- 模拟光伏或负载变化导致 `grid_power_kw` 接近 0。
- 策略停止放电，生成策略解释：`anti_backflow limit active`。

## 断网本地自治

要求：

- MQTT broker 不可达时，设备采集、状态机、联锁、策略和指令下发继续运行。
- 云端上报失败只影响 `upload_cursor` 和缓存积压。
- 关键告警和控制命令必须本地落库。

验证：

```text
1. 启动系统和设备模拟器。
2. 停止 broker 30 分钟。
3. 观察 SystemState 和 ControlCommand 是否继续更新。
4. 恢复 broker。
5. 检查缓存是否按 upload_cursor 补传。
```

## 指令回读校验

目标：避免“指令发出即认为成功”的假控制闭环。

流程：

```text
Command Scheduler 发送 set_power 指令
  -> PCS adapter 写目标功率
  -> 延时 readback_interval_ms
  -> 读取 PCS actual_power_kw
  -> abs(actual - target) <= tolerance 判定成功
  -> 否则重试或生成告警
```

失败场景：

- PCS 离线。
- PCS 拒绝指令。
- 实际功率长时间偏离目标。
- 重试次数耗尽。

必须生成：

- `control_commands` 审计记录。
- `PCS_COMMAND_TIMEOUT` 或 `PCS_POWER_DEVIATION` 告警。
- metrics 中命令失败计数增长。

## 不做清单

三个月首版不做：

- IEC61850 全量协议。
- 真实电网调度。
- 复杂功率预测算法。
- 电池 SOX 算法。
- 并网保护认证。
- 生产级 BMS/PCS 私有协议适配。

这些内容作为能力边界说明，不能写成已完成能力。
