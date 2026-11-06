#!/usr/bin/env python3
"""Summarize comparable GlassStressPerformance JSONL runs without dependencies."""
import json
import statistics
import sys
from pathlib import Path


def summarize(path):
    rows = [json.loads(line) for line in Path(path).read_text().splitlines() if line.strip()]
    if not rows:
        raise ValueError(f"Empty report: {path}")
    metadata = {key: rows[0][key] for key in ("backend", "density", "dark")}
    warm = [row for row in rows if row["phase"] != "cold"]
    if not warm:
        raise ValueError(f"No warm frames: {path}")
    timing_keys = ("frameMs", "layoutMs", "renderMs", "glassMs", "sortMs", "submitMs")
    result = {
        "metadata": metadata,
        "frames": len(rows),
        "warmFrames": len(warm),
        "phaseSequence": [row["phase"] for row in rows],
        "median": {key: statistics.median(row[key] for row in warm) for key in timing_keys},
        "p95FrameMs": sorted(row["frameMs"] for row in warm)[max(0, (95 * len(warm) + 99) // 100 - 1)],
        "medianCounts": {key: statistics.median(row[key] for row in warm)
                         for key in ("views", "layers", "rejected", "sorts", "glass")},
    }
    return result


def main():
    if len(sys.argv) != 3:
        raise ValueError("Usage: summarize_glass_stress.py BEFORE.jsonl AFTER.jsonl")
    before, after = map(summarize, sys.argv[1:])
    if before["metadata"] != after["metadata"] or before["phaseSequence"] != after["phaseSequence"]:
        raise ValueError("Compare the same backend, density, theme and phase sequence")
    for result in (before, after):
        del result["phaseSequence"]
    comparison = {"before": before, "after": after,
                  "medianFrameReductionPercent": 100 * (1 - after["median"]["frameMs"] / before["median"]["frameMs"])}
    print(json.dumps(comparison, indent=2))


if __name__ == "__main__":
    try:
        main()
    except (ValueError, OSError, KeyError) as error:
        sys.exit(str(error))
