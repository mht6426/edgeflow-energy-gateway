#!/usr/bin/env python3
"""
Minimal placeholder for Modbus slave testing.

真实联调时建议安装 pymodbus 后扩展本脚本，提供电表、温度和电流寄存器。
当前 C 程序默认使用内置模拟点位，保证没有 RS485 外设时也能验证主流程。
"""

import time


def main() -> None:
    print("mock_modbus placeholder: extend with pymodbus for RS485/TCP tests")
    while True:
        time.sleep(1)


if __name__ == "__main__":
    main()
