# EdgeFlow 代码规范

本文档约定**默认执行环境**与**注释要求**，所有现有及后续源码均须遵循。

---

## 1. 默认执行环境：Linux

| 项 | 约定 |
| --- | --- |
| 开发与运行 | Ubuntu/Debian 等 Linux；Windows 请用 **WSL2** 或 Linux 容器 |
| 部署目标 | RK3568/RK3588 **ARM Linux** |
| API | POSIX.1-2008（CMake 定义 `_POSIX_C_SOURCE=200809L`） |
| 路径 | `/tmp/edgeflow`、`/dev/ttyUSB0`、`/var/log/edgeflow` 等 Linux 路径 |
| 构建 | 非 Linux 配置时 CMake 发出 WARNING |

**禁止在本工程中为 Windows 添加兼容分支**（如 `_WIN32`、` _mkdir`）。若需在 Windows 上开发，使用 WSL。

---

## 2. 注释语言与风格

- **语言**：简体中文
- **术语**：保留 Mutex、RAII、POSIX、SPSC 等英文专业词
- **标识符**：不翻译代码中的函数名、类型名
- **原则**：解释「为什么」和「边界」，少复述代码字面意思

---

## 3. 每个文件必须包含的注释

### 3.1 `.h` 头文件

```c
/*
 * 模块名（学习阶段 Mx）
 *
 * 平台：Linux（若适用）
 * 职责：...
 * 不负责：...
 * 数据流 / 调用时机：...
 */
```

- 每个 `struct`、重要 `enum`：用途 + 谁维护
- 每个公开函数：`@param`、`@return`、调用时机
- 每个字段（非显而易见时）：含义、单位、示例

### 3.2 `.c` 源文件

```c
/*
 * 文件名 — 一句话说明
 *
 * 平台：Linux
 * 实现要点 / 全局状态 / 线程安全说明
 */
```

- `static` 辅助函数：简要说明
- 复杂逻辑块：分段标题 `/* --- 小节名 --- */`
- 标注后续步骤：`/* M6 Scheduler 推进 */`

### 3.3 测试文件 `tests/test_*.c`

- 文件头：覆盖范围、运行方式、工作目录要求
- 每个 `assert` 块前：分段注释说明在验证什么设计约束

### 3.4 `CMakeLists.txt`

- 文件头注释：目标平台、POSIX 定义、目标说明

---

## 4. 模板文件（写新模块前先看）

| 文件 | 说明 |
| --- | --- |
| `src/model/device_model.h` | 类型 + 字段 + API 注释范本 |
| `src/model/device_model.c` | 实现分段注释范本 |
| `src/platform/config.h` | 配置结构体字段分组注释 |
| `src/platform/logger.c` | Linux POSIX API 使用范本 |
| `src/main.c` | 启动流程注释范本 |
| `tests/test_device_model.c` | 测试分段注释范本 |

---

## 5. 新增模块检查清单

提交新代码前自检：

- [ ] 文件头含职责 / 不负责 / 学习阶段
- [ ] 无 Windows 专用代码路径
- [ ] 公开 API 在 `.h` 有完整注释
- [ ] 非显而易见逻辑有分段注释
- [ ] 有对应 `tests/test_*.c` 且带注释
- [ ] `CMakeLists.txt` 已注册目标并 `edgeflow_target_common()`

---

## 6. 相关文档

- [LEARNING_PATH.md](LEARNING_PATH.md) — 学习步骤与注释约定摘要
- [MODULE_DESIGN.md](MODULE_DESIGN.md) — 模块设计模板
- [DEVICE_MODEL.md](DEVICE_MODEL.md) — 统一设备模型理论
