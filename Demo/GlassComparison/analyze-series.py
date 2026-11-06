"""Create matched light/dark and tint/resize evidence from capture-series.py.

Usage: analyze-series.py series-directory output-directory
Requires Pillow. All metrics use original RGB pixels; montages are only resized
for display. The animated previews show settled checkpoints, not frame timing.
"""
import json
from pathlib import Path
import sys
from PIL import Image, ImageChops, ImageDraw, ImageFont, ImageStat

source, output = map(Path, sys.argv[1:])
output.mkdir(parents=True, exist_ok=True)
records = json.loads((source / "series.json").read_text())
images = {}
results = []
for record in records:
    frame = Path(record["frame"])
    metadata = json.loads((frame / "capture.json").read_text())
    scale = metadata["scale"]
    box = tuple(round(v * scale) for v in (metadata["x"], metadata["y"],
        metadata["x"] + metadata["width"], metadata["y"] + metadata["height"]))
    native = Image.open(frame / "screen.png").convert("RGB").crop(box)
    raster = Image.open(record["raster"]).convert("RGB")
    if native.size != raster.size:
        raise ValueError("Mismatched native and raster scale")
    images[record["id"]] = (native, raster)
    result = {k: record[k] for k in ["id", "appearance", "tint", "size"]}
    result["geometry"] = metadata["elements"]
    result["mean_channel_error"] = {}
    for name, (x, y, w, h, radius, _) in zip(["clear", "regular_card", "regular_bar"], metadata["elements"]):
        mask = Image.new("L", native.size)
        ImageDraw.Draw(mask).rounded_rectangle(tuple(v * scale for v in
            (x + 4, y + 4, x + w - 4, y + h - 4)),
            radius=min(radius - 4, (w - 8) / 2, (h - 8) / 2) * scale, fill=255)
        error = ImageStat.Stat(ImageChops.difference(native, raster), mask)
        result["mean_channel_error"][name] = round(sum(error.mean) / 3, 3)
    results.append(result)
(output / "metrics.json").write_text(json.dumps(results, indent=2) + "\n")
try:
    font = ImageFont.truetype("/System/Library/Fonts/Supplemental/Arial.ttf", 17)
except OSError:
    font = ImageFont.load_default(size=17)

for appearance in ["light", "dark"]:
    montage = Image.new("RGB", (1440, 654), "#161922")
    draw = ImageDraw.Draw(montage)
    for col, tint in enumerate(["none", "blue", "coral"]):
        identifier = f"{appearance}-{tint}-4"
        if identifier not in images:
            continue
        for row, (name, image) in enumerate(zip(["Apple UIKit", "NXKit raster"], images[identifier])):
            draw.text((col * 480 + 10, row * 327 + 8), f"{name} / {appearance} / {tint}", font=font, fill="white")
            montage.paste(image.resize((480, 293), Image.Resampling.LANCZOS), (col * 480, row * 327 + 30))
    montage.save(output / f"{appearance}-tints.png")
    # Card-only crops retain the same fixed background and position at every
    # size, making the material's growth and contraction easy to inspect.
    animation = []
    for index in range(9):
        frame = Image.new("RGB", (1080, 488), "#161922")
        draw = ImageDraw.Draw(frame)
        for col, tint in enumerate(["none", "blue", "coral"]):
            identifier = f"{appearance}-{tint}-{index}"
            if identifier not in images:
                continue
            result = next(r for r in results if r["id"] == identifier)
            w, h = result["geometry"][1][2:4]
            for row, (name, image) in enumerate(zip(["UIKit", "NXKit raster"], images[identifier])):
                draw.text((col * 360 + 8, row * 230 + 8), f"{name} / {tint} / {w:g} x {h:g}", font=font, fill="white")
                crop = image.crop((540, 230, 1260, 630)).resize((360, 200), Image.Resampling.LANCZOS)
                frame.paste(crop, (col * 360, row * 230 + 30))
        draw.text((8, 466), "Settled checkpoints; timing illustrative. UIKit above, NXKit raster below.", font=font, fill="white")
        animation.append(frame)
    animation[0].save(output / f"{appearance}-resize.gif", save_all=True, append_images=animation[1:],
        duration=[1000, 700, 700, 700, 1400, 700, 700, 700, 1400], loop=0, disposal=2)

neutral = Image.new("RGB", (1800, 880), "#161922")
draw = ImageDraw.Draw(neutral)
for mode, appearance in enumerate(["light", "dark"]):
    for index in range(5):
        identifier = f"{appearance}-none-{index}"
        if identifier not in images:
            continue
        for engine, (name, image) in enumerate(zip(["UIKit", "NXKit raster"], images[identifier])):
            row = mode * 2 + engine
            draw.text((index * 360 + 8, row * 220 + 3), f"{appearance} / {name} / step {index}", font=font, fill="white")
            neutral.paste(image.crop((540, 230, 1260, 630)).resize((360, 200), Image.Resampling.LANCZOS),
                (index * 360, row * 220 + 20))
neutral.save(output / "neutral-sizes.png")

initial = source / "light-initial"
if (initial / "screen.png").exists() and "light-none-4" in images:
    metadata = json.loads((initial / "capture.json").read_text())
    scale = metadata["scale"]
    box = tuple(round(v * scale) for v in (metadata["x"], metadata["y"],
        metadata["x"] + 720, metadata["y"] + 440))
    fresh = Image.open(initial / "screen.png").convert("RGB").crop(box)
    resized, raster = images["light-none-4"]
    comparison = Image.new("RGB", (2160, 480), "#161922")
    draw = ImageDraw.Draw(comparison)
    for index, (label, image) in enumerate([
        ("UIKit fresh / light traits / 300 x 160", fresh),
        ("UIKit after resizing / light traits / 300 x 160", resized),
        ("NXKit raster / light / 300 x 160", raster),
    ]):
        draw.text((index * 720 + 10, 10), label, fill="white", font=font)
        comparison.paste(image.resize((720, 440), Image.Resampling.LANCZOS), (index * 720, 40))
    comparison.save(output / "fresh-vs-resized.png")
    # Retain source pixels and the effective light trait for this important
    # distinction, independently of the presentation montage.
    fresh.save(output / "fresh-light.png")
    resized.save(output / "resized-light.png")
    (output / "fresh-capture.json").write_text(json.dumps(metadata, indent=2) + "\n")

history = {}
for appearance in ["light", "dark"]:
    for tint in ["none", "blue", "coral"]:
        values = []
        for index in range(4):
            a, b = f"{appearance}-{tint}-{index}", f"{appearance}-{tint}-{8-index}"
            if a in images and b in images:
                values.append(round(sum(ImageStat.Stat(ImageChops.difference(images[a][0], images[b][0])).mean) / 3, 4))
        history[f"{appearance}-{tint}"] = values
(output / "history.json").write_text(json.dumps(history, indent=2) + "\n")
print("Full-scene native growth/contraction differences:", json.dumps(history))
print("Largest card error:", max((r["mean_channel_error"]["regular_card"], r["id"]) for r in results))
