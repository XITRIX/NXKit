"""Capture native endpoint geometry and stable OS screenshots, without XCTest."""
from pathlib import Path
import hashlib, json, os, plistlib, shutil, subprocess, sys, time

device, output = sys.argv[1], Path(sys.argv[2]).resolve()
source = Path(__file__).resolve().parent
output.mkdir(parents=True, exist_ok=True)
app = output / 'EndpointReference.app'
app.mkdir(exist_ok=True)
info = plistlib.loads((source.parent / 'GlassComparison/Info.plist').read_bytes())
info.update(CFBundleIdentifier='org.nxkit.SliderEndpointReference', CFBundleName='Slider Endpoints', CFBundleExecutable='EndpointReference')
(app / 'Info.plist').write_bytes(plistlib.dumps(info))
shutil.copy2(source / 'EndpointReference.m', output / 'EndpointReference.m')
sdk = subprocess.check_output(['xcrun', '--sdk', 'iphonesimulator', '--show-sdk-path'], text=True).strip()
subprocess.run(['xcrun', '--sdk', 'iphonesimulator', 'clang', '-target', 'arm64-apple-ios26.0-simulator',
                '-isysroot', sdk, '-fobjc-arc', '-framework', 'UIKit', '-framework', 'QuartzCore',
                '-framework', 'Foundation', '-framework', 'CoreGraphics', str(output / 'EndpointReference.m'),
                '-o', str(app / 'EndpointReference')], check=True)
subprocess.run(['codesign', '--force', '--sign', '-', str(app)], check=True)
subprocess.run(['xcrun', 'simctl', 'install', device, str(app)], check=True)
container = Path(subprocess.check_output(['xcrun', 'simctl', 'get_app_container', device,
                                        info['CFBundleIdentifier'], 'data'], text=True).strip())
variants = ['default', 'thumbless', 'custom-thumb', 'ticks', 'neutral', 'neutral-center',
            'images', 'disabled', 'width180', 'range', 'range-neutral', 'range-neutral-center', 'track-images']
if '--continuous-ticks' in sys.argv:
    variants = ['ticks', 'neutral', 'neutral-center', 'range-neutral', 'range-neutral-center']
if '--quick' in sys.argv:
    variants = ['default', 'thumbless', 'neutral', 'range-neutral']
results = []
for dark in ([False] if '--continuous-ticks' in sys.argv else [False, True]):
    for variant in variants:
        case = {'id': variant + ('-dark' if dark else '-light'), 'variant': variant, 'dark': dark, 'continuousTicks': '--continuous-ticks' in sys.argv}
        start = time.time()
        environment = dict(os.environ, SIMCTL_CHILD_SLIDER_CASE=json.dumps(case))
        subprocess.run(['xcrun', 'simctl', 'launch', '--terminate-running-process', device,
                        info['CFBundleIdentifier']], env=environment, check=True, stdout=subprocess.DEVNULL)
        ready = container / 'Documents' / case['id'] / 'ready.txt'
        deadline = time.monotonic() + 15
        while not (ready.exists() and ready.stat().st_mtime >= start):
            assert time.monotonic() < deadline, case
            time.sleep(.05)
        destination = output / case['id']
        shutil.copytree(ready.parent, destination, dirs_exist_ok=True)
        subprocess.run(['xcrun', 'simctl', 'io', device, 'screenshot', str(destination / 'screen.png')],
                       check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        geometry = json.loads((destination / 'geometry.json').read_text())
        assert geometry['scale'] == 3, geometry['scale']
        result = dict(case, geometrySHA256=hashlib.sha256((destination / 'geometry.json').read_bytes()).hexdigest(),
                      screenshotSHA256=hashlib.sha256((destination / 'screen.png').read_bytes()).hexdigest())
        results.append(result)
        (output / 'manifest.json').write_text(json.dumps({'device': device,
            'sourceSHA256': hashlib.sha256((output / 'EndpointReference.m').read_bytes()).hexdigest(),
            'cases': results, 'capture': 'Stable native OS screenshots; no drawViewHierarchy or authored lens state'}, indent=2) + '\n')
        print(case['id'], flush=True)
subprocess.run(['xcrun', 'simctl', 'terminate', device, info['CFBundleIdentifier']], check=True)
