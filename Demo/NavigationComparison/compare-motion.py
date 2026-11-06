"""Compare live UIKit animation frames with deterministic portable rasters.

Usage: compare-motion.py NATIVE_DIR BEFORE_DIR AFTER_DIR OUTPUT_DIR
Requires Pillow and numpy. Inputs contain motion.json and its relative PNGs.
Time zero is the push/pop request. Geometry is interpolated on a fixed 60 Hz
grid through 650 ms; images use the nearest captured frame. No per-case time
offset, geometric alignment, resizing, or color fitting is applied to scores.
"""
import gzip
import hashlib
import json
import sys
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw


def read(root):
    return json.loads((root / 'motion.json').read_text())


def groups(frame):
    if 'groups' in frame:
        return [r for r in frame['groups'] if r[0] > 200]
    return [v['presented'] for v in frame['views']
            if 'UIPlatformGlassInteractionView' in v['class'] and v['presented'][0] > 200]


def retained(clip):
    if 'groups' in clip['frames'][0]:
        return [(f['t'], min(groups(f), key=lambda r: r[0])) for f in clip['frames']]
    initial = min((v for v in clip['frames'][0]['views']
                   if 'UIPlatformGlassInteractionView' in v['class'] and v['presented'][0] > 200),
                  key=lambda v: v['presented'][0])
    return [(f['t'], v['presented']) for f in clip['frames'] for v in f['views']
            if v['id'] == initial['id']]


GRID = np.arange(40) / 60
TOPOLOGY_CASES = {'split', 'join', 'text', 'accent', 'pop-split', 'pop-join'}


def trajectory(clip):
    samples = retained(clip)
    t, rects = np.array([x[0] for x in samples]), np.array([x[1] for x in samples])
    return np.array([np.interp(GRID, t, rects[:, i]) for i in range(4)]).T


def nearest(clip, t):
    return min(clip['frames'], key=lambda f: abs(f['t'] - t))


def image(root, frame):
    return Image.open(root / frame['image']).convert('RGB')


def silhouette(root, frame):
    # Black scene, no title, no content. The 8/255 threshold includes the glass
    # body/rim and excludes its black shadow; foreground is contained by glass.
    pixels = np.array(image(root, frame).crop((600, 0, 1194, 216)))
    return pixels.max(axis=2) > 8


def connected(mask):
    row = np.count_nonzero(mask[101:104], axis=0) >= 2
    runs = np.diff(np.r_[False, row, False].astype(int))
    return np.count_nonzero((np.flatnonzero(runs == -1) - np.flatnonzero(runs == 1)) >= 3) <= 1


def ink_sharpness(root, frame):
    # The stationary trailing icon in the unmatched replacement case. The
    # 28-point square excludes the 44-point capsule's rim. Energy normalization
    # removes opacity; a blur reduces high-frequency energy relative to ink.
    rgb = np.array(image(root, frame).crop((1050, 60, 1134, 144)), dtype=float)
    gray = rgb.mean(axis=2)
    ink = np.maximum(gray - np.percentile(gray, 10) - 2, 0)
    energy = float(np.sum(ink * ink))
    if ink.max() < 12 or energy < 100:
        return None
    return float((np.sum(np.diff(ink, axis=0) ** 2) + np.sum(np.diff(ink, axis=1) ** 2)) / energy)


def filmstrip(name, roots, clips, output):
    times = [0, .05, .1, .15, .2, .25, .3, .4, .6]
    result = Image.new('RGB', (744, 34 + len(times) * 91), (244, 244, 244))
    draw = ImageDraw.Draw(result)
    for col, label in enumerate(['UIKit', 'Before', 'After']):
        draw.text((col * 248 + 12, 10), label, fill='black')
        for row, t in enumerate(times):
            frame = nearest(clips[col][name], t)
            crop = image(roots[col], frame).crop((600 if name.startswith('text') else 780, 12, 1194, 222))
            crop.thumbnail((228, 82))
            result.paste(crop, (col * 248 + 12, 34 + row * 91))
            draw.text((col * 248 + 12, 34 + row * 91), f'{frame["t"]:.3f}s', fill='#d88c8c')
    result.save(output / f'{name}.png')
    frames = []
    for t in np.arange(25) / 30:
        frame = Image.new('RGB', (744, 136), (244, 244, 244))
        draw = ImageDraw.Draw(frame)
        for col, label in enumerate(['UIKit', 'Before', 'After']):
            draw.text((col * 248 + 12, 7), label, fill='black')
            shot = image(roots[col], nearest(clips[col][name], t)).crop((600 if name.startswith('text') else 780, 12, 1194, 222))
            shot.thumbnail((228, 110))
            frame.paste(shot, (col * 248 + 12, 24))
        frames.append(frame)
    frames[0].save(output / f'{name}.gif', save_all=True, append_images=frames[1:],
                   duration=[450] + [33] * 23 + [650], loop=0, disposal=2)


def main():
    roots = list(map(Path, sys.argv[1:4]))
    output = Path(sys.argv[4])
    output.mkdir(parents=True, exist_ok=True)
    data = [read(root) for root in roots]
    clips = [{c['name']: c for c in d['clips']} for d in data]
    results = []
    for name, native in clips[0].items():
        if name not in clips[2]:
            raise ValueError(f'Missing after clip: {name}')
        gaps = np.diff([f['t'] for f in native['frames'] if f['t'] < .7])
        if max(gaps) > .1:
            raise ValueError(f'{name}: native capture gap exceeds 100 ms; retain startup separately and repeat after warm-up')
        target = trajectory(native)
        row = {'name': name, 'nativeMaximumFrameGapSeconds': float(max(gaps))}
        for key, index in [('before', 1), ('after', 2)]:
            if name not in clips[index]:
                continue
            clip = clips[index][name]
            error = trajectory(clip) - target
            metrics = {'rectRMSEPt': float(np.sqrt(np.mean(error ** 2))),
                       'xywhRMSEPt': np.sqrt(np.mean(error ** 2, axis=0)).tolist(),
                       'maximumRectErrorPt': float(np.max(np.abs(error)))}
            if name.endswith('-dark'):
                ious, topology = [], []
                for t in GRID:
                    a = silhouette(roots[0], nearest(native, t))
                    b = silhouette(roots[index], nearest(clip, t))
                    ious.append(float(np.count_nonzero(a & b) / max(1, np.count_nonzero(a | b))))
                    topology.append(connected(a) == connected(b))
                metrics.update(meanSilhouetteIoU=float(np.mean(ious)),
                               connectedFrameAgreement=float(np.mean(topology)))
            row[key] = metrics
        results.append(row)
    common = [r for r in results if 'before' in r]
    summary = {}
    for key in ['before', 'after']:
        dark = [r[key] for r in common if r['name'].endswith('-dark')]
        topology = [r[key] for r in common if r['name'].endswith('-dark')
                    and r['name'].removesuffix('-dark') in TOPOLOGY_CASES]
        summary[key] = {
            'clips': len(common), 'framesPerClip': len(GRID),
            'retainedRectRMSEPt': float(np.sqrt(np.mean([r[key]['rectRMSEPt'] ** 2 for r in common]))),
            'darkClips': len(dark), 'meanDarkSilhouetteIoU': float(np.mean([r['meanSilhouetteIoU'] for r in dark])),
            'darkConnectedFrameAgreement': float(np.mean([r['connectedFrameAgreement'] for r in dark])),
            'joinSplitTopologyCases': len(topology),
            'joinSplitConnectedFrameAgreement': float(np.mean([r['connectedFrameAgreement'] for r in topology]))}
    blur = []
    for t in [.1, .15, .2, .25, .3, .4, .6]:
        blur.append({'t': t, **{label: ink_sharpness(roots[i], nearest(clips[i]['replace-unmatched-dark'], t))
                                for label, i in [('native', 0), ('after', 2)]}})
    result = {'nativeOS': data[0]['os'], 'scale': 3, 'sampleTimesSeconds': GRID.tolist(),
              'method': __doc__, 'summary': summary, 'clips': results, 'unmatchedInkSharpness': blur,
              'limitations': 'Silhouettes are thresholded dark rasters, not a material/color parity score. '
                             'Native snapshots sample the live scene with irregular frame intervals; '
                             'geometry is linearly interpolated without phase fitting. The separate compositor '
                             'recording validates the join sequence, not every transient pixel.'}
    (output / 'comparison.json').write_text(json.dumps(result, indent=2) + '\n')
    for label, root in zip(['native', 'before', 'after'], roots):
        (output / label).mkdir(exist_ok=True)
        with gzip.open(output / label / 'motion.json.gz', 'wt') as f:
            json.dump(read(root), f, separators=(',', ':'))
    for name in ['split-dark', 'join-dark', 'grow-dark', 'shrink-dark', 'text-dark', 'accent-dark', 'pop-split-dark', 'pop-join-dark']:
        filmstrip(name, roots, clips, output)
    # Preserve the unmatched/matched distinction without inventing a baseline
    # for the unmatched clip that was added after the initial implementation.
    for index, label in [(0, 'native'), (2, 'after')]:
        for name in ['replace-dark', 'replace-unmatched-dark']:
            chosen = [image(roots[index], nearest(clips[index][name], t)) for t in [0, .1, .15, .2, .25, .3, .6]]
            chosen[0].save(output / label / f'{name}.webp', lossless=True, save_all=True,
                           append_images=chosen[1:], duration=[150, 100, 100, 100, 100, 100, 500], loop=0)
    sources = ['Demo/NavigationComparison/MotionReference.m', 'Demo/NavigationComparison/build-motion.py', 'Demo/NavigationComparison/MotionRenderer.cpp',
               'Demo/NavigationComparison/compare-motion.py', 'UIKit/lib/UINavigationBar.cpp',
               'UIKit/lib/NavigationBarMotion.hpp', 'UIKit/lib/UINavigationBarInternal.hpp',
               'UIKit/tests/UINavigationTests.cpp']
    manifest = {p: hashlib.sha256(Path(p).read_bytes()).hexdigest() for p in sources}
    (output / 'source-manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
    print(json.dumps(summary, indent=2))


if __name__ == '__main__':
    main()
