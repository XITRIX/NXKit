"""Compare a real simctl framebuffer capture with the production SkSL material.

Usage: compare.py native-screen.png capture.json nxkit.ppm output-directory [before.ppm]
Requires Pillow. No color, exposure, alignment or per-shape corrections are applied.
"""
import json
from pathlib import Path
import re
import sys
from PIL import Image, ImageChops, ImageDraw, ImageFont, ImageStat

screen_path, metadata_path, nxkit_path, output_path, *before_path = sys.argv[1:]
output = Path(output_path)
output.mkdir(parents=True, exist_ok=True)
metadata = json.loads(Path(metadata_path).read_text())
scale = metadata["scale"]
box = tuple(round(v * scale) for v in (metadata["x"], metadata["y"],
    metadata["x"] + metadata["width"], metadata["y"] + metadata["height"]))
native = Image.open(screen_path).convert("RGB").crop(box)
nxkit = Image.open(nxkit_path).convert("RGB")
if native.size != nxkit.size:
    raise ValueError(f"Capture scales differ: native {native.size}, NXKit {nxkit.size}")
native.save(output / "native.png")
nxkit.save(output / "nxkit.png")
Path(output / "capture.json").write_text(json.dumps(metadata, indent=2) + "\n")
scene = Path(__file__).with_name("Scene.h").read_text()
elements = [list(map(float, row)) for row in re.findall(
    r"\{([\d.]+), ([\d.]+), ([\d.]+), ([\d.]+), ([\d.]+), ([01])\}", scene)]
renders = [("NXKit", nxkit)]
elements = metadata.get("elements", elements)
if before_path:
    renders.insert(0, ("Before", Image.open(before_path[0]).convert("RGB")))
    renders[0][1].save(output / "before.png")
metrics = {}
for name, image in renders:
    diff = ImageChops.difference(native, image)
    values = {}
    for index, (x, y, w, h, radius, clear) in enumerate(elements):
        # Exclude four points at the perimeter: these measurements assess the
        # material, separately from UIKit's continuous corner contour/shadow.
        mask = Image.new("L", native.size)
        ImageDraw.Draw(mask).rounded_rectangle(tuple(v * scale for v in
            (x + 4, y + 4, x + w - 4, y + h - 4)), radius=(radius - 4) * scale, fill=255)
        values[["clear", "regular_card", "regular_bar"][index]] = round(
            sum(ImageStat.Stat(diff, mask).mean) / 3, 3)
    metrics[name] = values
(output / "metrics.json").write_text(json.dumps(metrics, indent=2) + "\n")
try:
    font = ImageFont.truetype("/System/Library/Fonts/Supplemental/Arial.ttf", 20)
except OSError:
    font = ImageFont.load_default(size=20)

panels = [(f"Apple UIKit / iPadOS {metadata['os']} / {metadata.get('appearance', 'light')}", native),
          ("NXKit / production SkSL, raster backend", nxkit)]

def save_panels(panels, filename):
    comparison = Image.new("RGB", (720 * len(panels), 480), "#181c24")
    draw = ImageDraw.Draw(comparison)
    for index, (label, image) in enumerate(panels):
        draw.text((index * 720 + 12, 10), label, fill="white", font=font)
        comparison.paste(image.resize((720, 440), Image.Resampling.LANCZOS), (index * 720, 40))
    comparison.save(output / filename)

save_panels(panels, "comparison.png")
if before_path:
    save_panels([panels[0], ("NXKit before", renders[0][1]), panels[1]], "before-after.png")

# Identical enlarged crops; no sharpening or enhancement.
details = Image.new("RGB", (1280, 560), "#181c24")
draw = ImageDraw.Draw(details)
for row, (label, logical_box) in enumerate([
    ("Regular edge / 2x enlargement", (294, 155, 374, 215)),
    ("Clear refraction / 2x enlargement", (76, 205, 204, 270)),
]):
    for col, (name, image) in enumerate([("Apple UIKit", native), ("NXKit", nxkit)]):
        draw.text((col * 640 + 12, row * 280 + 8), f"{name}: {label}", fill="white", font=font)
        crop = image.crop(tuple(round(v * scale) for v in logical_box))
        details.paste(crop.resize((crop.width * 2, crop.height * 2), Image.Resampling.NEAREST),
                      (col * 640 + 12, row * 280 + 40))
details.save(output / "details.png")
print(json.dumps(metrics, indent=2))
