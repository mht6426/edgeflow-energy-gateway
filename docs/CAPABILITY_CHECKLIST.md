# 已实现能力自查清单

> 自用清单：只勾选**代码里确实跑通**的能力，便于自查与对照 `IMPLEMENTATION_STATUS.md`。未实现项见末尾。

## 系统/运行时

- [x] 三线程运行时（ingress / worker / monitor），`SIGINT/SIGTERM` 优雅退出
- [x] SPSC 无锁环形队列解耦采集与处理（C11 atomic，满队列丢弃计数）
- [x] epoll + timerfd 周期 Reactor（monitor 线程）
- [x] 线程心跳 + stale 检测
- [x] systemd `Type=notify` watchdog 集成

## 设备接入

- [x] Adapter 插件注册表 + `poll_all` 聚合
- [x] Modbus RTU：CRC16 校验、模拟数据、termios 真实串口读写（含超时）
- [ ] Modbus TCP / 裸 TCP / UDP（未实现）

## 业务

- [x] 统一设备模型：`device_t`/`point_t`/`telemetry_t`/`alarm_event_t`/`command_t`
- [x] 状态机三态（`INIT`/`RUNNING`/`FAULT`）+ 温度高/SOC 低告警
- [x] 单一削峰策略（`grid_power_kw` 超阈值生成放电 `Command`）
- [x] Command Scheduler：`command_id` + 五态 + 模拟回读 + 审计
- [ ] 分时电价 / 需量限制 / 防逆流 / 恒功率（未实现）
- [ ] 超时/重试、急停/消防联锁（未实现）

## 存储与上报

- [x] SQLite WAL：telemetry / alarms / commands 三表 + `uploaded` 标志 + 索引
- [x] JSONL 审计追加
- [x] 自研 MQTT 3.1.1（CONNECT/PUBLISH/PINGREQ，QoS0，线程锁，变长 Remaining Length）
- [x] worker 实时上报 + monitor 断网批量补传
- [ ] `upload_cursor`/`runtime_kv` 表、批量打包、HTTP 上报、TLS/鉴权（未实现）

## 可观测 / 工具

- [x] Prometheus 文本 metrics（队列水位、丢弃、待补传、补传计数、心跳 age）
- [x] CLI：`validate-config` / `status` / `storage-stats`
- [x] 结构化日志文件
- [ ] CLI `devices`/`alarms`/`commands`、SIGHUP 配置热加载（未实现）

## 工程化

- [x] CMake 构建，`-Wall -Wextra -Wpedantic`
- [x] 8 个 ctest 单元测试
- [x] GitHub Actions CI（构建 + 测试）
- [x] aarch64 交叉编译 toolchain
- [ ] 板端 24h soak test 实测数据（见 `SOAK_TEST_RECORD.md`，按真实运行情况填写）
