#!/usr/bin/env python3
"""Compare the complete native/production eight-layout navigation matrix.

No per-clip onset alignment or time warping is performed. Native samples keep
their actual CADisplayLink times; portable geometry is interpolated at those
times. PNGs retain their independent checkpoint timestamps.
"""
import argparse
import gzip
import json
import math
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw


def read(root):
    return json.loads((root / "cycle.json").read_text())


def groups(frame, native, left):
    rows = ([v for v in frame["views"] if "UIPlatformGlassInteractionView" in v["class"]]
            if native else frame["groups"])
    return sorted([r for r in rows if (r["frame"][0] < 130) == left], key=lambda r: r["frame"][0])


def series(clip, native, left, index):
    result = []
    for frame in clip["frames"]:
        rows = groups(frame, native, left)
        # An absent participant is invisible at the Back center. Geometry is
        # scored with opacity weights so invisible coordinates are immaterial.
        rect, alpha = ([38, 22, 0, 0], 0)
        if len(rows) > index:
            rect, alpha = rows[index]["frame"], rows[index]["alpha"]
        result.append([frame["t"], *rect, alpha])
    return np.asarray(result)


def interpolate(rows, times):
    return np.column_stack([np.interp(times, rows[:, 0], rows[:, j]) for j in range(1, rows.shape[1])])


def content(clip, field):
    pop = clip["pop"]
    start = 0 if field == "sourceContent" else (-402 * .3 if pop else 402)
    end = (402 if pop else -402 * .3) if field == "sourceContent" else 0
    return np.asarray([[f["t"], f[field][0] if f[field] else (start if f["t"] == 0 else end)] for f in clip["frames"]])


def score(native, portable):
    times = np.asarray([f["t"] for f in native["frames"] if .04 <= f["t"] <= .65])
    back = interpolate(series(native, True, True, 0), times)
    ours = interpolate(series(portable, False, True, 0), times)
    back_error = float(np.mean((back[:, :4] - ours[:, :4]) ** 2))
    result = {"backRectMSE": back_error}
    if 5 in (native["source"], native["target"]):
        star = interpolate(series(native, True, True, 1), times)
        ours = interpolate(series(portable, False, True, 1), times)
        weight = np.maximum(star[:, 4], ours[:, 4])
        result["starWeightedRectMSE"] = float(np.sum(np.mean((star[:, :4] - ours[:, :4]) ** 2, axis=1) * weight) / max(.0001, weight.sum()))
        result["starOpacityMSE"] = float(np.mean((star[:, 4] - ours[:, 4]) ** 2))
    errors = []
    for field in ("sourceContent", "targetContent"):
        errors.extend((interpolate(content(native, field), times)[:, 0] - interpolate(content(portable, field), times)[:, 0]) ** 2)
    result["contentXMSE"] = float(np.mean(errors))
    # Hidden-background UIKit hosts remain in the hierarchy even when they do
    # not draw material. The portable 'groups' trace includes real materials;
    # exclude those transitions from right material count/rectangle scoring.
    if 7 not in (native["source"], native["target"]):
        errors, agree = [], []
        for f in native["frames"]:
            if not .04 <= f["t"] <= .65:
                continue
            other = min(portable["frames"], key=lambda p: abs(p["t"] - f["t"]))
            a = [r for r in groups(f, True, False) if r["alpha"] > .1]
            b = [r for r in groups(other, False, False) if r["alpha"] > .1]
            agree.append(len(a) == len(b))
            if len(a) == len(b):
                errors.extend((np.asarray(x["frame"]) - y["frame"]) ** 2 for x, y in zip(a, b))
        result["rightCountAgreement"] = float(np.mean(agree))
        if errors:
            result["rightMatchedRectMSE"] = float(np.mean(errors))
    return result


def aggregate(rows):
    keys = sorted({k for row in rows for k in row})
    output = {}
    for key in keys:
        values = [row[key] for row in rows if key in row]
        value = float(np.mean(values))
        output[key.replace("MSE", "RMSE")] = math.sqrt(value) if key.endswith("MSE") else value
    return output


def pictures(clip):
    frames = [f for f in clip["frames"] if "image" in f]
    return [frames[i] for i in (0, 6, 12, 18, 24, 30, 36, 48, 72, 90)] if len(frames) == 91 else frames


def filmstrip(name, roots, clips, output):
    frames = [pictures(c) for c in clips]
    labels = ["Native UIKit", "NXKit before", "NXKit after"] if len(roots) == 3 else ["Native UIKit", "NXKit after"]
    width, row_height = 402, 96
    image = Image.new("RGB", (width * len(roots), 26 + row_height * 10), "#dedede")
    draw = ImageDraw.Draw(image)
    for k, label in enumerate(labels):
        draw.text((k * width + 8, 7), label, fill="black")
    animation = []
    for i in range(10):
        panel = Image.new("RGB", (width * len(roots), 106), "#dedede")
        pd = ImageDraw.Draw(panel)
        for k, root in enumerate(roots):
            frame = frames[k][i]
            thumbnail = Image.open(root / frame["image"]).convert("RGB").resize((width, 80), Image.Resampling.LANCZOS)
            image.paste(thumbnail, (k * width, 26 + i * row_height))
            draw.text((k * width + 4, 107 + i * row_height), f"{frame['t']:.3f}s", fill="black")
            pd.text((k * width + 8, 5), labels[k] + f"  {frame['t']:.3f}s", fill="black")
            panel.paste(thumbnail, (k * width, 26))
        animation.append(panel)
    image.save(output / f"{name}.png")
    animation[0].save(output / f"{name}.gif", save_all=True, append_images=animation[1:], duration=[100, 100, 100, 100, 100, 100, 200, 400, 300, 600], loop=0)


def contacts(native_root, portable_root, native, portable, output):
    # All 288 clips, with actual timestamps, are reviewable in these sheets.
    for dark in (False, True):
        for pop in (False, True):
            for source in range(0 if pop else -1, 8):
                cases = [c for c in native if c["dark"] == dark and c["pop"] == pop and c["source"] == source]
                width, height = 201, 53
                sheet = Image.new("RGB", (width * 6, 28 + len(cases) * height * 2), "#dedede")
                draw = ImageDraw.Draw(sheet)
                draw.text((6, 7), f"{'Pop' if pop else 'Push'} from {source}, {'dark' if dark else 'light'} — each pair: native / NXKit after", fill="black")
                for row, clip in enumerate(cases):
                    for side, (root, current) in enumerate(((native_root, clip), (portable_root, portable[clip["name"]]))):
                        images = pictures(current)
                        for column, checkpoint in enumerate((0, 2, 3, 5, 7, 9)):
                            frame = images[checkpoint]
                            thumb = Image.open(root / frame["image"]).convert("RGB").resize((width, 40), Image.Resampling.LANCZOS)
                            x, y = column * width, 28 + (row * 2 + side) * height
                            sheet.paste(thumb, (x, y))
                            draw.text((x + 2, y + 40), f"to {clip['target']} · {frame['t']:.3f}s", fill="black")
                sheet.save(output / f"matrix-{'pop' if pop else 'push'}-{source}-{'dark' if dark else 'light'}.png")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("native", type=Path)
    parser.add_argument("before", type=Path)
    parser.add_argument("after", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--before-composites", type=Path, help="parent of optional corrected baseline composite directories")
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    reports = [read(root) for root in (args.native, args.before, args.after)]
    maps = [{c["name"]: c for c in r["clips"]} for r in reports]
    assert all(len(m) == 288 and m.keys() == maps[0].keys() for m in maps)
    cases = []
    for clip in reports[0]["clips"]:
        assert len(pictures(clip)) == 10
        cases.append({k: clip[k] for k in ("name", "source", "target", "pop", "dark")} | {
            "before": score(clip, maps[1][clip["name"]]), "after": score(clip, maps[2][clip["name"]])})
    summary = {}
    for label, subset in (("all", cases), ("leadingChange", [c for c in cases if (c["source"] == 5) != (c["target"] == 5)]), ("unchangedLeading", [c for c in cases if c["source"] >= 0 and c["target"] >= 0 and (c["source"] == 5) == (c["target"] == 5)])):
        summary[label] = {"cases": len(subset)} | {stage: aggregate([c[stage] for c in subset]) for stage in ("before", "after")}
    report = {"nativeOS": reports[0]["os"], "scale": 3, "cases": 288, "timeAlignment": "none", "maxNativeSamplingGap": max(b["t"] - a["t"] for c in reports[0]["clips"] for a, b in zip(c["frames"], c["frames"][1:])), "summary": summary, "clips": cases}
    (args.output / "comparison.json").write_text(json.dumps(report, indent=2) + "\n")
    for stage, root in zip(("native", "before", "after"), (args.native, args.before, args.after)):
        with gzip.GzipFile(args.output / f"{stage}-trace.json.gz", "wb", mtime=0) as f:
            f.write((root / "cycle.json").read_bytes())
    contacts(args.native, args.after, reports[0]["clips"], maps[2], args.output)
    for name in ("push-4-5-light", "pop-5-4-light", "push--1-5-dark", "pop-5--1-dark", "push-5-5-light", "push-0-3-dark", "pop-3-0-dark", "push-3-4-light"):
        roots, clips = [args.native, args.after], [maps[0][name], maps[2][name]]
        if args.before_composites:
            baseline = args.before_composites / ("nxkit-cycle-before-composite-" + name.rsplit("-", 1)[0] + "-")
            if (baseline / "cycle.json").exists():
                roots.insert(1, baseline)
                clips.insert(1, next(c for c in read(baseline)["clips"] if c["name"] == name))
        filmstrip(name, roots, clips, args.output)
    print(json.dumps(summary, indent=2))


if __name__ == "__main__":
    main()
