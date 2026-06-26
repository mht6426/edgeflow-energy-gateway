#!/usr/bin/env python3
"""根据 CMakeLists.txt 中的源文件生成 compile_commands.json（供 clangd 索引）。"""

from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

# 与 CMakeLists.txt 中 add_executable 源文件保持一致
SOURCES = [
    "src/main.c",
    "src/common/time_util.c",
    "src/core/ring.c",
    "src/model/device_model.c",
    "src/ingress/modbus_rtu.c",
    "src/egress/mqtt_client.c",
    "src/platform/config.c",
    "src/platform/logger.c",
    "src/platform/metrics.c",
    "src/platform/storage.c",
    "src/runtime/control_loop.c",
    "src/runtime/app.c",
    "tests/test_ring.c",
    "tests/test_device_model.c",
    "tests/test_control_loop.c",
    "tests/test_modbus_crc.c",
    "tests/test_config.c",
]

FLAGS = ["-std=c11", "-D_POSIX_C_SOURCE=200809L", "-Isrc"]


def main() -> None:
    entries = []
    for rel in SOURCES:
        src = ROOT / rel
        if not src.is_file():
            continue
        cmd = "clang " + " ".join(FLAGS) + f" -c {rel.replace(chr(92), '/')}"
        entries.append(
            {
                "directory": str(ROOT).replace("\\", "/"),
                "command": cmd,
                "file": str(src).replace("\\", "/"),
            }
        )
    out = ROOT / "compile_commands.json"
    out.write_text(json.dumps(entries, indent=2) + "\n", encoding="utf-8")
    print(f"Wrote {len(entries)} entries to {out}")


if __name__ == "__main__":
    main()
