"""Measure portable/native held-outs at declared geometry, without image fitting."""
from pathlib import Path
import argparse
import gzip
import hashlib
import importlib.util
import json
import numpy as np
from PIL import Image, ImageCms

ROOT = Path(__file__).resolve().parent
REPO = ROOT.parents[3]
spec = importlib.util.spec_from_file_location('native_lens_measure', ROOT / 'measure.py')
measure = importlib.util.module_from_spec(spec)
spec.loader.exec_module(measure)
sha = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--reference', required=True, type=Path)
parser.add_argument('--portable', required=True, type=Path)
parser.add_argument('--renderer', required=True, type=Path)
args = parser.parse_args()
folder = args.reference.resolve()
case = next(c for c in json.loads((folder / 'cases.json').read_text())
            if c['contentSetup'] == 'portal-only')
name = case['id']
metadata = json.loads(gzip.decompress((folder / (name + '.json.gz')).read_bytes()))
scale = metadata['scale']
frame = next(row['frame'] for row in metadata['hierarchy'] if row.get('identifier') == 'manual-lens')
origin = (frame[0] + frame[2] / 2 - case['centerX'], frame[1] + frame[3] / 2 - case['centerY'])
points = [(p[0] + origin[0], p[1] + origin[1]) for p in case['markerPoints']]
control = name.replace('-blue', '-control')
inputs = {}
records = {}
for revision, paths in [('native', [folder / (n + '.png') for n in [name, control]]),
                         ('portable', [args.portable / (n + '.ppm') for n in [name, control]])]:
    images = [Image.open(path).convert('RGB') for path in paths]
    assert all(image.size == (720 * scale, 560 * scale) for image in images)
    records[revision] = measure.ink_points(np.array(images[0], float), np.array(images[1], float),
                                          case['markerColor'], scale, points,
                                          (frame[1] + .5, frame[1] + frame[3] - .5))
    inputs[revision] = {path.name: sha(path) for path in paths}
    if revision == 'portable':
        profile = ImageCms.ImageCmsProfile(ImageCms.createProfile('sRGB')).tobytes()
        for label, image in zip(['blue', 'control'], images):
            image.save(folder / ('portable-' + label + '.png'), icc_profile=profile)
comparisons = []
for native, portable in zip(records['native'], records['portable']):
    comparisons.append(dict(edge=native['edge'],
                            GBSeparation=dict(native=native['GBSeparation'], portable=portable['GBSeparation'],
                                              absoluteError=abs(native['GBSeparation'] - portable['GBSeparation'])),
                            radialGBSeparation=(dict(native=native['radialGBSeparation'], portable=portable['radialGBSeparation'],
                                                     absoluteError=abs(native['radialGBSeparation'] - portable['radialGBSeparation']))
                                                 if native['radialGBSeparation'] is not None else None),
                            channelCentroidAbsoluteError=np.linalg.norm(np.array(native['centroids']) -
                                                                       portable['centroids'], axis=1).tolist()))
sources = ['UIKit/lib/LensMaterial.cpp', 'UIKit/lib/NativeLensShader.hpp',
           'UIKit/lib/NativeLensProfile.cpp', 'UIKit/lib/NativeLensProfileData.hpp',
           'UIKit/lib/NativeGlassLighting.cpp']
abi = REPO / 'Demo/build/ios-sim-ninja/Thirdparty/UIKit/UIKitCore/UIKitCore-metal-generated/NXMetalShaderABI.hpp'
result = dict(noFit=True,
              method='Fixed declared/captured model geometry and independent native/portable empty-source controls. '
                     'No pixel registration, optical gain fit, exposure adjustment or resampling. Native PNGs are retained sRGB; '
                     'portable PPM RGB bytes are interpreted as sRGB, matching the existing raster comparison convention.',
              portableInvocation=dict(scale=scale, scaleArgument=('--scale 3' if scale == 3 else 'default 2')),
              size=frame[2:], frame=frame, scale=scale, sourceRGB=case['markerColor'],
              comparisons=comparisons, records=records, inputSHA256=inputs,
              renderer=dict(path=str(args.renderer.resolve()), SHA256=sha(args.renderer)),
              sourceSHA256={name: sha(REPO / name) for name in sources},
              shaderABI=dict(path=str(abi), SHA256=sha(abi)) if abi.exists() else None,
              captureExtentPolicy='Portable virtual extent = logical size + sqrt(2)*native margin, independent of display scale. '
                                  'Inferred reconstruction policy; actual native allocator padding remains unmeasured.')
(folder / 'portable-metrics.json').write_text(json.dumps(result, indent=2) + '\n')
for row in comparisons:
    print(row['edge'], row['GBSeparation'], row['radialGBSeparation'])
