#!/usr/bin/env python3
"""Generate seed data for this application/Skia/driver/shader ABI only."""
import argparse
from pathlib import Path


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--directory", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    lines = ["#include <cstddef>", "namespace NXKit::Internal {"]
    for filename, symbol in (("pipeline.cache", "switchBakedPipelineCache"),
                             ("pipeline.keys", "switchBakedPipelineKeys")):
        path = args.directory / filename
        data = path.read_bytes() if path.exists() else b""
        if len(data) > 32 * 1024 * 1024:
            raise ValueError(f"Cache exceeds 32 MiB: {path}")
        values = [f"0x{value:02x}" for value in data] or ["0"]
        lines.append(f"extern const unsigned char {symbol}[] = {{")
        lines.extend(",".join(values[i:i + 32]) + "," for i in range(0, len(values), 32))
        lines += ["};", f"extern const std::size_t {symbol}Size = {len(data)};"]
    lines.append("}")
    text = "\n".join(lines) + "\n"
    args.output.parent.mkdir(parents=True, exist_ok=True)
    if not args.output.exists() or args.output.read_text() != text:
        args.output.write_text(text)


if __name__ == "__main__":
    main()
