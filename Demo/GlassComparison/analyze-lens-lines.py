"""Measure native source-line support against an identical no-content lens.

Usage: python analyze-lens-lines.py NATIVE_DIRECTORY OUTPUT_JSON
This reports observed intervals, not a fitted or recovered private equation.
"""
import json
import math
from pathlib import Path
import sys

import numpy as np
from PIL import Image

native, destination = map(Path, sys.argv[1:3])
threshold = 10
rows = []
for metadata_path in sorted([*native.glob('scan-*.json'), *native.glob('dense-*.json')]):
    metadata = json.loads(metadata_path.read_text())
    request = metadata.get('request', {})
    pattern = request.get('probePattern', '')
    if pattern not in ('scan-x', 'scan-y'):
        continue
    style, dark = int(request.get('style', 1)), int(bool(request.get('dark')))
    control_id = f'scan-none-d{dark}-s{style}'
    control = json.loads((native / f'{control_id}.json').read_text())
    # The ink pattern and its coordinate are the intentional differences.
    for key in ('width', 'height', 'centerX', 'centerY', 'base', 'lifted',
                'liftInset', 'warps', 'contentMode', 'liftContainer'):
        assert request.get(key) == control['request'].get(key), (metadata_path, key)
    frame = next(view['frame'] for view in metadata['hierarchy']
                 if view.get('identifier') == 'manual-lens')
    scale = metadata['scale']
    x, y, width, height = frame
    image = np.asarray(Image.open(metadata_path.with_suffix('.png'))).astype(float)
    reference = np.asarray(Image.open(native / f'{control_id}.png')).astype(float)
    if pattern == 'scan-y':
        fixed = round((x + width / 2) * scale)
        start, end = math.floor(y * scale), math.ceil((y + height) * scale)
        change = np.abs(image[start:end, fixed] - reference[start:end, fixed]).mean(1)
        bar_origin = 250
    else:
        fixed = round((y + height / 2) * scale)
        start, end = math.floor(x * scale), math.ceil((x + width) * scale)
        change = np.abs(image[fixed, start:end] - reference[fixed, start:end]).mean(1)
        bar_origin = 40
    active = np.flatnonzero(change > threshold)
    support = []
    for group in np.split(active, np.where(np.diff(active) > 1)[0] + 1):
        if len(group):
            support.append({
                'barInterval': [(start + int(group[0])) / scale - bar_origin,
                                (start + int(group[-1]) + 1) / scale - bar_origin],
                'peakMeanRGBChange': round(float(change[group].max()), 3),
            })
    rows.append({'id': request['id'], 'axis': pattern[-1], 'style': style,
                 'sourceCoordinate': request['scanCoordinate'],
                 'sourceWidthPoints': 1, 'controlID': control_id,
                 'nativeFrame': frame, 'support': support})
destination.write_text(json.dumps({
    'method': 'Native minus matched shader-only control; half-open pixel-area intervals.',
    'thresholdMeanRGB': threshold, 'colorSpace': 'sRGB', 'rows': rows,
}, indent=2) + '\n')
print(f'{len(rows)} measured source-line scenes: {destination}')
