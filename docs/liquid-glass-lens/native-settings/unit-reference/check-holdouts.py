"""Verify retained margin, wide-geometry and3x controls without image fitting."""
from pathlib import Path
import gzip
import importlib.util
import json
import numpy as np
from PIL import Image

ROOT = Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location('native_lens_measure', ROOT / 'measure.py')
measure = importlib.util.module_from_spec(spec)
spec.loader.exec_module(measure)

for directory in ['margin-reference', 'wide-reference', 'phone-reference']:
    folder = ROOT.parent / directory
    expected = json.loads((folder / 'metrics.json').read_text())
    cases = json.loads((folder / 'cases.json').read_text())
    for case in cases:
        if case['contentSetup'] != 'portal-only':
            continue
        name = case['id']
        metadata = json.loads(gzip.decompress((folder / (name + '.json.gz')).read_bytes()))
        frame = next(row['frame'] for row in metadata['hierarchy']
                     if row.get('identifier') == 'manual-lens')
        # Transform declared source points using captured model geometry.
        # This is a coordinate conversion, never pixel registration or fitting.
        origin = (frame[0] + frame[2] / 2 - case['centerX'],
                  frame[1] + frame[3] / 2 - case['centerY'])
        points = [(point[0] + origin[0], point[1] + origin[1])
                  for point in case['markerPoints']]
        image = np.array(Image.open(folder / (name + '.png')).convert('RGB'), float)
        control = np.array(Image.open(folder / (name.replace('-blue', '-control') + '.png')).convert('RGB'), float)
        actual = measure.ink_points(image, control, case['markerColor'],
                                    scale=metadata['scale'], points=points,
                                    clip_y=(frame[1] + .5, frame[1] + frame[3] - .5))
        reference = (next(row['points'] for row in expected['records'] if row['id'] == name)
                     if directory == 'margin-reference' else expected['records'])
        measure.check_numbers(actual, reference)
        overrides = metadata.get('lensFilterOverridesApplied', [])
        if directory == 'margin-reference':
            assert len(overrides) == 1
            row = overrides[0]
            assert row['before'] == 100 and row['verified'] and row['copiedFilter']
            assert row['applied'] == case['lensFilterOverrides']['foregroundMargin']
        else:
            assert not overrides
        print(name, 'verified at', metadata['scale'], 'x')
