# 硬件选型

## 结论

首选 RK3568/RK3568J 工控板或工控机，不优先购买 RK3588。

EdgeFlow Industrial Controller 更看重工业接口、长期稳定运行、串口/CAN/双网口和现场部署，而不是 AI 算力或视频编解码能力。

## 为什么首选 RK3568/RK3568J

- 国产 Rockchip SoC，符合国产 ARM Linux 与自主可控叙事。
- 四核 Cortex-A55 足够运行 EMS 边缘控制器、SQLite、MQTT 和 metrics。
- 相比 RK3588 更贴近工商业储能边缘控制器的成本、功耗和产品形态。
- 常见工控板支持双千兆网口、RS485、CAN、4G/5G 扩展。
- Debian/Ubuntu/Buildroot 资料较多，适合三个月内完成板端闭环。

## 推荐配置

- CPU：RK3568J 优先，RK3568 可接受。
- 内存：4GB 推荐，2GB 可用。
- 存储：32GB eMMC 推荐，16GB 起步。
- 网络：双千兆网口优先，便于模拟站内设备网和上行网络。
- 串口：至少 1 路 RS485，最好 2 路。
- CAN：至少 1 路，便于后续 BMS CAN stub。
- 无线：4G/5G miniPCIe 或 M.2 扩展可选。
- 电源：12V 工业电源输入优先。
- 系统：前三个月使用板厂 Debian/Ubuntu ARM64。

## 推荐开发板类型

优先级：

1. RK3568J 工控机形态：更接近真实 EMS 控制器，适合演示和板端验证。
2. RK3568 工控板：成本较低，适合开发阶段。
3. 普通 RK3568 SBC + USB-RS485：可作为预算受限方案。

## 外设清单

- USB-RS485 转换器或板载 RS485。
- Modbus RTU 电表，或 PC 端 `pymodbus` 模拟 BMS/PCS/Meter。
- CAN 转 USB 模块，后续模拟 BMS CAN。
- 本地 Mosquitto / EMQX broker。
- 12V 电源、网线、串口调试线。

## RK3588 何时使用

只有在明确扩展以下方向时才考虑 RK3588：

- 多路视频分析。
- AI 边缘推理。
- NVR 或视觉盒子。
- 复杂多容器边缘计算节点。

EdgeFlow Industrial Controller 首版不需要 RK3588。选择 RK3588 反而会被追问成本、功耗和选型合理性。

## 不建议

- 不建议首版使用纯 MCU/RTOS 板卡，这会偏离 Linux C 和 ARM Linux 设备软件主线。
- 不建议首版直接做完整 Buildroot/Yocto BSP，容易卡在系统构建和驱动适配。
- 不建议为了追求性能购买高端板，把预算优先留给 RS485、电表、CAN 和电源等真实外设。
