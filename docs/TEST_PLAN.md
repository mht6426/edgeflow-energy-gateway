# 模块级测试计划

## 基础构建

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/edgeflow -c configs/gateway.json
```

## Device Adapter Layer

单元测试：

- Modbus RTU CRC 正确。
- Modbus TCP frame 编解码正确。
- 设备超时返回明确错误码。
- Adapter 不产生业务告警和策略结果。

集成测试：

- 模拟 BMS/PCS/Meter 周期生成 `Telemetry`。
- TCP 断开后自动重连或标记设备离线。
- 串口打开失败时进程不崩溃。

## Unified Device Model

单元测试：

- `Device` 在线/离线状态转换。
- `Point` scale/offset 转换。
- `Telemetry` quality 标记。
- `Command` 状态转换。

集成测试：

- Adapter 输出协议数据后只通过统一模型进入业务层。
- 状态机和策略引擎不直接依赖 Modbus 寄存器。

## State And Alarm Engine

单元测试：

- 设备离线触发告警。
- SOC 越界触发告警。
- 温度高触发告警。
- 急停进入 STOPPED。
- 告警恢复需要满足恢复条件和恢复时间窗。

集成测试：

- 模拟 telemetry 缺失导致设备离线。
- 告警联锁优先级高于策略引擎输出。

## Strategy Engine

单元测试：

- 分时电价。
- 削峰填谷。
- 最大需量限制。
- 恒功率充放电。
- 告警状态下输出安全目标。

集成测试：

- 策略输出 `Command`，不直接操作 Adapter。
- 策略解释文本能说明为什么输出该命令。

## Command Scheduler

单元测试：

- command_id 唯一。
- 超时后重试。
- 重试耗尽后失败。
- 回读校验成功进入 VERIFIED。
- 回读校验失败进入 FAILED。

集成测试：

- 命令结果写入 SQLite。
- 指令失败生成 Alarm。
- Adapter 异常不会导致 Scheduler 线程退出。

## Storage Layer

单元测试：

- SQLite WAL 初始化。
- telemetry 插入。
- alarm 插入和恢复。
- command 审计。
- upload_cursor 前进。
- 数据清理。

集成测试：

- broker 不可达时缓存积压增长。
- broker 恢复后按游标补传。
- 异常退出后数据库可恢复。

## Reactor + Thread Pool

单元测试：

- epoll 注册和注销 fd。
- timerfd 周期触发。
- 任务入队和执行。
- 线程池优雅退出。

集成测试：

- TCP/UDP/MQTT socket 由 Reactor 管理。
- 阻塞式 Modbus RTU 轮询不阻塞 Reactor。
- 队列满时触发背压 metrics。

## Platform Layer

单元测试：

- JSON 配置校验。
- logger 输出。
- metrics 格式。
- thread heartbeat 更新。

集成测试：

- SIGHUP reload。
- CLI validate-config。
- systemd watchdog。
- kill -STOP 触发 watchdog 恢复。

## 24h Soak Test

记录：

- 设备采集成功率。
- 控制命令成功/失败/超时次数。
- 告警总数和未恢复告警数。
- MQTT 成功/失败次数。
- SQLite 缓存积压最大值。
- CPU 与 RSS。
- thread heartbeat 是否正常。
- systemd 是否发生重启。
