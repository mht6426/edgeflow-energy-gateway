# 第三方依赖（vendored）

本目录包含 EdgeFlow 静态链接的第三方 C 库，便于 Linux 开发与 aarch64 交叉编译，无需依赖系统包管理器版本。

| 库 | 版本 | 用途 |
|---|---|---|
| [cJSON](https://github.com/DaveGamble/cJSON) | v1.7.18 | JSON 配置解析 |
| [libmodbus](https://github.com/stephane/libmodbus) | v3.1.10 | Modbus RTU / TCP 设备接入 |

更新方式（在仓库根目录）：

```bash
cd third_party/cJSON && git fetch --tags && git checkout v1.7.18
cd ../libmodbus && git fetch --tags && git checkout v3.1.10
```

构建由根目录 `CMakeLists.txt` 通过 `add_subdirectory(third_party)` 集成。
