# 24h 稳定性测试记录模板

## 测试环境

- 开发板：
- OS：
- 编译方式：
- 配置文件：
- MQTT broker：
- BMS 模拟器：
- PCS 模拟器：
- Meter 模拟器：
- RS485/CAN 设备：

## 测试方法

```bash
sudo systemctl restart edgeflow
date
watch -n 5 'cat /run/edgeflow/metrics.prom'
```

建议同时记录：

```bash
pidstat -p $(pidof edgeflow) 5
ps -o pid,pcpu,pmem,rss,cmd -p $(pidof edgeflow)
journalctl -u edgeflow --since "24 hours ago"
```

## 目标指标

- 运行时长：大于等于 24h。
- BMS/PCS/Meter 采集成功率：大于等于 99.9%。
- 控制命令失败率：小于 0.1%。
- 未恢复关键告警：0。
- 策略线程 heartbeat 正常。
- CPU：模拟站端场景低于 15% 单核。
- RSS：低于 64MB。
- MQTT：broker 可达时成功发布持续增长。
- 断网恢复：缓存未满前提下可补传。
- systemd 重启次数：0。

## 测试记录

| 项目 | 结果 |
| --- | --- |
| 开始时间 | 待填写 |
| 结束时间 | 待填写 |
| 运行时长 | 待填写 |
| BMS 采集成功率 | 待填写 |
| PCS 采集成功率 | 待填写 |
| Meter 采集成功率 | 待填写 |
| SystemState 切换次数 | 待填写 |
| StrategyDecision 总数 | 待填写 |
| ControlCommand 成功数 | 待填写 |
| ControlCommand 失败数 | 待填写 |
| AlarmEvent 总数 | 待填写 |
| 未恢复告警数 | 待填写 |
| mqtt publish ok | 待填写 |
| mqtt publish fail | 待填写 |
| SQLite 缓存积压最大值 | 待填写 |
| queue dropped | 待填写 |
| CPU 平均值 | 待填写 |
| RSS 最大值 | 待填写 |
| 异常重启次数 | 待填写 |

## 结论

待实测后填写。没有真实 24h 数据前，简历只写“已设计稳定性测试方案”，不要写“通过 24h 稳定性测试”。
