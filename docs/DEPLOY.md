# 部署说明

## PC 本地验证

```bash
cmake -S . -B build
cmake --build build
./build/edgeflow -c configs/gateway.json
```

默认配置使用模拟 Modbus 数据。运行后检查：

```bash
ls -l /tmp/edgeflow
cat /tmp/edgeflow/metrics.prom
```

## ARM64 交叉编译

安装工具链：

```bash
sudo apt install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu cmake
```

构建：

```bash
cmake -S . -B build-aarch64 -DCMAKE_TOOLCHAIN_FILE=toolchain/aarch64-linux-gnu.cmake
cmake --build build-aarch64
```

## 板端安装

将产物复制到开发板：

```bash
scp build-aarch64/edgeflow root@board:/tmp/
scp configs/gateway.json root@board:/tmp/
```

执行安装脚本：

```bash
sudo deploy/scripts/install.sh build-aarch64/edgeflow configs/gateway.json
```

启用服务：

```bash
sudo systemctl daemon-reload
sudo systemctl enable --now edgeflow
systemctl status edgeflow
```

## systemd 验证

- 重启开发板后服务自动启动。
- `journalctl -u edgeflow -f` 可查看日志。
- `/run/edgeflow/metrics.prom` 有指标输出。
- `kill -STOP $(pidof edgeflow)` 后 systemd watchdog 应触发恢复。

## RS485 验证

修改 `configs/gateway.json`：

```json
{
  "simulate": false,
  "serial_device": "/dev/ttyUSB0"
}
```

确认串口权限：

```bash
ls -l /dev/ttyUSB0
sudo usermod -aG dialout edgeflow
```

## 故障排查

- 无串口权限：检查 `dialout` 分组或使用 udev 规则。
- MQTT 不通：先在板端 `ping broker`，再检查 `broker_host` 和 `broker_port`。
- metrics 不更新：确认 `/run/edgeflow` 是否存在且服务用户有写权限。
- 缓存增长过快：检查 MQTT broker 是否不可达，查看丢点和缓存指标。
