#!/usr/bin/env python3
"""Check real C/C++ TLS code generation on first configure and reconfigure.

Run with DEVKITPRO set: python3 Demo/tests/SwitchToolchainTests.py
Requires devkitPro and Demo/libromfs-generator, but no Skia archives or device.
"""

import json
import os
from pathlib import Path
import shlex
import subprocess
import tempfile


def run(*args):
    result = subprocess.run(args, text=True, capture_output=True)
    if result.returncode:
        raise RuntimeError(f"{shlex.join(args)}\n{result.stdout}\n{result.stderr}")
    return result.stdout


def main():
    repo = Path(__file__).resolve().parents[2]
    devkit = Path(os.environ["DEVKITPRO"])
    with tempfile.TemporaryDirectory(prefix="nxkit-switch-tls-") as directory:
        source = Path(directory)
        build = source / "build"
        (source / "CMakeLists.txt").write_text(f"""
cmake_minimum_required(VERSION 3.29)
set(UIKIT_PATH "{repo.as_posix()}")
set(LIBROMFS_PREBUILT_GENERATOR "{repo.as_posix()}/Demo/libromfs-generator")
include("${{UIKIT_PATH}}/Submodules/cmake/toolchain.cmake")
project(SwitchTLSProbe LANGUAGES C CXX)
add_library(probe STATIC probe_c.c probe_cpp.cpp)
""")
        # Both languages must use libnx's software thread pointer. Keep the
        # storage externally visible so optimization cannot remove the access.
        (source / "probe_c.c").write_text(
            "_Thread_local int c_value; int c_tls(void) { return ++c_value; }\n")
        (source / "probe_cpp.cpp").write_text(
            "thread_local int cpp_value; int cpp_tls() { return ++cpp_value; }\n")
        for stage in ("initial configure", "reconfigure"):
            run("cmake", "-S", str(source), "-B", str(build),
                "-DPLATFORM_SWITCH=ON", "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON")
            entries = json.loads((build / "compile_commands.json").read_text())
            assert len(entries) == 2, entries
            for entry in entries:
                args = entry.get("arguments") or shlex.split(entry["command"])
                assert "-mtp=soft" in args, (stage, entry)
                assert "-ftls-model=local-exec" in args, (stage, entry)
                assert str(devkit / "portlibs/switch/include") in args, (stage, entry)
                # Generate assembly using exactly the configured compilation
                # flags, independent of the CMake generator's object paths.
                output = args.index("-o")
                del args[output:output + 2]
                args.remove("-c")
                assembly = source / (Path(entry["file"]).name + ".s")
                run(*args, "-S", "-o", str(assembly))
                text = assembly.read_text().lower()
                assert "__aarch64_read_tp" in text, (stage, text)
                assert "tpidr_el0" not in text, (stage, text)
            run("cmake", "--build", str(build), "--clean-first")
            print(f"PASS: C and C++ use libnx TLS after {stage}")
        assert (devkit / "libnx/include/switch.h").exists()


if __name__ == "__main__":
    main()
