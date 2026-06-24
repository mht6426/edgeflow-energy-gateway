# 模块开发模板

## 开发原则

不要一次生成整个项目。每个模块都必须按以下顺序推进：

```text
设计 -> 代码 -> 单元测试 -> 集成测试
```

每次输出代码前，必须先说明：

1. 模块职责。
2. 输入输出。
3. 数据流。
4. 线程模型。
5. 异常处理。

## 模块说明模板

### 模块名称

填写模块名，例如 `Config Manager`。

### 模块职责

说明模块负责什么，不负责什么。

示例：

```text
Config Manager 负责加载、校验和热更新 JSON 配置。
不负责解释业务策略，不直接操作设备。
```

### 输入输出

输入：

- 配置文件路径。
- reload 信号。
- CLI validate-config 请求。

输出：

- 只读配置快照。
- 配置校验结果。
- reload 结果。

### 数据流

```text
file -> parser -> validator -> immutable snapshot -> runtime modules
```

### 线程模型

说明模块在哪个线程运行，是否跨线程共享，是否需要锁。

示例：

```text
配置加载在 main 线程执行。
reload 由 platform_monitor 触发。
运行时模块通过 atomic shared_ptr 或 rwlock 读取配置快照。
```

### 异常处理

必须列出错误路径：

- 文件不存在。
- JSON 格式错误。
- 字段缺失。
- 参数越界。
- reload 失败。

### 单元测试

必须覆盖：

- 正常配置。
- 缺失字段。
- 越界字段。
- 非法 JSON。
- reload 保留旧配置。

### 集成测试

必须覆盖：

- 进程启动加载配置。
- SIGHUP reload。
- CLI validate-config。
- 错误配置不影响当前运行配置。

## 模块推进顺序

建议顺序：

1. Unified Device Model。
2. Config Manager + Logger。
3. Device Adapter 插件接口。
4. Modbus TCP 模拟设备接入。
5. State And Alarm Engine。
6. Command Scheduler。
7. SQLite WAL Storage。
8. Reactor + Thread Pool。
9. MQTT Uploader。
10. CLI、Metrics、Watchdog、systemd。

## 代码质量要求

- C 模块默认使用 C11。
- 复杂业务对象和状态机可使用 C++17。
- 模块边界必须清晰，禁止循环依赖。
- 所有 fd、socket、SQLite handle 必须有明确释放路径。
- 所有线程必须支持优雅退出。
- 所有跨线程队列必须有容量和背压策略。
- 错误码和日志必须能定位到模块、设备和操作。
