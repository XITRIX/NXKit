#!/usr/bin/env python3
"""Replace scoped, rerendered clips in a complete cycle matrix."""
import argparse
import json
from pathlib import Path
import shutil

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("matrix", type=Path)
parser.add_argument("patches", type=Path, nargs="+")
args = parser.parse_args()
report = json.loads((args.matrix / "cycle.json").read_text())
clips = {c["name"]: c for c in report["clips"]}
updated = set()
for root in args.patches:
    for clip in json.loads((root / "cycle.json").read_text())["clips"]:
        name = clip["name"]
        assert name in clips and name not in updated
        assert len(clip["frames"]) == 91
        for frame in clip["frames"]:
            if "image" in frame:
                assert (root / frame["image"]).is_file()
        shutil.copytree(root / name, args.matrix / name, dirs_exist_ok=True)
        clips[name] = clip
        updated.add(name)
report["clips"] = [clips[c["name"]] for c in report["clips"]]
assert len(report["clips"]) == 288
(args.matrix / "cycle.json").write_text(json.dumps(report))
print(f"Updated {len(updated)} clips; matrix still contains all 288 cases")
