"""Measure static native material handoff after ICC conversion.

Usage: python analyze-merge-handoff.py CAPTURES OUTPUT.json
The body projection is a diagnostic, not an image correction or a parity score.
It fits one cubic smoothstep across both palettes, themes and creation orders.
Held/pulled images are excluded so they remain independent optical checkpoints.
"""
from pathlib import Path
import json
import sys
import numpy as np
from comparison_images import load_srgb, COLOR_MANAGEMENT

captures, output = map(Path, sys.argv[1:3])
samples = []
reversal = []
for directory in sorted(captures.iterdir()):
    if not (directory / 'trace.json').exists():
        continue
    trace = json.loads((directory / 'trace.json').read_text())
    config = trace['configuration']
    if not config.get('pair') or config.get('gap', 12) >= 8:
        continue
    tints = trace.get('resolvedItemTints', {})
    # An explicit tint owns neutral material independent of insertion order;
    # two explicit tints retain the insertion order observed in native images.
    left_owns = bool(config.get('reverseOrder') and tints.get('glass0') is not None)
    image = np.asarray(load_srgb(directory / 'initial.png'), dtype=float)
    scale = trace['scale']
    colors = []
    for i in range(2):
        cx = 360 + (i - .5) * (52 + config['gap'])
        values = [image[round(scale * (220 + dy)), round(scale * (cx + dx))]
                  for dx in [-10, -8, 8, 10] for dy in [-10, -8, 8, 10]]
        colors.append(np.mean(values, axis=0))
    back, front = colors[::-1] if left_owns else colors
    delta = front - back
    boundary = 360 - config['gap'] / 2 if left_owns else 360 + config['gap'] / 2
    for distance in np.arange(2, 10, .5):
        x = boundary + distance if left_owns else boundary - distance
        color = image[round(scale * 220), round(scale * x)]
        weight = np.dot(color - back, delta) / np.dot(delta, delta)
        samples.append(dict(case=directory.name, distance=float(distance), weight=float(weight)))
    if not config.get('reverseOrder'):
        reverse = directory.with_name(directory.name.replace('-forward', '-reverse'))
        if reverse.exists():
            for phase in ['initial', 'release-5']:
                a = np.asarray(load_srgb(directory / (phase + '.png')), dtype=float)[388:492, 640:800]
                b = np.asarray(load_srgb(reverse / (phase + '.png')), dtype=float)[388:492, 640:800]
                reversal.append(dict(case=directory.name, phase=phase,
                                     mean=float(abs(a-b).mean()), maximum=float(abs(a-b).max())))
distances = np.array([s['distance'] for s in samples])
observed = np.array([s['weight'] for s in samples])
best = None
for start in np.arange(8, 12, .1):
    for end in np.arange(0, 4, .1):
        t = np.clip((start - distances) / (start - end), 0, 1)
        loss = np.mean((t*t*(3-2*t) - observed)**2)
        if best is None or loss < best['meanSquaredWeightError']:
            best = dict(meanSquaredWeightError=float(loss), start=float(start), end=float(end))
output.write_text(json.dumps(dict(colorManagement=COLOR_MANAGEMENT,
    method='Native static central row projected onto each material body control; initial images only; no renderer or image fitting.',
    fit=best, reversal=reversal, samples=samples), indent=2) + '\n')
print(json.dumps(best, indent=2))
