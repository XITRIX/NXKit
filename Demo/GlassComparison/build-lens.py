"""Build the independent, native-only private-lens research fixture.
Usage: python build-lens.py OUTPUT
Requires Pillow/NumPy for deterministic background assets, plus Xcode.
"""
from pathlib import Path
import plistlib
import hashlib
import json
from PIL import ImageDraw
import subprocess
import sys
import numpy as np
from PIL import Image, ImageCms

output = Path(sys.argv[1]).resolve()
output.mkdir(parents=True, exist_ok=True)
source = Path(__file__).resolve().parent
app = output / 'LensReference.app'
app.mkdir(exist_ok=True)
profile = ImageCms.ImageCmsProfile(ImageCms.createProfile('sRGB')).tobytes()
colors = {'black': (0, 0, 0), 'white': (255, 255, 255), 'gray': (128, 128, 128),
          'red': (255, 0, 0), 'green': (0, 255, 0), 'blue': (0, 0, 255), 'yellow': (255, 255, 0)}
for name, color in colors.items():
    Image.new('RGB', (720, 560), color).save(app / f'{name}.png', icc_profile=profile)
y, x = np.mgrid[:560, :720]
lines = np.full((560, 720, 3), 128, dtype=np.uint8)
lines[x % 40 < 3] = 255
lines[y % 40 < 3] = 0
Image.fromarray(lines).save(app / 'lines.png', icc_profile=profile)
ramp = np.stack([np.round(x / 719 * 255), np.round(y / 559 * 255), np.full_like(x, 128)], axis=2).astype('uint8')
Image.fromarray(ramp).save(app / 'ramp.png', icc_profile=profile)
angle = 2 * np.pi * (x + y * .35) / 120
stripes = np.stack([128 + 110 * np.sin(angle), 128 + 110 * np.sin(angle + 2.094),
                    128 + 110 * np.sin(angle + 4.189)], axis=2).clip(0, 255).astype('uint8')
Image.fromarray(stripes).save(app / 'stripes.png', icc_profile=profile)
# Native edge probe underlay, plus an opaque2D coordinate ramp at exact2× size.
grid = np.full((560, 720, 3), 235, dtype=np.uint8)
grid[x % 16 < 1] = (18, 18, 18)
grid[y % 16 < 1] = (245, 245, 245)
grid[(x % 64 < 2)] = (255, 35, 10)
grid[(y % 64 < 2)] = (0, 90, 255)
Image.fromarray(grid).save(app / 'probe-grid.png', icc_profile=profile)
map_y, map_x = np.mgrid[:408, :1536]
coordinate_map = np.stack([np.round(map_x / 1535 * 255), np.round(map_y / 407 * 255),
                           np.full_like(map_x, 128)], axis=2).astype('uint8')
Image.fromarray(coordinate_map).save(app / 'probe-map.png', icc_profile=profile)
# Backdrop-only probe: full-stage source, independent of selected portal input.
backdrop_markers = Image.new('RGB',(1440,1120),(128,128,128))
marker_draw = ImageDraw.Draw(backdrop_markers)
for index,(mx,my) in enumerate([(360.25,268),(360.25,308),(310.25,288),(410.25,288),(322.25,272),(398.25,272)]):
    value=0 if index%2==0 else 255
    marker_draw.ellipse((mx*2-4,my*2-4,mx*2+4,my*2+4),fill=(value,value,value))
backdrop_markers.save(app / 'backdrop-markers.png',icc_profile=profile)
edge_markers=Image.new('RGB',(1440,1120),(128,128,128)); edge_draw=ImageDraw.Draw(edge_markers)
for index,(mx,my) in enumerate([(360.25,263),(360.25,313),(305.25,288),(415.25,288),(312.57233047,270.32233047),(407.92766953,270.32233047)]):
    value=0 if index%2==0 else 255
    edge_draw.ellipse((mx*2-4,my*2-4,mx*2+4,my*2+4),fill=(value,value,value))
edge_markers.save(app/'backdrop-edge-markers.png',icc_profile=profile)
fixture_digest=hashlib.sha256((source/'LensReference.m').read_bytes()).hexdigest()
fixture_source=output/f'LensReference-{fixture_digest}.m'
fixture_source.write_bytes((source/'LensReference.m').read_bytes())
(output/'SettingsProbe').mkdir(exist_ok=True)
(output/'SettingsProbe/RuntimeInspection.h').write_bytes((source/'SettingsProbe/RuntimeInspection.h').read_bytes())
(app/'fixture-source.json').write_text(json.dumps({'sourceSHA256':fixture_digest,'sourceSnapshot':fixture_source.name,'inspectionHeaderSHA256':hashlib.sha256((source/'SettingsProbe/RuntimeInspection.h').read_bytes()).hexdigest()},indent=2)+'\n')
info = plistlib.loads((source / 'Info.plist').read_bytes())
info.update(CFBundleIdentifier='org.nxkit.LensReference', CFBundleName='Lens Research', CFBundleExecutable='LensReference')
(app / 'Info.plist').write_bytes(plistlib.dumps(info))
sdk = subprocess.check_output(['xcrun', '--sdk', 'iphonesimulator', '--show-sdk-path'], text=True).strip()
subprocess.run(['xcrun', '--sdk', 'iphonesimulator', 'clang', '-target', 'arm64-apple-ios26.0-simulator',
                '-isysroot', sdk, '-fobjc-arc', '-framework', 'UIKit', '-framework', 'QuartzCore',
                '-framework', 'Foundation', '-framework', 'CoreGraphics', str(fixture_source), '-o', str(app / 'LensReference')], check=True)
subprocess.run(['codesign', '--force', '--sign', '-', str(app)], check=True)
print(app)
