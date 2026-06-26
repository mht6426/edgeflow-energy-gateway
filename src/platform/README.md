# platform — 平台层

**平台**：Linux（配置路径、日志目录、后续 metrics/storage 均为 POSIX 文件 API）

**学习阶段**：

| 子模块 | 阶段 |
| --- | --- |
| Config Manager | M2 |
| Logger | M2 |
| Metrics | M10 |
| Storage (JSONL → SQLite) | M7 |
| Watchdog / CLI | M10 |

当前为空。下一步在 M2 实现 `config` 与 `logger`。

**实现时请遵循** [CODE_STYLE.md](../../docs/CODE_STYLE.md)：Linux 默认环境、每个 `.c/.h` 详细中文注释。
