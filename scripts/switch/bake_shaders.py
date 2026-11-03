#!/usr/bin/env python3
"""Compile Demo pipelines on the target GPU and bundle them into its NRO.

Credentials are read from NXKIT_SWITCH_FTP_USER/PASSWORD, never saved to CMake
or logs. The Switch must be in its nxlink-capable launcher with FTP enabled.
"""
import argparse
import errno
from ftplib import FTP
import hashlib
import json
import os
from pathlib import Path
import pty
import select
import shutil
import struct
import subprocess
import time


def run_device(command, log):
    # nxlink buffers stdout when redirected to a file. A terminal keeps its
    # diagnostics visible even if the GPU hangs before normal process exit.
    master, slave = pty.openpty()
    try:
        with subprocess.Popen(command, stdout=slave, stderr=subprocess.STDOUT) as process:
            os.close(slave)
            slave = None
            deadline = time.monotonic() + 1200
            with log.open("wb") as stream:
                while True:
                    remaining = deadline - time.monotonic()
                    if remaining <= 0:
                        process.kill()
                        raise subprocess.TimeoutExpired(command, 1200)
                    if not select.select([master], [], [], min(1, remaining))[0]:
                        continue
                    try:
                        chunk = os.read(master, 65536)
                    except OSError as error:
                        if error.errno != errno.EIO:
                            raise
                        break
                    if not chunk:
                        break
                    stream.write(chunk)
                    stream.flush()
            if process.wait(timeout=5):
                raise subprocess.CalledProcessError(process.returncode, command)
    finally:
        os.close(master)
        if slave is not None:
            os.close(slave)


def validate_cache(data, abi, magic):
    if len(data) < 88 or len(data) > 32 * 1024 * 1024 + 88:
        raise ValueError("Invalid shader cache size")
    if data[:8] != magic or data[8:72] != abi.encode("ascii"):
        raise ValueError("Shader cache ABI mismatch; no stale cache will be bundled")
    length, expected_hash = struct.unpack_from("<QQ", data, 72)
    value = 14695981039346656037
    for byte in data[88:]:
        value = ((value ^ byte) * 1099511628211) & ((1 << 64) - 1)
    if length != len(data) - 88 or not length or value != expected_hash:
        raise ValueError("Shader cache truncated or corrupt")
    if magic == b"NXVKP001" and length <= 32:
        raise ValueError("Driver returned only a Vulkan cache header, with no compiled pipeline data")


def validate_run(log, require_bundled=False):
    text = log.read_text(errors="replace")
    if "NXKIT_SHADER_BAKE_COMPLETE" not in text:
        raise RuntimeError(f"Shader compilation did not complete; inspect {log}")
    if "Switch Vulkan presented frame " not in text:
        raise RuntimeError(f"No successful presentation was recorded; inspect {log}")
    if "Shader compilation error" in text or "Failed to create GraphicsPipeline" in text:
        raise RuntimeError(f"Shader compilation failed; inspect {log}")
    if "could not be saved" in text:
        raise RuntimeError(f"Shader cache persistence failed; inspect {log}")
    if require_bundled and ("Switch Vulkan bundled pipeline cache: " not in text or
                            "Switch Vulkan bundled pipeline cache: 0 bytes" in text):
        raise RuntimeError(f"Bundled pipeline cache was not loaded; inspect {log}")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--build-dir", type=Path, required=True)
    parser.add_argument("--address", required=True)
    parser.add_argument("--ftp-port", type=int, default=5000)
    parser.add_argument("--nxlink", default="nxlink")
    parser.add_argument("--target", default="ThorVGApp")
    args = parser.parse_args()
    if not args.address:
        parser.error("Set NXKIT_SWITCH_ADDRESS to the Switch IP when configuring CMake")
    user, password = os.getenv("NXKIT_SWITCH_FTP_USER"), os.getenv("NXKIT_SWITCH_FTP_PASSWORD")
    if not user or not password:
        parser.error("Set NXKIT_SWITCH_FTP_USER and NXKIT_SWITCH_FTP_PASSWORD in the environment")
    build = args.build_dir.resolve()
    abi = (build / "switch-shader-abi.txt").read_text().strip()
    if len(abi) != 64 or any(c not in "0123456789abcdef" for c in abi):
        raise ValueError("Invalid build shader ABI")
    nro = build / (args.target + ".nro")
    output = Path((build / "switch-shader-cache-dir.txt").read_text().strip())
    output.mkdir(parents=True, exist_ok=True)
    log = output / "bake.log"
    command = [args.nxlink, "-a", args.address, "-r", "2", "-s",
               "--args=--nxkit-shader-bake", str(nro)]
    print("Running shader compilation fixture on Switch; log:", log, flush=True)
    run_device(command, log)
    validate_run(log)

    connection = None
    for attempt in range(30):
        try:
            connection = FTP()
            connection.connect(args.address, args.ftp_port, timeout=10)
            connection.login(user, password)
            break
        except OSError:
            if connection: connection.close()
            connection = None
            time.sleep(1)
    if connection is None:
        raise RuntimeError("Switch did not return to its FTP launcher after shader compilation")
    try:
        for name, magic in (("pipeline.cache", b"NXVKP001"), ("pipeline.keys", b"NXVKK001")):
            data = bytearray()
            def collect(chunk):
                if len(data) + len(chunk) > 32 * 1024 * 1024 + 88:
                    raise ValueError("Remote cache exceeds size limit")
                data.extend(chunk)
            connection.retrbinary(f"RETR /switch/NXKit/shaders/{abi}/{name}", collect)
            validate_cache(data, abi, magic)
            (output / name).write_bytes(data)
            print(f"Compiled {name}: {len(data):,} bytes", flush=True)
    finally:
        connection.close()
    # Changes are content-addressed and excluded from the ABI itself, so the
    # second build embeds this seed without invalidating it or recursing here.
    subprocess.run(["cmake", "--build", str(build), "--target", args.target + ".nro", "-j8"], check=True)
    current_abi = (build / "switch-shader-abi.txt").read_text().strip()
    if current_abi != abi:
        raise RuntimeError("Sources changed during shader compilation; rerun the bake target")
    validation_log = output / "bundled-validation.log"
    validated_hash = hashlib.sha256(nro.read_bytes()).hexdigest()
    # SD data stays untouched. The NRO explicitly ignores it for both native
    # cache loading and Graphite key replay during this fresh-install pass.
    print("Validating bundled shaders on Switch:", validation_log, flush=True)
    command = [args.nxlink, "-a", args.address, "-r", "2", "-s",
               "--args=--nxkit-shader-bake --nxkit-bundled-cache-only", str(nro)]
    run_device(command, validation_log)
    validate_run(validation_log, require_bundled=True)
    if (build / "switch-shader-abi.txt").read_text().strip() != abi or \
            hashlib.sha256(nro.read_bytes()).hexdigest() != validated_hash:
        raise RuntimeError("Build changed during bundled validation; rerun the bake target")
    package = build / "switch-package"
    package.mkdir(exist_ok=True)
    shutil.copy2(nro, package / nro.name)
    license_root = Path(__file__).resolve().parents[2] / "licenses"
    for name in ("NVK-Switch-NOTICE.md", "Skia-Vulkan-Allocator-LICENSE"):
        shutil.copy2(license_root / name, package / name)
    manifest = {"shader_abi": abi, "nro_sha256": validated_hash,
                "bundled_cache_validation": True,
                "pipeline_cache_bytes": (output / "pipeline.cache").stat().st_size,
                "pipeline_keys_bytes": (output / "pipeline.keys").stat().st_size,
                "coverage": "Demo gallery, grouped glass, lens states, stress, tabs, text and XML"}
    (package / "shader-manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
    print("Switch package with bundled compiled pipelines:", package, flush=True)


if __name__ == "__main__":
    main()
