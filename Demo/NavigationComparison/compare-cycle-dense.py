#!/usr/bin/env python3
"""Render timestamped, half-speed native/portable movies of the 32-clip cycle.

Each panel uses the nearest actually captured snapshot on a shared request-
relative clock. No pixel interpolation, per-clip onset fitting or time shifts
are applied; individual source timestamps show the remaining sampling mismatch.
"""
import argparse
from functools import lru_cache
import gzip
import hashlib
import importlib.util
import json
from pathlib import Path
import shutil
import subprocess

from PIL import Image, ImageDraw

spec = importlib.util.spec_from_file_location("cycle_comparison", Path(__file__).with_name("compare-cycle.py"))
comparison = importlib.util.module_from_spec(spec)
spec.loader.exec_module(comparison)


@lru_cache(maxsize=128)
def thumbnail(path):
    with Image.open(path) as image:
        return image.convert("RGB").resize((402, 80), Image.Resampling.LANCZOS)


def at(clip, time):
    frames = [f for f in clip["frames"] if "image" in f]
    return min(frames, key=lambda frame: abs(frame["t"] - time))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("native", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("shards", type=Path, nargs="+")
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    native = json.loads((args.native / "cycle.json").read_text())
    n = {c["name"]: c for c in native["clips"]}
    p, roots = {}, {}
    for root in args.shards:
        for clip in json.loads((root / "cycle.json").read_text())["clips"]:
            if clip["target"] != clip["source"] + (-1 if clip["pop"] else 1):
                continue
            assert len(clip["frames"]) == 91 and all("image" in f for f in clip["frames"])
            assert clip["name"] not in p
            p[clip["name"]], roots[clip["name"]] = clip, root
    assert len(n) == 32 and n.keys() == p.keys()
    assert all(all("image" in f for f in c["frames"]) for c in n.values())
    results = {name: comparison.score(n[name], p[name]) for name in n}
    report = {"nativeFrames": sum(len(c["frames"]) for c in n.values()),
              "portableFrames": sum(len(c["frames"]) for c in p.values()),
              "maxNativeGap": max(b["t"] - a["t"] for c in n.values() for a, b in zip(c["frames"], c["frames"][1:])),
              "summary": comparison.aggregate(list(results.values())), "clips": results,
              "method": __doc__}
    (args.output / "dense-comparison.json").write_text(json.dumps(report, indent=2) + "\n")
    for stage, data in (("native", native), ("after", {"clips": list(p.values())})):
        with gzip.GzipFile(args.output / f"dense-{stage}-trace.json.gz", "wb", mtime=0) as f:
            f.write(json.dumps(data, separators=(",", ":")).encode())
    hashes = {}
    for name, clip in n.items():
        hashes[name] = {}
        for label, current, root in (("native", clip, args.native), ("after", p[name], roots[name])):
            digest = hashlib.sha256()
            for frame in current["frames"]:
                digest.update(hashlib.sha256((root / frame["image"]).read_bytes()).digest())
            hashes[name][label] = digest.hexdigest()
    (args.output / "dense-image-hashes.json").write_text(json.dumps(hashes, indent=2) + "\n")
    for dark in (False, True):
        theme = "dark" if dark else "light"
        names = [f"push-{i}-{i+1}-{theme}" for i in range(-1, 7)]
        names += [f"pop-{i}-{i-1}-{theme}" for i in range(7, -1, -1)]
        output = args.output / f"cycle-{theme}.mp4"
        command = [shutil.which("ffmpeg") or "/opt/homebrew/bin/ffmpeg", "-y", "-loglevel", "error", "-f", "rawvideo", "-pixel_format", "rgb24", "-video_size", "804x132", "-framerate", "30", "-i", "pipe:0", "-an", "-c:v", "libx264", "-crf", "18", "-pix_fmt", "yuv420p", "-movflags", "+faststart", str(output)]
        encoder = subprocess.Popen(command, stdin=subprocess.PIPE)
        for name in names:
            for i in range(61):
                t = min(i / 60, .75)
                image = Image.new("RGB", (804, 132), "#dedede")
                draw = ImageDraw.Draw(image)
                for column, (label, clip, root) in enumerate((("Native UIKit", n[name], args.native), ("NXKit after", p[name], roots[name]))):
                    frame = at(clip, t)
                    x = column * 402
                    draw.text((x + 8, 6), f"{label}   sample {frame['t']:.3f}s", fill="black")
                    image.paste(thumbnail(root / frame["image"]), (x, 26))
                draw.text((8, 112), f"{name}   clock {t:.3f}s   playback 1/2 speed", fill="black")
                encoder.stdin.write(image.tobytes())
        encoder.stdin.close()
        assert encoder.wait() == 0
    print(json.dumps(report["summary"], indent=2))


if __name__ == "__main__":
    main()
