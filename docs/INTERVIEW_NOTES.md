# 面试导向设计说明

## 为什么 Adapter 禁止业务逻辑

通信层只负责协议收发、基础解析和错误上报。如果把 SOC 联锁、削峰填谷等业务逻辑写进 Adapter，后续新增 Modbus TCP、UDP 或 MQTT 设备时会复制业务规则，导致维护困难。

工程回答：

```text
Adapter 的输出是统一 Telemetry，业务层只面向 Device Model。
这样协议变化不会影响状态机和策略引擎，单元测试也可以脱离真实设备。
```

## 为什么业务层不能直接访问协议数据

Modbus 寄存器、TCP frame、串口节点是协议细节。业务层直接访问这些细节，会造成模块耦合，并使状态机难以复用。

工程回答：

```text
协议层负责把寄存器和 frame 转换为 Point/Telemetry。
State Engine 和 Strategy Engine 只处理统一模型。
```

## 为什么使用状态机

工业控制器不是简单 if/else。设备离线、急停、告警恢复、降级运行都需要明确状态转换，否则容易出现重复告警、恢复抖动和控制指令误下发。

工程回答：

```text
状态机可以把 INIT、RUNNING、DEGRADED、FAULT、STOPPED 的转换条件固定下来。
告警联锁优先级高于策略引擎，避免故障状态下继续下发控制命令。
```

## 为什么控制命令必须经过 Scheduler

策略引擎只负责决策，不直接写设备。真实现场中，写命令可能超时、失败、被设备拒绝，必须有重试、回读和审计。

工程回答：

```text
Command Scheduler 为每条命令分配 command_id，跟踪 PENDING、SENT、ACKED、VERIFIED、FAILED 状态。
这样可以追踪“策略想做什么、实际发了什么、设备是否执行成功”。
```

## 为什么 SQLite 使用 WAL

边缘设备需要同时写入 telemetry、告警、命令审计，并在网络恢复后读取待补传数据。WAL 模式更适合读写并发和异常恢复。

工程回答：

```text
SQLite WAL 能减少读写互相阻塞，断电后恢复也比普通文件可靠。
控制命令和告警属于审计数据，不能只写内存或普通 append 文件。
```

## 断网时如何保证本地自治

云端不可达只影响上报，不影响本地采集、状态机、策略和命令调度。

工程回答：

```text
Uploader 失败后只推进缓存积压 metrics，不阻塞 Adapter、State Engine 和 Command Scheduler。
数据写入 SQLite WAL，网络恢复后按 upload_cursor 补传。
```

## 设备离线如何降级

Adapter 连续失败后更新 Device 在线状态。State Engine 根据关键设备类型决定进入 DEGRADED、FAULT 或 STOPPED。

工程回答：

```text
例如 BMS 离线时无法确认 SOC 和温度，策略引擎必须禁止充放电。
非关键设备离线则只产生告警，不一定停机。
```

## 指令可靠性如何保证

可靠性来自命令状态机、超时重试、回读校验和审计日志。

工程回答：

```text
命令发送成功不代表执行成功。
必须读取设备实际状态或目标寄存器，确认实际值和目标值在容差范围内，才进入 VERIFIED。
```

## epoll/Reactor 和 Thread Pool 如何分工

Reactor 负责大量 fd 和定时事件的就绪通知，Thread Pool 负责可能阻塞或耗时的任务。

工程回答：

```text
TCP、UDP、MQTT socket 和 timerfd 适合放在 epoll/Reactor。
Modbus RTU 串口轮询、SQLite 写入、策略计算和命令回读放在线程池，避免阻塞 Reactor。
```

## ARM Linux 部署时 systemd/watchdog/heartbeat 如何配合

线程 heartbeat 监控模块内部健康，systemd watchdog 监控进程整体健康。

工程回答：

```text
每个核心线程周期更新 heartbeat。
platform_monitor 检查 heartbeat，如果线程卡死则停止喂狗，让 systemd 拉起进程。
这样可以处理死锁、阻塞和线程异常退出。
```
