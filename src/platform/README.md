# platform — 平台层

**平台**：Linux（POSIX 文件 API、systemd notify）

| 子模块 | 说明 |
| --- | --- |
| `config.*` | JSON 配置（cJSON） |
| `logger.*` | 文件日志 |
| `storage.*` | SQLite WAL + JSONL |
| `metrics.*` | Prometheus 文本格式 |
| `watchdog.*` / `heartbeat.*` | systemd 与线程健康 |
| `cli/main.c` | 运维 CLI |

实现时请遵循 [开发指南.md](../../docs/开发指南.md)。
