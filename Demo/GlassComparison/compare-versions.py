"""Compare fresh and restored native glass on two OS versions.
Usage: compare-versions.py older-probe-directory newer-probe-directory output-directory
All metrics use original pixels. The montage crops the same scene rectangle.
"""
import json
from pathlib import Path
import sys
from PIL import Image, ImageChops, ImageDraw, ImageFont, ImageStat

older, newer, output = map(Path, sys.argv[1:])
output.mkdir(parents=True, exist_ok=True)
try:
    font = ImageFont.truetype("/System/Library/Fonts/Supplemental/Arial.ttf", 17)
except OSError:
    font = ImageFont.load_default(size=17)
montage = Image.new("RGB", (1440, 610), "#161922")
draw = ImageDraw.Draw(montage)
results = []
for row, directory in enumerate([older, newer]):
    fresh = None
    for col, (name, label) in enumerate([
        ("fresh", "Fresh 300 x 160"),
        ("2-1-False", "120 x 64 -> 300 x 160, direct"),
        ("4-1-True", "120 x 64 -> 300 x 160, animated"),
    ]):
        metadata = json.loads((directory / f"{name}.json").read_text())
        scale = metadata["scale"]
        scene_box = tuple(round(v * scale) for v in (metadata["x"], metadata["y"],
            metadata["x"] + 720, metadata["y"] + 440))
        scene = Image.open(directory / f"{name}.png").convert("RGB").crop(scene_box)
        if fresh is None:
            fresh = scene
        mask = Image.new("L", scene.size)
        ImageDraw.Draw(mask).rounded_rectangle(tuple(v * scale for v in (304, 134, 596, 286)), radius=48 * scale, fill=255)
        stats = ImageStat.Stat(scene, mask)
        difference = sum(ImageStat.Stat(ImageChops.difference(fresh, scene), mask).mean) / 3
        results.append({"os": metadata["os"], "osBuild": metadata.get("osBuild"),
            "state": name, "effectiveAppearance": metadata["effectiveAppearance"],
            "meanRGB": stats.mean, "meanChannelDifferenceFromFresh": difference})
        prefix = f"ios-{metadata['os']}-{name}"
        # Save unscaled, unmodified crop pixels as evidence, plus the actual
        # scene/trait metadata. Montage resampling is for presentation only.
        card_context = scene.crop(tuple(v * scale for v in (270, 115, 630, 315)))
        card_context.save(output / f"{prefix}.png")
        (output / f"{prefix}.json").write_text(json.dumps(metadata, indent=2) + "\n")
        draw.text((col * 480 + 10, row * 305 + 7), f"iPadOS {metadata['os']} / {label}", font=font, fill="white")
        montage.paste(card_context.resize((480, 267), Image.Resampling.LANCZOS), (col * 480, row * 305 + 32))
montage.save(output / "comparison.png")
(output / "metrics.json").write_text(json.dumps(results, indent=2) + "\n")
print(json.dumps(results, indent=2))
