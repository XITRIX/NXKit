#!/usr/bin/env python3
"""Merge completed NavigationCycleRenderer shards without losing PNG evidence."""
import argparse
import json
import shutil
from pathlib import Path

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("output", type=Path)
parser.add_argument("shards", type=Path, nargs="+")
args = parser.parse_args()
args.output.mkdir(parents=True, exist_ok=True)
clips = []
for root in args.shards:
    report = json.loads((root / "cycle.json").read_text())
    for clip in report["clips"]:
        assert len(clip["frames"]) == 91
        images = [f["image"] for f in clip["frames"] if "image" in f]
        assert len(images) in (10, 91) and all((root / p).is_file() for p in images)
        shutil.copytree(root / clip["name"], args.output / clip["name"], dirs_exist_ok=True)
        clips.append(clip)
clips.sort(key=lambda c: (c["dark"], c["pop"], c["source"], c["target"]))
assert len(clips) == 288 and len({c["name"] for c in clips}) == 288
manifest = json.loads((Path(__file__).parent / "CycleCases.json").read_text())
assert {c["name"] for c in clips} == {c["name"] for c in manifest}
(args.output / "cycle.json").write_text(json.dumps({"scale": 3, "imageBarOriginY": 12, "clips": clips}))
pictures = sum(sum("image" in f for f in c["frames"]) for c in clips)
print(f"Merged {len(clips)} clips, {len(clips) * 91} samples and {pictures} PNGs")
