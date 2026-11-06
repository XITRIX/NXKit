#!/usr/bin/env python3
"""Keep real UIKit input and target presentation geometry as a small replay fixture.

Usage: extract-interactions.py OUTPUT TRACE_DIRECTORY [TRACE_DIRECTORY ...]
The raw layer inventory is deliberately excluded. Events retain their measured
dispatch time when available, otherwise the original hardware timestamp.
"""
import argparse
import json
import math
from pathlib import Path

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("output", type=Path)
parser.add_argument("directories", type=Path, nargs="+")
parser.add_argument("--fits", type=Path, required=True, help="analyze-interactions.py output, for measured compositor alignment")
args = parser.parse_args()
fits = json.loads(args.fits.read_text())
traces = {}
for directory in args.directories:
    for path in directory.glob("*/trace.json"):
        trace = json.loads(path.read_text())
        traces[trace["configuration"]["id"]] = trace
args.output.parent.mkdir(parents=True, exist_ok=True)
with args.output.open("w") as output:
    output.write("# UIKit real-touch reference; points and monotonic seconds.\n")
    output.write("# case id width height OS; event t phase dx dy; frame t centerDx centerDy width height\n")
    for name, trace in sorted(traces.items()):
        config = trace["configuration"]
        target = "glass1" if config.get("pair") else "glass0"
        identifier = next(s["id"] for s in trace["schema"] if s["name"] == target)
        origin = next(row for row in trace["frames"][0]["layers"] if row[0] == identifier)
        cx, cy = origin[1] + origin[3] / 2, origin[2] + origin[4] / 2
        width, height = config["width"], config["height"]
        output.write(f"case {name} {width} {height} {trace['os']}\n")
        # Keep acquisition latency out of production spring constants. The
        # fitted offset is retained explicitly so this is a curve/geometry
        # regression, not a claim about end-to-end event-to-display latency.
        fit = fits[name]
        move_latency = .006 if "dispatch" in trace["events"][0] else .022
        output.write("alignment %.8f %.8f %.8f\n" % (
            fit['press']['latency'], move_latency, fit['release']['latency']))
        first = trace["events"][0]
        for event in trace["events"]:
            output.write("event %.8f %d %.8f %.8f\n" % (
                event.get("dispatch", event["t"]), event["phase"], event["x"]-first["x"], event["y"]-first["y"]))
        rejected = 0
        for frame in trace["frames"]:
            row = next((r for r in frame["layers"] if r[0] == identifier), None)
            if row is None or any(v is None or not math.isfinite(v) for v in row[1:5]):
                raise ValueError(f"Missing/invalid target geometry in {name}")
            if abs(row[1]+row[3]/2-cx)>max(width,height)*.5 or abs(row[2]+row[4]/2-cy)>max(width,height)*.5:
                # Earlier fixture versions keyed IDs by tree path. A detached
                # UIKit wrapper occasionally produced one local-space row at
                # release. Do not invent a replacement geometry sample.
                rejected += 1
                continue
            output.write("frame %.8f %.8f %.8f %.8f %.8f\n" % (
                frame["t"], row[1]+row[3]/2-cx, row[2]+row[4]/2-cy, row[3], row[4]))
        output.write("end\n")
        if rejected:
            output.write(f"# {name}: omitted {rejected} invalid coordinate-space samples from older path-based logger\n")
print(f"Extracted {len(traces)} UIKit touch traces to {args.output}")
