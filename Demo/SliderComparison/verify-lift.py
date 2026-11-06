#!/usr/bin/env python3
"""Verify retained native artifacts; this does not execute UIKit."""
import gzip
import hashlib
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2] / 'docs/slider/lift-motion'


def sha(data):
    return hashlib.sha256(data).hexdigest()


traces = {}
for path in sorted((ROOT / 'native').glob('*/*/trace.json.gz')):
    compressed = path.read_bytes()
    raw = gzip.decompress(compressed)
    provenance = json.loads(path.with_name('provenance.json').read_text())
    assert sha(raw) == provenance.get('rawSHA256', provenance.get('rawTraceSHA256'))
    assert sha(compressed) == provenance.get('gzipSHA256', provenance.get('compressedTraceSHA256'))
    data = json.loads(raw)
    assert data['frames'] and data['touches']
    assert all(a['t'] <= b['t'] for a, b in zip(data['frames'], data['frames'][1:]))
    contacts = sum(t['phase'] == 0 for t in data['touches'])
    ends = sum(t['phase'] in (3, 4) for t in data['touches'])
    if '--quiet' not in sys.argv:
        print(f'{path.parent.name}: {len(data["frames"])} frames, {contacts} contacts/{ends} ends')
    traces[path.parent.name] = (sha(raw), data)

video = json.loads((ROOT / 'video/manifest.json').read_text())
for name, digest in video['imageSHA256'].items():
    assert sha((ROOT / 'video' / name).read_bytes()) == digest, name
for recording in video['recordings']:
    assert sha((ROOT / 'video' / (recording['id'] + '.mp4')).read_bytes()) == recording['sourceSHA256']
    for case in recording['cases']:
        digest, data = traces[case['id']]
        assert digest == case['traceSHA256']
        ids = {f['frameID'] for f in data['frames']}
        assert all(frame['markerID'] in ids for frame in case['frames'])

direct = traces['direct-filter-slider'][1]
end = direct['touches'][-1]['dispatchT']
blur_errors, release_warp, clear_errors = [], [], []
for frame in direct['frames']:
    layers = {layer['id']: layer for layer in frame['layers']}
    q = (layers[5]['presentationBounds'][2] - 37) / 20.35
    below = layers.get(8, {})
    filters = below.get('presentationFilters', [])
    blur = next((f['inputRadius'] for f in filters if 'inputRadius' in f), None)
    warp = next((f['inputAmount'] for f in filters if 'inputAmount' in f), None)
    if blur is not None and 0 < q < 1:
        blur_errors.append(abs(blur - 6 * (1 - q)))
    if frame['t'] >= end and warp is not None:
        release_warp.append(warp)
    clear = next((f['inputAmount'] for f in layers.get(13, {}).get('presentationFilters', []) if 'inputAmount' in f), None)
    white = layers[7]['presentationOpacity']
    if frame['t'] >= end and clear is not None and 0 < white < 1:
        clear_errors.append(abs(clear + 17.5 * (1 - white)))
assert blur_errors and max(blur_errors) < 1e-10
assert release_warp and all(value == 0 for value in release_warp)
assert clear_errors and max(clear_errors) < 1e-6
ablation_root = ROOT / 'native/fill-ablation'
ablation = json.loads((ablation_root / 'selected-frames.json').read_text())
assert sha((ablation_root / 'native-ablation.mp4').read_bytes()) == ablation['sourceVideoSHA256']
for frame in ablation['frames']:
    assert sha((ablation_root / frame['image']).read_bytes()) == frame['imageSHA256']
    trace = traces[frame['case']][1]
    assert any(sample['frameID'] == frame['markerID'] and sample['t'] == frame['traceTime'] for sample in trace['frames'])
material = traces['material-inputs-slider'][1]
material_samples = [f for frame in material['frames'] for layer in frame['layers'] if layer['id'] == 21
                    for f in layer.get('presentationFilters', []) if f['type'] == 'glassBackground']
assert material_samples
assert all(len([key for key in sample if key.startswith('input')]) == 70 for sample in material_samples)
assert all(sample['inputRefractionOpacity'] == 0 for sample in material_samples)
assert material_samples[0]['inputFaceOpacity'] == 1 and material_samples[-1]['inputFaceOpacity'] == 0
print(f'Direct-filter interior residual: blur {max(blur_errors):.3g}, ClearGlass/white {max(clear_errors):.3g}; release below displacement exactly zero')
print(f'PASS: {len(traces)} native traces, {len(video["imageSHA256"])} image hashes, video hashes and frame-marker references verified')
print(f'PASS: 8 native fill-ablation crops/source video and {len(material_samples)} complete material input samples verified')
print('Limits: retained-artifact verification only; no new runtime observation, physical-device claim, true native regrab coverage, or 120Hz capture claim.')
