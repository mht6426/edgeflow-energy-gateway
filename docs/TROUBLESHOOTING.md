# EMS 排障案例

## 案例 1：MQTT broker 不可达但站端控制继续运行

### 现象

云端订阅不到 telemetry 和 alarm，但本地 BMS/PCS/Meter 状态仍在刷新，策略仍在生成 PCS 目标功率。

### 指标

```text
edgeflow_mqtt_publish_fail_total 持续增长
edgeflow_upload_backlog_records 增长
edgeflow_bms_poll_success_total 持续增长
edgeflow_strategy_decision_total 持续增长
edgeflow_command_success_total 持续增长
```

### 根因

broker 不可达或网络断开。EMS 控制器设计要求云端不可达不影响本地自治，因此采集、状态机、策略和命令调度继续运行，上报数据进入 SQLite WAL 缓存。

### 修复

1. 检查 broker 地址、端口和网络路由。
2. 检查板端 DNS、网关和防火墙。
3. 确认 `upload_cursor` 未损坏。
4. 恢复 broker 后观察补传是否开始。

### 验证

```bash
mosquitto_sub -t 'ems/site001/#'
```

恢复后：

```text
edgeflow_mqtt_publish_ok_total 增长
edgeflow_upload_backlog_records 下降
edgeflow_system_state 保持正常
```

## 案例 2：PCS 实际功率长期偏离目标功率

### 现象

策略输出 `target_power_kw = -50kW`，但 PCS 回读 `actual_power_kw = -10kW`，持续超过允许偏差。

### 指标

```text
edgeflow_command_retry_total 增长
edgeflow_command_failed_total 增长
edgeflow_alarm_active_total 增长
edgeflow_pcs_power_deviation_kw 超过阈值
```

### 根因

可能原因：

- PCS 拒绝功率指令。
- PCS 处于故障或限功率状态。
- 指令寄存器地址或缩放系数配置错误。
- PCS 回读寄存器和写入寄存器不是同一语义。

### 修复

1. 检查 PCS `online` 和 `alarm_code`。
2. 检查 `device_profile` 中目标功率寄存器和 scale。
3. 增加指令回读延迟，排除设备响应慢。
4. 超过重试次数后进入降级策略，目标功率置 0。

### 验证

- `control_commands` 中失败指令有完整审计。
- `PCS_POWER_DEVIATION` 告警生成。
- 系统状态进入 `FAULT` 或 `OFFLINE_DEGRADED`。
- 修复配置后回读偏差恢复到阈值以内。

## 案例 3：BMS 离线导致系统禁止充放电

### 现象

BMS 采集失败，系统从 `CHARGING` 或 `DISCHARGING` 切换到 `OFFLINE_DEGRADED`，PCS 目标功率变为 0。

### 指标

```text
edgeflow_bms_poll_fail_total 增长
edgeflow_device_online{device="bms"} 变为 0
edgeflow_system_state 变为 OFFLINE_DEGRADED
edgeflow_pcs_target_power_kw 变为 0
```

### 根因

BMS 通信超时、RS485 接线异常、CAN 设备异常或设备地址配置错误。EMS 控制器无法确认 SOC 和温度时，必须禁止继续充放电。

### 修复

1. 检查 RS485/CAN 接线和终端电阻。
2. 检查 BMS 设备地址、波特率和寄存器点表。
3. 检查串口权限和设备节点。
4. 通信恢复后清除离线告警。

### 验证

- BMS 采集成功计数恢复增长。
- `BMS_OFFLINE` 告警恢复。
- 系统回到 `STANDBY`。
- 策略重新允许充/放电。

## 案例 4：策略错误触发防逆流限制

### 现象

峰时段本应放电，但 `grid_power_kw` 接近 0，策略输出目标功率为 0。

### 指标

```text
edgeflow_strategy_reason = STRATEGY_ANTI_BACKFLOW
edgeflow_grid_power_kw <= anti_backflow_margin_kw
edgeflow_pcs_target_power_kw = 0
```

### 根因

防逆流策略优先级高于削峰填谷。站点负载较低或光伏出力较高时，继续放电可能造成反送电，因此策略限制 PCS 放电。

### 修复

这不是程序错误。需要检查：

- `anti_backflow_margin_kw` 设置是否合理。
- 电表方向是否配置反了。
- 电网功率符号定义是否和策略一致。

### 验证

修正电表方向或阈值后，策略解释日志与目标功率符合预期。
