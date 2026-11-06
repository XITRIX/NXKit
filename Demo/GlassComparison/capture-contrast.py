"""Isolate backdrop contrast from size and view traits on native UIKit.
Usage: capture-contrast.py DEVICE_ID native-app-directory output-directory
Four fresh view configurations: small/large regular card x light/dark traits.
Within each, only the backdrop changes; labels use dynamic UIColor.labelColor.
"""
import json
from pathlib import Path
import subprocess
import sys
import time

device, app, output_path = sys.argv[1:]
output = Path(output_path).resolve()
output.mkdir(parents=True, exist_ok=True)
bundle = "org.nxkit.NativeGlassReference"

def sim(*args):
    return subprocess.run(["xcrun", "simctl", *args], check=True, capture_output=True, text=True).stdout.strip()

sim("install", device, str(Path(app).resolve()))
documents = Path(sim("get_app_container", device, bundle, "data")) / "Documents"

def metadata():
    try:
        return json.loads((documents / "capture.json").read_text())
    except (FileNotFoundError, json.JSONDecodeError):
        return {}

def wait_for(predicate):
    deadline = time.monotonic() + 20
    while True:
        m = metadata()
        if predicate(m):
            return m
        if time.monotonic() > deadline:
            raise RuntimeError("Native contrast fixture did not acknowledge request")
        time.sleep(.05)

records = []
for appearance in ["light", "dark"]:
    for size, size_name in [(0, "small"), (1, "large")]:
        flags = ["--labels", "--black", f"--size={size}"] + (["--dark"] if appearance == "dark" else [])
        sim("launch", "--terminate-running-process", device, bundle, *flags)
        wait_for(lambda m: m.get("id") == "initial" and m.get("labels")
            and m.get("appearance") == appearance and m.get("size") == size)
        for index, background in enumerate(["black", "white", "gray", "checker", "black"]):
            identifier = f"{appearance}-{size_name}-{background}-{index}"
            request = {"id": identifier, "size": size, "tint": "none", "background": background, "animate": False}
            (documents / "request.tmp").write_text(json.dumps(request))
            (documents / "request.tmp").replace(documents / "request.json")
            m = wait_for(lambda m: m.get("id") == identifier)
            frame = output / identifier
            frame.mkdir(exist_ok=True)
            sim("io", device, "screenshot", str(frame / "screen.png"))
            m["arguments"] = m["arguments"][1:]
            (frame / "capture.json").write_text(json.dumps(m, indent=2) + "\n")
            records.append({"id": identifier, "frame": str(frame), "metadata": m})
            (output / "contrast.json").write_text(json.dumps(records, indent=2) + "\n")
            print(m["os"], identifier, "content appearances", m["contentAppearances"], flush=True)
