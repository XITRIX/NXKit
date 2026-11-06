"""Verify retained native tab-shadow artifacts; no UIKit process is launched."""
import gzip, hashlib, json
from pathlib import Path

root = Path(__file__).resolve().parents[2] / 'docs/liquid-glass-lens/tab-shadow/native'
sha = lambda data: hashlib.sha256(data).hexdigest()
manifest = json.loads((root / 'manifest.json').read_text())
assert manifest['testExitCode'] == 0
sources = json.loads((root / 'source-manifest.json').read_text())
for name, expected in sources.items():
    assert sha((root / name).read_bytes()) == expected
total = 0
largest = 0
for case in ['tab-shadow-light', 'tab-shadow-dark']:
    directory = root / case
    compressed = (directory / 'trace.json.gz').read_bytes()
    raw = gzip.decompress(compressed)
    provenance = json.loads((directory / 'provenance.json').read_text())
    assert sha(raw) == provenance['rawTraceSHA256']
    assert sha(compressed) == provenance['gzipSHA256']
    data = json.loads(raw)
    assert data['configuration']['control'] == 'tab' and data['scale'] == 3
    assert not data['rootControlFlagsAvailable']
    assert [t['phase'] for t in data['touches']] == [0, 3]
    assert data['schema'][0]['delegateClass'] == 'UITabBar'
    end = data['touches'][-1]['dispatchT']
    samples = []
    for frame in data['frames']:
        layers = {layer['id']: layer for layer in frame['layers']}
        material = next((f for layer in frame['layers'] for f in layer['presentationFilters'] if f['type'] == 'glassBackground'), None)
        if material is None:
            continue
        assert len([key for key in material if key.startswith('input')]) == 70
        samples.append(material)
        q = material['inputFaceOpacity']
        if frame['t'] >= end and 0 < q < 1:
            for key, active in [('inputInnerRefractionAmount', -10.5), ('inputInnerRefractionHeight', 7), ('inputShadowAmount', 17.5)]:
                largest = max(largest, abs(material[key] / active - q))
    assert samples and samples[-1]['inputFaceOpacity'] == 0
    assert samples[-1]['inputRingShadowOpacity'] == 0
    assert samples[-1]['inputShadowAmount'] == 0
    total += len(samples)
assert largest < 1e-12
selected = json.loads((root / 'selected-frames.json').read_text())
assert sha((root / 'native-motion.mp4').read_bytes()) == selected['videoSHA256']
for frame in selected['frames']:
    assert sha((root / frame['image']).read_bytes()) == frame['imageSHA256']
print(f'PASS: 2 real-contact native tab traces, {total} complete material samples, 10 native video crops and source hashes.')
print(f'Normalized interior material-clock maximum residual: {largest:.3g}.')
print('Coverage: retained artifact verification only; no new UIKit execution, device result, arbitrary regrab or custom tab-layout claim.')
