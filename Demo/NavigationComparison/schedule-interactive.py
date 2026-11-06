"""Write a progress-matched InteractiveReplay CSV from native capture folders.

Usage: schedule-interactive.py NATIVE_DIRECTORY OUTPUT.csv
Input: NATIVE_DIRECTORY/CASE/trace.json, as exported by InteractiveReference.
"""
import csv
import json
import sys
from pathlib import Path

native, output = map(Path, sys.argv[1:3])
captures = sorted(native.glob("*/trace.json"))
if not captures:
    raise SystemExit("No CASE/trace.json captures found")
with output.open("w", newline="") as stream:
    writer = csv.writer(stream, lineterminator="\n")
    for path in captures:
        capture = json.loads(path.read_text())
        name = capture["name"]
        if "-dark-" not in name and "-light-" not in name:
            raise ValueError("Case name must identify its theme: " + name)
        width = next(f["sourceContent"][2] for f in capture["frames"]
                     if f.get("sourceContent") and f["sourceContent"][2] > 0)
        released = False
        for index, frame in enumerate(capture["frames"]):
            active = frame["state"] in (1, 2) and not released
            released |= not active
            content = frame.get("sourceContent")
            # Preserve native spring overshoot in the evidence; the portable
            # recognizer bounds tracking progress itself.
            progress = content[0] / width if content else int(name.endswith("finish"))
            writer.writerow((name, capture["source"], capture["target"],
                             int("-dark-" in name), index, frame["t"], progress,
                             int(active), int("image" in frame)))
