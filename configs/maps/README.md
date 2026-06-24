# 点表映射（规划中）

`slave1.json` 为 Modbus 寄存器到统一 Point 的映射示例，**当前运行时未加载此文件**。

运行时模拟器直接输出以下点位（见 `src/ingress/modbus_rtu.c`）：

| point_id | device_id | 说明 |
| --- | --- | --- |
| `soc_percent` | bms_001 | 电池 SOC |
| `max_cell_temp_c` | bms_001 | 最高单体温度 |
| `grid_power_kw` | meter_001 | 电网侧功率 |

后续 Adapter 插件化重构时将接入点表加载逻辑。
