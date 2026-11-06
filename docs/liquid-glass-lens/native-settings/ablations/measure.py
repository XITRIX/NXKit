"""Recompute native copied-filter ablations and verify archives; --check is read-only."""
from pathlib import Path
import argparse
import base64
import gzip
import importlib.util
import json
import numpy as np
from PIL import Image

ROOT = Path(__file__).resolve().parent
REPO = ROOT.parents[3]


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    value = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(value)
    return value


baseline = module('native_lens_unit_measure', ROOT.parent / 'unit-reference/measure.py')
decode = module('native_lens_archive_decode', REPO / 'Demo/GlassComparison/SettingsProbe/decode.py')


def normalize(value):
    if isinstance(value, dict):
        if 'archiveBase64' in value:
            return decode.unpack(base64.b64decode(value['archiveBase64']))
        return {k: normalize(v) for k, v in value.items()}
    if isinstance(value, list):
        return [normalize(v) for v in value]
    return value


def load(name):
    return np.array(Image.open(ROOT / (name + '.png')).convert('RGB'), float)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    ink = []
    for route in ['source', 'wrapper', 'foreground', 'backdrop', 'inner']:
        name = route + '-ink-blue'
        ink.append(dict(id=name, points=baseline.ink_points(
            load(name), load(route + '-ink-control'), [0, 122, 255])))
    backgrounds = []
    for route in ['source', 'foreground', 'backdrop']:
        for kind, roi in [('near', [350, 260.5, 20, 14]),
                          ('deep', [352, 270, 16, 10])]:
            name = route + '-backdrop-' + kind
            backgrounds.append(dict(baseline.measure(load(name), load(route + '-backdrop-control'),
                                                      roi, [0, 0, 0]), id=name))
    applied = {}
    for path in sorted(ROOT.glob('*.json.gz')):
        capture = json.loads(gzip.decompress(path.read_bytes()))
        audit = capture['lensFilterOverridesApplied']
        assert len(audit) == 1
        row = audit[0]
        assert row['verified'] and row['copiedFilter'] and row['applied'] == 0, path
        tree = normalize(next(lens for lens in capture['lensSettings']
                              if lens['identifier'] == 'manual-lens')['layerTree'])
        node = tree
        for component in row['path'].split('.'):
            if component:
                node = node['sublayers'][int(component)]
        effect = next(f for f in node['value']['filters']
                      if f['CAFilterType'] == row['filterType'])
        assert effect['CAFilterInputs'][row['key']] == 0, path
        applied[path.name[:-3]] = audit
    assert len(applied) == 19
    target = ROOT / 'metrics.json'
    existing = json.loads(target.read_text())
    if args.check:
        baseline.check_numbers(ink, existing['ink'])
        baseline.check_numbers(backgrounds, existing['backdrop'])
        assert applied == existing['applied']
    else:
        existing.update(ink=ink, backdrop=backgrounds, applied=applied)
        target.write_text(json.dumps(existing, indent=2) + '\n')
    print('19 native ablation archives and numeric records verified' if args.check else
          '19 native ablation archives verified; numeric records updated')


if __name__ == '__main__':
    main()
