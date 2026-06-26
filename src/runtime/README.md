# runtime — 控制器运行时

**学习阶段**：

| 子模块 | 阶段 |
| --- | --- |
| State And Alarm Engine | M5 |
| Command Scheduler | M6 |
| Strategy（削峰/TOU 等） | M5 起 |
| `gateway_app` 双线程/多线程编排 | M8 |

当前为空。状态机与指令调度在 M5、M6 单独实现并单测，再组装运行时。
