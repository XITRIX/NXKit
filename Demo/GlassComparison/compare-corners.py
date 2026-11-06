"""Align native solid CALayers with the portable corner regression fixture."""
import json
import sys
from pathlib import Path
import numpy as np
from PIL import Image, ImageDraw, ImageFont

screen, metadata, raster, output = map(Path, sys.argv[1:])
output.mkdir(parents=True, exist_ok=True)
meta = json.loads(metadata.read_text())
scale = meta['scale']
native_screen = Image.open(screen).convert('RGB')
x, y = meta['x'] * scale, meta['y'] * scale
native = native_screen.crop((round(x), round(y), round(x + 720 * scale), round(y + 760 * scale)))
nxkit = Image.open(raster).convert('RGB')
native.save(output / 'native.png')
nxkit.save(output / 'nxkit.png')
results = []
for row, (top, height, radius) in enumerate(zip(meta['ys'], meta['heights'], meta['radii'])):
    for column, curve in enumerate(['circular', 'continuous']):
        rect = (int((20 + 360 * column) * scale), int(top * scale),
                int((320 + 360 * column) * scale), int((top + height) * scale))
        a, b = np.asarray(native.crop(rect), dtype=float), np.asarray(nxkit.crop(rect), dtype=float)
        aa, bb = a[:, :, 0] > 127, b[:, :, 0] > 127
        results.append({'curve': curve, 'radius': radius, 'height': height,
                        'meanAbsoluteChannelError': float(np.mean(np.abs(a-b))),
                        'silhouetteIntersectionOverUnion': float(np.sum(aa & bb) / np.sum(aa | bb))})
(output / 'metrics.json').write_text(json.dumps({'native': meta, 'results': results}, indent=2) + '\n')
# One canonical corner, enlarged with nearest-neighbor sampling so AA is visible.
font = ImageFont.load_default(size=20)
tiles = []
for column, curve in enumerate(['circular', 'continuous']):
    for label, source in [('UIKit', native), ('NXKit', nxkit)]:
        rect = ((20 + 360*column)*scale, 260*scale,
                (20 + 360*column + 82)*scale, (260 + 82)*scale)
        detail = source.crop(tuple(map(int, rect))).resize((328, 328), Image.Resampling.NEAREST)
        tile = Image.new('RGB', (352, 384), '#202124')
        ImageDraw.Draw(tile).text((12, 12), f'{label} {curve}', fill='white', font=font)
        tile.paste(detail, (12, 44))
        tiles.append(tile)
montage = Image.new('RGB', (352 * len(tiles), 384))
for index, tile in enumerate(tiles):
    montage.paste(tile, (index * 352, 0))
montage.save(output / 'comparison.png')
for result in results:
    print(result)
