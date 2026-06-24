# EdgeFlow 开发进度总览

> **用途**：在不同设备上 `git clone` 后，先看本文档即可知道「整体路线图在哪」「当前做到哪」「下一步做什么」。
>
> **维护约定**：每完成一个节点，更新本文档顶部的「当前节点」、对应条目的状态和 `最后更新` 日期，并提交 Git。

**最后更新**：2026-06-24  
**当前节点**：`P1.6` — 第 1 月最小闭环（进行中，约 **42%**）  
**下一任务**：`P1.7` 补全基础状态机与告警联锁（SOC 越界、急停、PCS 离线）

---

## 一、30 秒速览

| 项目 | 内容 |
| --- | --- |
| 产品名 | EdgeFlow Industrial Controller |
| 仓库目录名 | `edgeflow-energy-gateway`（历史命名，可不改） |
| 目标平台 | Linux / ARM64（RK3568/RK3588） |
| 当前架构 | 双线程 + SPSC 队列（非文档中的 7 worker） |
| 整体进度 | Phase 0 完成；Phase 1 进行中；Phase 2/3 未开始 |
| 详细实现边界 | [IMPLEMENTATION_STATUS.md](IMPLEMENTATION_STATUS.md) |
| 三个月路线 | [ROADMAP.md](ROADMAP.md) |

### 进度条（按里程碑节点加权估算）

```text
Phase 0 仓库基线        [████████████████████] 100%
Phase 1 第1月最小闭环   [████████░░░░░░░░░░░░]  42%
Phase 2 第2月策略可靠性 [░░░░░░░░░░░░░░░░░░░░]   0%
Phase 3 第3月产品化     [░░░░░░░░░░░░░░░░░░░░]   0%
Phase 4 最低可投 DoD    [███░░░░░░░░░░░░░░░░░]  15%
Phase 5 进阶加分项      [░░░░░░░░░░░░░░░░░░░░]   0%
```

---

## 二、里程碑节点总览

图例：`✅ 已完成` · `🔄 进行中` · `⏳ 未开始` · `⏸ 阻塞/待定`

### Phase 0 — 仓库与文档基线

| 节点 | 内容 | 状态 | 关键产出 |
| --- | --- | --- | --- |
| P0.1 | GitHub 仓库卫生 | ✅ | `.gitignore`、`LICENSE`(MIT)、移除 `build/` |
| P0.2 | CI 自动构建测试 | ✅ | `.github/workflows/ci.yml` |
| P0.3 | 项目定位与边界文档 | ✅ | `README.md`、`PROJECT_SPEC.md` |
| P0.4 | 架构与设计文档 | ✅ | `ARCHITECTURE.md`、`MODULE_DESIGN.md` 等 12 份 |
| P0.5 | 实现边界声明 | ✅ | `IMPLEMENTATION_STATUS.md` |
| P0.6 | 部署脚本与 systemd | ✅ | `deploy/`（已移除未实现的 WatchdogSec） |

### Phase 1 — 第 1 月：工业控制平台最小闭环

| 节点 | 内容 | 状态 | 关键产出 / 文件 |
| --- | --- | --- | --- |
| P1.1 | 统一设备模型（类型层） | ✅ | `src/model/device_model.{c,h}` |
| P1.2 | 配置加载与默认值 | ✅ | `src/platform/config.{c,h}`、`configs/gateway.json` |
| P1.3 | SPSC 队列 + 双线程运行时 | ✅ | `src/core/ring.{c,h}`、`src/runtime/app.{c,h}` |
| P1.4 | Modbus RTU CRC + 模拟采集 | ✅ | `src/ingress/modbus_rtu.{c,h}` |
| P1.5 | 削峰填谷策略（可配置阈值） | ✅ | `src/runtime/control_loop.{c,h}` |
| P1.6 | JSONL 缓存 + 日志 + metrics | ✅ | `src/platform/storage/logger/metrics.*` |
| P1.7 | 基础状态机（全状态） | 🔄 | 仅有 INIT/RUNNING/FAULT；缺 STANDBY/DEGRADED/STOPPED |
| P1.8 | 基础告警联锁 | 🔄 | 仅 `BMS_TEMP_HIGH`；缺 SOC 越界、急停、PCS 离线 |
| P1.9 | 独立设备模拟器 | ⏳ | 当前点位内嵌在 `modbus_rtu.c`；缺 BMS/PCS/Meter/TempFire 独立模块 |
| P1.10 | Modbus RTU 真实串口 | ⏳ | `simulate:false` 仅告警返回，未读写 `/dev/ttyUSB*` |
| P1.11 | Modbus TCP 采集链路 | ⏳ | 无 `modbus_tcp.c` |
| P1.12 | Device/Point 运行时注册 | ⏳ | 类型已定义，运行时未使用 |
| P1.13 | Command Scheduler（真实） | ⏳ | 策略内直接 `command_mark_verified`，无队列/超时/重试 |
| P1.14 | PCS 指令模拟下发 | 🔄 | 有命令对象 + JSONL 审计；无真实 PCS adapter 写操作 |
| P1.15 | 第 1 月验收测试 | ⏳ | 部分单测有；缺集成测试与验收 checklist 打勾 |

### Phase 2 — 第 2 月：策略与可靠性

| 节点 | 内容 | 状态 | 关键产出 |
| --- | --- | --- | --- |
| P2.1 | 分时电价（TOU）策略 | ⏳ | — |
| P2.2 | 最大需量限制 | ⏳ | — |
| P2.3 | 防逆流控制 | ⏳ | — |
| P2.4 | SQLite WAL 本地缓存 | ⏳ | 当前为 JSONL |
| P2.5 | MQTT 长连接 + 批量上报 | ⏳ | 当前每点短连接 |
| P2.6 | 上报 alarm / command_result / heartbeat | ⏳ | 仅 telemetry |
| P2.7 | 指令超时、重试、回读校验 | ⏳ | — |
| P2.8 | 断网 30min 本地自治 + 补传 | ⏳ | 无 `upload_cursor` |
| P2.9 | 第 2 月验收测试 | ⏳ | — |

### Phase 3 — 第 3 月：产品化与求职证据

| 节点 | 内容 | 状态 | 关键产出 |
| --- | --- | --- | --- |
| P3.1 | CLI 工具 | ⏳ | — |
| P3.2 | 配置热加载（SIGHUP / CLI reload） | ⏳ | — |
| P3.3 | Prometheus metrics 完善 | 🔄 | 基础 6 项已有；缺 CPU/RSS/补传延迟等 |
| P3.4 | thread heartbeat | ⏳ | — |
| P3.5 | systemd watchdog 喂狗 | ⏳ | 已从 unit 移除 WatchdogSec，待实现后再加回 |
| P3.6 | RK3568 板端部署验证 | ⏳ | 有脚本，无实测记录 |
| P3.7 | 24h soak test | ⏳ | `SOAK_TEST_RECORD.md` 为模板 |
| P3.8 | 故障闭环文档实测 | ⏳ | `TROUBLESHOOTING.md` 部分指标为规划项 |
| P3.9 | 简历 / 面试稿与代码对齐 | 🔄 | `RESUME_SNIPPETS.md` 已有，需随实现更新 |

### Phase 4 — 最低可投版本（DoD）

| 条目 | 状态 |
| --- | --- |
| ARM Linux 板端可运行 | ⏳ |
| BMS/PCS/Meter 模拟器可运行 | 🔄（内嵌模拟，非独立模块） |
| 状态机和告警联锁 | 🔄 |
| 至少一种策略（削峰或 TOU） | ✅（削峰填谷原型） |
| 控制命令下发、回读校验、审计 | 🔄（模拟回读） |
| SQLite 或等价可靠缓存 | 🔄（JSONL，非 SQLite） |
| MQTT 上报和断网补传 | 🔄（上报有，补传无） |
| metrics、日志、systemd/watchdog | 🔄（watchdog 无） |
| 24h 测试计划或实测记录 | ⏳ |

### Phase 5 — 进阶加分项

| 节点 | 状态 |
| --- | --- |
| CAN BMS stub | ⏳ |
| IEC104 基础遥测 demo | ⏳ |
| Buildroot rootfs 集成 | ⏳ |
| HTTP 状态查询接口 | ⏳ |
| epoll/Reactor + Thread Pool | ⏳ |

---

## 三、当前代码地图（clone 后对照）

```text
edgeflow-energy-gateway/
├── src/
│   ├── main.c                 # 入口、信号处理、配置路径
│   ├── common/                # time_util、datapoint 别名
│   ├── core/ring.*            # SPSC 无锁队列 ✅
│   ├── model/device_model.*   # 统一模型类型 + 辅助函数 ✅
│   ├── ingress/modbus_rtu.*   # CRC ✅ / 模拟采集 ✅ / 串口 ⏳
│   ├── egress/mqtt_client.*   # MQTT 短连接发布 ✅
│   ├── platform/
│   │   ├── config.*           # JSON 配置 ✅
│   │   ├── logger.*           # 日志 ✅
│   │   ├── metrics.*          # Prometheus 文件 ✅
│   │   └── storage.*          # JSONL 缓存 ✅
│   └── runtime/
│       ├── app.*              # 双线程运行时 ✅
│       └── control_loop.*     # 状态机原型 + 削峰策略 ✅
├── tests/
│   ├── test_ring.c            # ✅
│   ├── test_device_model.c    # ✅
│   ├── test_control_loop.c    # ✅
│   ├── test_modbus_crc.c      # ✅
│   ├── test_config.c          # ✅
│   └── tools/mock_modbus.py   # 占位脚本 ⏳
├── configs/
│   ├── gateway.json           # 开发配置（/tmp/edgeflow）
│   ├── gateway.prod.json      # 板端配置（/var、/run）
│   └── maps/slave1.json       # 点表示例，运行时未加载 ⏳
├── deploy/                    # install.sh + systemd ✅
├── toolchain/                 # aarch64 交叉编译 ✅
└── docs/                      # 设计文档 + 本进度文档
```

### 当前运行时数据流

```text
ingress_thread
  modbus_rtu_poll() [simulate:true → 3 个模拟点位]
    → spsc_ring_push()
worker_thread
  spsc_ring_pop()
    → apply_rules()          # 温度标记 BAD quality
    → storage_append_point() # JSONL
    → control_loop_process_telemetry()
         ├─ 温度高 → FAULT + BMS_TEMP_HIGH 告警
         └─ 电网功率超阈值 → PCS 放电命令（模拟 VERIFIED）
    → storage_append_alarm/command()
    → mqtt_publish_point()   # 短连接 + CONNACK 校验
    → metrics_write_file()   # 每秒
```

---

## 四、新设备接续开发（标准流程）

### 4.1 拉取与构建

```bash
git clone <你的仓库地址>
cd edgeflow-energy-gateway

# 确认当前节点（应与本文件顶部一致）
head -n 12 docs/PROGRESS.md

cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

### 4.2 验证当前能力

```bash
# 终端 1：启动（无需 broker 也可跑，MQTT 失败计入 metrics）
./build/edgeflow -c configs/gateway.json

# 终端 2：观察输出
tail -f /tmp/edgeflow/edgeflow.log
cat /tmp/edgeflow/metrics.prom
cat /tmp/edgeflow/offline-cache.jsonl   # 有 telemetry / alarm / command 记录
```

**预期现象（P1.6 节点）：**

- 日志出现 `edgeflow started: ... simulate=true`
- metrics 中 `edgeflow_ingress_points_total` 持续增长
- 当模拟 `grid_power_kw` 较高时，日志出现 `command verified`
- 当模拟温度过高时，日志出现 `alarm active: ... BMS_TEMP_HIGH`

### 4.3 交叉编译（可选，RK 板端）

```bash
cmake -S . -B build-aarch64 -DCMAKE_TOOLCHAIN_FILE=toolchain/aarch64-linux-gnu.cmake
cmake --build build-aarch64
# scp 到板端后：
# sudo deploy/scripts/install.sh build-aarch64/edgeflow
```

---

## 五、推荐开发顺序（从这里继续）

> 按 ROADMAP 第 1 月验收标准，优先补齐 **P1.7 → P1.8 → P1.13**，再进入第 2 月。

### 下一步：P1.7 + P1.8 基础状态机与告警

| 步骤 | 任务 | 建议修改文件 | 验收 |
| --- | --- | --- | --- |
| 1 | 补全 `SYSTEM_STATE_STANDBY/DEGRADED/STOPPED` 转换 | `control_loop.c` | 单测覆盖各状态转换 |
| 2 | SOC 高/低越界 → 禁止充放电 + 告警 | `control_loop.c`、`config.h` | `test_control_loop.c` 新增用例 |
| 3 | 急停数字量输入 → `SYSTEM_STATE_STOPPED` | `modbus_rtu.c`（模拟点位）、`control_loop.c` | 手动/单测触发急停 |
| 4 | PCS 离线检测（连续采集失败） | `app.c`、`control_loop.c` | 模拟器可开关 offline |
| 5 | 更新 `IMPLEMENTATION_STATUS.md` 和本文档 | `docs/` | 节点状态改为 ✅ |

### 再下一步：P1.13 Command Scheduler

| 步骤 | 任务 | 建议新建文件 | 验收 |
| --- | --- | --- | --- |
| 1 | 新建命令队列与状态机 | `src/runtime/command_scheduler.{c,h}` | 命令状态 PENDING→SENT→VERIFIED/FAILED |
| 2 | 策略只产出 Command，不直接 verify | `control_loop.c` | 职责分离 |
| 3 | 模拟回读放到 scheduler | `command_scheduler.c` | 可配置超时/重试 |
| 4 | 单测 + 集成测试 | `tests/test_command_scheduler.c` | ctest 通过 |

### 第 2 月入口：P2.1 ~ P2.4

完成 Phase 1 验收后，按 `ROADMAP.md` 顺序：TOU → 需量 → 防逆流 → SQLite WAL。

---

## 六、测试覆盖现状

| 测试 | 复盖模块 | 状态 |
| --- | --- | --- |
| `test_ring` | SPSC 队列 | ✅ |
| `test_device_model` | 模型辅助函数 | ✅ |
| `test_control_loop` | 削峰放电、温度告警 | ✅ |
| `test_modbus_crc` | CRC16 | ✅ |
| `test_config` | 配置加载 | ✅ |
| `test_command_scheduler` | 指令调度 | ⏳ 待建 |
| `test_storage` | JSONL 缓存 | ⏳ 待建 |
| `test_mqtt` | MQTT 客户端 | ⏳ 待建 |
| 集成测试（短时运行 edgeflow） | 端到端 | ⏳ 待建 |

---

## 七、文档维护 checklist

每完成一个节点，提交代码时同步更新：

- [ ] 本文档 `docs/PROGRESS.md` 顶部「当前节点」和进度条
- [ ] `docs/IMPLEMENTATION_STATUS.md` 模块对照表
- [ ] 若行为变化：`configs/gateway.json` 示例字段
- [ ] 若可写简历：`docs/RESUME_SNIPPETS.md`
- [ ] 若板端验证：`docs/SOAK_TEST_RECORD.md`

### 节点状态更新模板

```markdown
**最后更新**：YYYY-MM-DD
**当前节点**：`P1.8` — 描述
**下一任务**：`P1.9` — 描述
```

---

## 八、相关文档索引

| 文档 | 何时阅读 |
| --- | --- |
| [PROGRESS.md](PROGRESS.md) | **每次 clone / 换设备第一件事** |
| [IMPLEMENTATION_STATUS.md](IMPLEMENTATION_STATUS.md) | 确认能否写进简历 |
| [ROADMAP.md](ROADMAP.md) | 看三个月阶段目标 |
| [ARCHITECTURE.md](ARCHITECTURE.md) | 设计新模块前 |
| [MODULE_DESIGN.md](MODULE_DESIGN.md) | 写代码前（职责/线程/异常） |
| [TEST_PLAN.md](TEST_PLAN.md) | 补测试用例时 |
| [DEPLOY.md](DEPLOY.md) | 板端部署时 |
| [INTERVIEW_NOTES.md](INTERVIEW_NOTES.md) | 准备面试时 |

---

## 九、变更记录

| 日期 | 节点 | 说明 |
| --- | --- | --- |
| 2026-06-24 | P0.1–P0.6 | GitHub 发布基线：gitignore、LICENSE、CI、README 重构 |
| 2026-06-24 | P1.1–P1.6 | 双线程运行时、模拟采集、削峰策略、JSONL/MQTT/metrics、5 个单测 |
| 2026-06-24 | — | 创建本进度文档，定位当前节点 P1.6，下一步 P1.7 |
