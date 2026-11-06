"""Compare fixed-size contrast controls across two native UIKit runtimes.
Usage: analyze-contrast.py older-contrast-directory newer-contrast-directory output-directory
"""
import json
from pathlib import Path
import sys
from PIL import Image, ImageDraw, ImageFont

older, newer, output = map(Path, sys.argv[1:])
output.mkdir(parents=True, exist_ok=True)
try:
    font = ImageFont.truetype("/System/Library/Fonts/Supplemental/Arial.ttf", 16)
except OSError:
    font = ImageFont.load_default(size=16)
records = [json.loads((directory / "contrast.json").read_text()) for directory in [older, newer]]
results = []
for appearance in ["light", "dark"]:
    montage = Image.new("RGB", (1440, 940), "#161922")
    draw = ImageDraw.Draw(montage)
    for size_index, size_name in enumerate(["small", "large"]):
        for version_index, cases in enumerate(records):
            row = size_index * 2 + version_index
            for col, background in enumerate(["black", "white", "gray", "checker"]):
                case = next(c for c in cases if c["id"].startswith(f"{appearance}-{size_name}-{background}-"))
                m = case["metadata"]
                scale = m["scale"]
                box = tuple(round(v * scale) for v in (m["x"], m["y"], m["x"]+720, m["y"]+440))
                image = Image.open(Path(case["frame"]) / "screen.png").convert("RGB").crop(box)
                crop = image.crop(tuple(v * scale for v in (270, 115, 630, 315)))
                name = f"{m['os']}-{case['id']}"
                crop.save(output / f"{name}.png")
                (output / f"{name}.json").write_text(json.dumps(m, indent=2) + "\n")
                x,y,w,h,_,_ = m["elements"][1]
                body = image.getpixel((round((x+w/2)*scale), round((y+h*.2)*scale)))
                def luma(rgb): return sum(c*k for c,k in zip(rgb,[.2126,.7152,.0722]))
                body_luma = luma(body)
                label = image.crop(tuple(round(v*scale) for v in (x+w/2-29,y+h/2-11,x+w/2+29,y+h/2+11)))
                ink_lumas = [luma(p) for p in label.getdata()]
                ink_direction = "light" if max(ink_lumas)-body_luma > body_luma-min(ink_lumas) else "dark"
                if background == "checker": ink_direction = "not classified on patterned backdrop"
                results.append({"os":m["os"],"osBuild":m.get("osBuild"),"appearance":appearance,
                    "size":size_name,"background":background,"cardBodyRGB":body,
                    "cardContentAppearance":m["contentAppearances"][1],
                    "resolvedLabelColor":m["resolvedLabelColors"][1],"observedInk":ink_direction})
                draw.text((col*360+8,row*235+5),f"{m['os']} / {appearance} / {size_name} / {background}",font=font,fill="white")
                crop.thumbnail((360,200),Image.Resampling.LANCZOS)
                montage.paste(crop,(col*360,row*235+30))
    montage.save(output / f"{appearance}-contrast.png")
(output / "results.json").write_text(json.dumps(results,indent=2)+"\n")
for result in results:
    if result["background"] in ["black","white"]:
        print(result)
