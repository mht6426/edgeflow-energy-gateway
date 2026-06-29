# EdgeFlow 开发进度总览

> **用途**：快速了解当前进度。实现边界以 [IMPLEMENTATION_STATUS.md](IMPLEMENTATION_STATUS.md) 为准，运行时数据流见 [RUNTIME_FLOW.md](RUNTIME_FLOW.md)。

**当前状态**：M1–M10 + 进阶闭环已完成（三线程 + SQLite WAL + MQTT 长连接补传 + CLI + watchdog + epoll monitor）
**下一方向**：板端 24h soak test、策略扩展（TOU/需量/防逆流）、Modbus TCP、MQTT TLS/鉴权

---

## 一、进度

```text
M1–M10 基础闭环                     [████████████████████] 100%  ✅
进阶（SQLite/补传/CLI/epoll/watchdog） [████████████████████] 100%  ✅
板端 soak test / 策略扩展 / TLS        [░░░░░░░░░░░░░░░░░░░░]   0%  ⏳
```

---

## 二、当前代码地图

```text
edgeflow-energy-gateway/
├── src/
│   ├── main.c                      # 进程入口：配置→日志→三线程运行时→优雅退出
│   ├── model/device_model.*        # 统一设备模型
│   ├── platform/config.*           # JSON 配置（手写扁平解析）
│   ├── platform/logger.*           # 文件日志
│   ├── platform/storage.*          # SQLite WAL + JSONL + 补传查询
│   ├── platform/metrics.*          # Prometheus 文本 metrics
│   ├── platform/watchdog.*         # systemd notify watchdog
│   ├── platform/heartbeat.*        # 线程心跳
│   ├── common/time_util.*          # 单调时钟
│   ├── core/ring.*                 # SPSC 无锁队列
│   ├── core/reactor.*              # epoll + timerfd 周期 Reactor
│   ├── ingress/adapter.*           # Adapter 插件注册表
│   ├── ingress/stub_adapter.*      # 演示采集
│   ├── ingress/modbus_*            # Modbus RTU + CRC16 + termios 串口
│   ├── runtime/state_engine.*      # 状态机 + 削峰 + 告警
│   ├── runtime/command_scheduler.* # 指令调度 + 模拟回读 + 审计
│   ├── runtime/app.*               # 三线程编排
│   ├── egress/mqtt_session.*       # MQTT 长连接 + 补传（线程安全）
│   └── cli/main.c                  # edgeflow-cli
├── tests/                          # 8 个 ctest 单测
├── configs/                        # gateway.json（开发）/ gateway.prod.json（板端）
└── docs/                           # 设计与实现文档
```

---

## 三、验证当前能力

```bash
cmake -S . -B build && cmake --build build
ctest --test-dir build --output-on-failure
./build/edgeflow -c configs/gateway.json
tail -20 /tmp/edgeflow/edgeflow.log
```

---

## 四、文档索引

| 文档 | 何时阅读 |
| --- | --- |
| [IMPLEMENTATION_STATUS.md](IMPLEMENTATION_STATUS.md) | 确认已实现 vs 规划边界 |
| [RUNTIME_FLOW.md](RUNTIME_FLOW.md) | 对照源码看数据流 |
| [LEARNING_PATH.md](LEARNING_PATH.md) | 逐模块开发记录 |
| [ARCHITECTURE.md](ARCHITECTURE.md) | 目标架构（含未实现项标注） |
| [DESIGN_DECISIONS.md](DESIGN_DECISIONS.md) | 关键设计取舍 |
| [CAPABILITY_CHECKLIST.md](CAPABILITY_CHECKLIST.md) | 能力自查清单 |
