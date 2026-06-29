# 设计决策记录

> 自用的"为什么这么设计"备忘，便于回看代码时回忆取舍。标注 `[已实现]` / `[目标]` 区分当前状态。

## Adapter 为什么禁止业务逻辑 [已实现]

通信层只负责协议收发、基础解析和错误上报。若把 SOC 联锁、削峰等业务写进 Adapter，新增 Modbus TCP/UDP/MQTT 设备时会复制业务规则，难以维护。

做法：Adapter 输出统一 `telemetry_t`，业务层只面向 Device Model，协议变化不影响状态机和策略；单元测试可脱离真实设备。

## 业务层为什么不直接访问协议数据 [已实现]

Modbus 寄存器、串口节点是协议细节，业务层直接访问会造成耦合、状态机难复用。

做法：协议层把寄存器/帧转换成 `Point`/`Telemetry`，`state_engine` 只处理统一模型。

## 为什么用状态机 [部分实现]

设备离线、急停、告警恢复、降级运行需要明确的状态转换，否则容易重复告警、恢复抖动、误下发指令。

现状：当前只实现 `INIT`/`RUNNING`/`FAULT` 三态与温度/SOC 告警；`STANDBY`/`DEGRADED`/`STOPPED`、急停联锁、告警去抖为目标。

## 控制命令为什么必须经过 Scheduler [部分实现]

策略只负责决策，不直接写设备。真实现场写命令可能超时、失败、被拒，必须有审计与回读。

现状：`command_scheduler` 为每条命令分配 `command_id`，跟踪 `PENDING/SENT/ACKED/VERIFIED/FAILED` 并写审计；超时/重试为目标。

## SQLite 为什么用 WAL [已实现]

边缘设备需要同时写 telemetry/告警/命令审计，并在网络恢复后读取待补传数据。WAL 减少读写互相阻塞，异常恢复比普通文件追加可靠。

## 断网时如何本地自治 [已实现]

云端不可达只影响上报，不影响本地采集、状态机、策略、指令调度。上报失败只增加缓存积压 metrics，数据写入 SQLite（`uploaded=0`），网络恢复后由 monitor 线程批量补传。

## epoll/Reactor 与线程的分工 [部分实现]

Reactor 负责 fd 与定时事件就绪通知，阻塞/耗时任务放到独立线程。

现状：当前是固定三线程（ingress/worker/monitor），Reactor 仅承载一个 timerfd（周期触发 watchdog/补传/PING）；多 fd Reactor + 线程池为目标。

## MQTT 会话为什么要加锁 [已实现]

`mqtt_session` 的 socket fd 被 worker（实时 publish）与 monitor（补传/PING/心跳）两个线程共享。无锁并发 `write` 会交错 MQTT 报文、关闭中的 fd 也会产生 race。

做法：给 `mqtt_session` 加 `pthread_mutex`，拆出"持锁内部函数"与"加锁公开 API"避免非递归锁自死锁；`replay` 先取 SQLite 数据再进锁以缩短临界区。

## MQTT Remaining Length 为什么要变长编码 [已实现]

MQTT 固定报头的剩余长度 >127 字节必须用变长字节（每字节低 7 位有效、最高位为续传位）。单字节编码会让真实 broker 误判长度而断连。telemetry JSON 常超过 127 字节。

## systemd watchdog 与线程心跳如何配合 [已实现]

线程 heartbeat 监控模块内部健康，systemd watchdog 监控进程整体健康。每个核心线程周期更新 heartbeat，monitor 线程喂狗；若线程卡死、心跳超时可被观测，进程异常时由 systemd 拉起。
