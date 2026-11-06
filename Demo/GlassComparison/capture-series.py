"""Capture native settled resize checkpoints on an already booted 2x iPad.

Usage: capture-series.py DEVICE_ID app-directory prepared-directory output-directory
Run after build-native.sh and build-raster.sh. Requires Pillow for analysis only.
The Simulator fixture's own Documents/request.json controls its retained views.
"""
import json
from pathlib import Path
import subprocess
import sys
import time

device, app_path, prepared_path, output_path = sys.argv[1:5]
options = sys.argv[5:]
output = Path(output_path).resolve()
output.mkdir(parents=True, exist_ok=True)
prepared = Path(prepared_path).resolve()
bundle = "org.nxkit.NativeGlassReference"

def simctl(*args):
    return subprocess.run(["xcrun", "simctl", *args], check=True, capture_output=True, text=True).stdout.strip()

simctl("install", device, str(Path(app_path).resolve()))
container = Path(simctl("get_app_container", device, bundle, "data")) / "Documents"

def read_metadata():
    try:
        return json.loads((container / "capture.json").read_text())
    except (FileNotFoundError, json.JSONDecodeError):
        return {}

records = json.loads((output / "series.json").read_text()) if options and (output / "series.json").exists() else []
for appearance in (["light"] if "--light-only" in options else ["light", "dark"]):
    flags = ["--dark"] if appearance == "dark" else []
    simctl("launch", "--terminate-running-process", device, bundle, *flags)
    deadline = time.monotonic() + 20
    while True:
        m = read_metadata()
        if m.get("appearance") == appearance and m.get("id") == "initial" and m.get("effectiveAppearance") == appearance:
            break
        if time.monotonic() > deadline:
            raise RuntimeError("Native fixture did not initialize")
        time.sleep(0.1)
    time.sleep(1)
    initial = output / f"{appearance}-initial"
    initial.mkdir(exist_ok=True)
    simctl("io", device, "screenshot", str(initial / "screen.png"))
    metadata = read_metadata()
    metadata["arguments"] = metadata["arguments"][1:]
    (initial / "capture.json").write_text(json.dumps(metadata, indent=2) + "\n")
    for tint in (["none"] if "--neutral-only" in options else ["none", "blue", "coral"]):
        # The decreasing samples detect history dependence after growing.
        for index, size in enumerate([0, .25, .5, .75, 1, .75, .5, .25, 0]):
            identifier = f"{appearance}-{tint}-{index}"
            request = {"id": identifier, "size": size, "tint": tint}
            staged = container / "request.tmp"
            staged.write_text(json.dumps(request))
            staged.replace(container / "request.json")
            deadline = time.monotonic() + 10
            while True:
                metadata = read_metadata()
                if metadata.get("id") == identifier:
                    break
                if time.monotonic() > deadline:
                    raise RuntimeError(f"Native request timed out: {identifier}")
                time.sleep(0.05)
            frame = output / identifier
            frame.mkdir(exist_ok=True)
            simctl("io", device, "screenshot", str(frame / "screen.png"))
            metadata["arguments"] = metadata["arguments"][1:]
            (frame / "capture.json").write_text(json.dumps(metadata, indent=2) + "\n")
            # Raster results depend only on target geometry; reuse the result
            # for the matching contraction state rather than rerendering it.
            raster = output / f"{appearance}-{tint}-{size:.2f}.ppm"
            if not raster.exists():
                raster_flags = flags + [f"--size={size}"]
                if tint != "none":
                    raster_flags += [f"--tint={tint}"]
                subprocess.run([str(prepared / "GlassComparisonRenderer"), str(prepared / "scene.rgb"),
                    str(raster), *raster_flags], check=True)
            records = [r for r in records if r["id"] != identifier]
            records.append({**request, "appearance": appearance, "frame": str(frame), "raster": str(raster)})
            (output / "series.json").write_text(json.dumps(records, indent=2) + "\n")
            print(identifier, "size", size, flush=True)
