"""Recompute retained native unit controls; --check preserves recorded metrics."""
from pathlib import Path
import argparse
import json
import numpy as np
from PIL import Image

ROOT = Path(__file__).resolve().parent


def load(name):
    return np.array(Image.open(ROOT / (name + '.png')).convert('RGB'), float)


def measure(image, control, region, color, scale=2):
    # Match LensRefractionTests.cpp: truncating point-to-pixel loop bounds,
    # denominator exclusion, no upper coverage clamp, and threshold .1.
    image, control = np.asarray(image, float), np.asarray(control, float)
    x0, y0, width, height = region
    assert np.isfinite(scale) and scale > 0
    assert image.shape == control.shape and image.shape[2] == 3
    xs = np.arange(int(x0 * scale), int((x0 + width) * scale))
    ys = np.arange(int(y0 * scale), int((y0 + height) * scale))
    assert len(xs) and len(ys) and xs[0] >= 0 and ys[0] >= 0
    assert xs[-1] < image.shape[1] and ys[-1] < image.shape[0]
    pixels = image[ys[:, None], xs]
    backdrop = control[ys[:, None], xs]
    denominator = np.array(color) - backdrop
    coverage = np.divide(pixels - backdrop, denominator,
                         out=np.zeros_like(pixels), where=abs(denominator) >= 16)
    weights = np.maximum(0, coverage - .1)
    mass = weights.sum((0, 1))
    xx, yy = np.meshgrid((xs + .5) / scale, (ys + .5) / scale)
    centers = np.stack(((weights * xx[:, :, None]).sum((0, 1)) / mass,
                        (weights * yy[:, :, None]).sum((0, 1)) / mass), axis=1)
    return dict(mass=mass.tolist(), centroids=centers.tolist(), roi=region)


def ink_points(image, control, color, scale=2, points=None,
               clip_y=(260.5, 315.5)):
    result = []
    if points is None:
        points = [(360.25, 268), (360.25, 308), (310.25, 288), (410.25, 288)]
    assert len(points) == 4
    for i, point in enumerate(points):
        lo, hi = max(clip_y[0], point[1] - 18), min(clip_y[1], point[1] + 18)
        row = measure(image, control, [point[0] - 18, lo, 36, hi - lo], color, scale)
        centers = np.array(row['centroids'])
        axis, sign = (0 if i < 2 else 1), (-1 if i in (1, 3) else 1)
        row.update(edge=['top', 'bottom', 'left', 'right'][i],
                   GBSeparation=float(sign * (centers[1, axis] - centers[2, axis])),
                   radialGBSeparation=(float((centers[2, 0] - centers[1, 0]) *
                                             (-1 if i == 3 else 1)) if i >= 2 else None))
        result.append(row)
    return result


def check_numbers(actual, expected):
    """Check numeric records independently of unchanged provenance strings."""
    if isinstance(actual, dict):
        for key, value in actual.items():
            check_numbers(value, expected[key])
    elif isinstance(actual, list):
        assert len(actual) == len(expected)
        for a, e in zip(actual, expected):
            check_numbers(a, e)
    elif isinstance(actual, (int, float)):
        assert np.isfinite(actual) and abs(actual - expected) < 1e-9, (actual, expected)
    else:
        assert actual == expected, (actual, expected)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    rows = []
    for dark in (0, 1):
        control = load(f'ink-control-dark{dark}')
        for color, rgb in [('blue', [0, 122, 255]), ('cyan', [0, 210, 255])]:
            name = f'ink-{color}-dark{dark}'
            rows.append(dict(id=name, sourceRGB=rgb,
                             points=ink_points(load(name), control, rgb)))
    backgrounds = []
    for name, roi in [('backdrop-unit-near', [350, 260.5, 20, 14]),
                      ('backdrop-unit-deep', [352, 270, 16, 10])]:
        backgrounds.append(dict(measure(load(name), load('backdrop-unit-control'),
                                        roi, [0, 0, 0]), id=name))
    for file, records in [('ink-metrics.json', rows),
                          ('backdrop-unit-metrics.json', backgrounds)]:
        target = ROOT / file
        existing = json.loads(target.read_text())
        if args.check:
            check_numbers(records, existing['records'])
        else:
            existing['records'] = records
            target.write_text(json.dumps(existing, indent=2) + '\n')
    print('Native unit-reference numeric records verified' if args.check else
          'Native unit-reference numeric records updated')


if __name__ == '__main__':
    main()
