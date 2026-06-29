# 交付路线

## 目标

按阶段把项目从最小闭环推进到可在 ARM Linux 板端长期运行的工业边缘控制器，覆盖软件架构、协议通信、可靠控制、现场排障与产品化交付。不做电力系统潮流/并网算法。

## 第 1 月：工业控制平台最小闭环

目标：证明项目不是普通采集网关，而是具备统一设备模型、状态机和指令闭环的控制器运行时。

交付：

- 项目定位、README、架构文档重构为 EdgeFlow Industrial Controller。
- `PROJECT_SPEC.md` 固化项目边界和禁止项。
- BMS/PCS/Meter/TempFire 模拟器。
- Modbus RTU/TCP 至少一条可运行采集链路。
- 统一设备模型：`Device`、`Point`、`Telemetry`、`Alarm`、`Command`。
- 基础状态机：待机、充电、放电、故障、停机、离线降级。
- 基础告警：SOC 高/低、温度高、PCS 离线、急停。
- PCS 目标功率指令模拟下发和审计日志。

验收：

- 能看到 SOC、电表功率、PCS 状态周期刷新。
- SOC 越界能触发禁止充/放电。
- 急停能触发停机。
- 策略生成的 PCS 目标功率进入命令队列。
- 指令下发结果可追溯。

## 第 2 月：策略与可靠性

目标：形成工业边缘控制平台核心竞争力。

交付：

- 分时电价策略。
- 削峰填谷策略。
- 最大需量限制。
- 防逆流控制。
- SQLite WAL 本地缓存。
- MQTT 批量上报 telemetry、alarm、command_result、heartbeat。
- 指令超时、重试和回读校验。
- 断网 30 分钟恢复补传。

验收：

- 峰谷时段变化会改变充放电策略。
- 负载超过阈值时输出放电目标功率。
- 电网功率接近需量上限时限制充电或放电支撑。
- PCS 实际功率偏离目标时触发告警。
- broker 不可达时本地控制不停止。
- broker 恢复后缓存按游标补传。

## 第 3 月：产品化与现场可用

目标：可在板端长期运行、可观测、可排障。

交付：

- CLI 工具：状态、设备、告警、策略、缓存、配置校验。
- 配置热加载：SIGHUP reload 或显式 CLI reload。
- Prometheus metrics。
- thread heartbeat。
- systemd watchdog。
- 24h soak test。
- 故障场景测试文档。

验收：

- RK3568 板端 `systemctl status edgeflow` 为 active。
- 24h 运行无异常退出。
- 有 CPU、RSS、队列水位、缓存积压、补传延迟、告警数数据。
- 至少 3 个排障闭环：断网、设备离线、指令失败。
- 文档表述与仓库实现一致，不夸大真实能力。

## 最小可用版本 DoD

- ARM Linux 板端可运行。
- BMS/PCS/Meter 模拟器可运行。
- 有状态机和告警联锁。
- 有至少一种示例策略：分时电价或削峰填谷。
- 有控制命令下发、回读校验和审计记录。
- 有 SQLite 或等价可靠缓存设计与验证。
- 有 MQTT 上报和断网补传。
- 有 metrics、日志、systemd/watchdog。
- 有 24h 测试计划或实测记录。

## 进阶加分项

- CAN BMS stub。
- IEC104 基础遥测上报 demo。
- Buildroot rootfs 集成。
- HTTP 状态查询接口。

只有实际做通后，才在文档中标记为已实现。
