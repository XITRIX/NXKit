"""Capture settled framebuffer + runtime metadata from the standalone lens oracle.
Usage: python capture-lens.py DEVICE OUTPUT [CASES_JSON]
Native-only private API stays in LensReference.m; this driver uses simctl.
"""
from pathlib import Path
import hashlib
import io
import json
import subprocess
import sys
import time
import uuid
from PIL import Image, ImageCms

device, output = sys.argv[1], Path(sys.argv[2]).resolve()
native = output / 'native'
native.mkdir(exist_ok=True)
def sim(*args):
    return subprocess.check_output(['xcrun', 'simctl', *args], text=True, timeout=120).strip()
bundle = 'org.nxkit.LensReference'
sim('install', device, str(output / 'LensReference.app'))
sim('launch', '--terminate-running-process', device, bundle)
documents = Path(sim('get_app_container', device, bundle, 'data')) / 'Documents'
cases_path = Path(sys.argv[3]) if len(sys.argv) > 3 else Path(__file__).with_name('lens-cases.json')
cases = json.loads(cases_path.read_text())
records = []
profile = ImageCms.ImageCmsProfile(ImageCms.createProfile('sRGB')).tobytes()
executable_hash = hashlib.sha256((output / 'LensReference.app/LensReference').read_bytes()).hexdigest()
build_manifest=output/'LensReference.app/fixture-source.json'
source_hash=json.loads(build_manifest.read_text())['sourceSHA256'] if build_manifest.exists() else None
for original_request in cases:
    # The simulator can migrate the data container during launch, so resolve it
    # after launch. A fresh nonce rejects stale acknowledgements for repeated IDs.
    request = dict(original_request, captureNonce=str(uuid.uuid4()))
    staged = documents / 'request.tmp'
    staged.write_text(json.dumps(request))
    staged.replace(documents / 'request.json')
    deadline = time.monotonic() + 25
    while True:
        try:
            metadata = json.loads((documents / 'capture.json').read_text())
        except (FileNotFoundError, json.JSONDecodeError):
            metadata = {}
        if metadata.get('request') == request:
            break
        if time.monotonic() > deadline:
            raise RuntimeError(f'{request["id"]}: native acknowledgement timed out')
        time.sleep(.04)
    name = request['id']
    raw = native / (name + '-framebuffer.png')
    sim('io', device, 'screenshot', str(raw))
    image = Image.open(raw)
    icc = image.info.get('icc_profile')
    if not icc and image.info.get('srgb') is None:
        raise RuntimeError('Framebuffer color space is unspecified')
    scale = metadata['scale']
    x, y = metadata['x'] * scale, metadata['y'] * scale
    crop = image.crop((x, y, x + metadata['width'] * scale, y + metadata['height'] * scale)).convert('RGB')
    if icc:
        crop = ImageCms.profileToProfile(crop, ImageCms.ImageCmsProfile(io.BytesIO(icc)),
                                        ImageCms.createProfile('sRGB'), outputMode='RGB')
    crop.save(native / (name + '.png'), icc_profile=profile)
    metadata.update(nativeInspectionHeaderSHA256=json.loads(build_manifest.read_text()).get('inspectionHeaderSHA256') if build_manifest.exists() else None, nativeExecutableSHA256=executable_hash, nativeFixtureSourceSHA256=source_hash,
                    captureColorSpace='Embedded ICC converted to sRGB' if icc else 'Framebuffer PNG sRGB chunk')
    (native / (name + '.json')).write_text(json.dumps(metadata, indent=2) + '\n')
    if metadata.get('sourceAssets'):
        assets = native / 'source-assets'
        assets.mkdir(exist_ok=True)
        for filename in metadata['sourceAssets'].values():
            if Path(filename).name != filename:
                raise RuntimeError('Native source asset must be a simple filename')
            source = Image.open(documents / filename)
            source_icc = source.info.get('icc_profile')
            if source_icc:
                source = ImageCms.profileToProfile(source, ImageCms.ImageCmsProfile(io.BytesIO(source_icc)),
                                                  ImageCms.createProfile('sRGB'), outputMode='RGBA')
            elif source.info.get('srgb') is not None:
                source = source.convert('RGBA')
            else:
                raise RuntimeError(f'{filename}: source color space is unspecified')
            source.save(assets / filename, icc_profile=profile)
    if request.get('exportContent'):
        assets = native / 'content'
        assets.mkdir(exist_ok=True)
        for kind in ['normal', 'selected']:
            filename = f'content-dark{int(bool(request.get("dark")))}-{kind}.png'
            source = Image.open(documents / filename)
            source_icc = source.info.get('icc_profile')
            if source_icc:
                source = ImageCms.profileToProfile(source, ImageCms.ImageCmsProfile(io.BytesIO(source_icc)),
                                                  ImageCms.createProfile('sRGB'), outputMode='RGBA')
            elif source.info.get('srgb') is not None:
                source = source.convert('RGBA')
            else:
                raise RuntimeError(f'{filename}: content color space is unspecified')
            source.save(assets / filename, icc_profile=profile)
    records.append({k: v for k, v in metadata.items() if k not in ['trace', 'hierarchy', 'classes']})
    (native / 'series.json').write_text(json.dumps(records, indent=2) + '\n')
    print(name, flush=True)
