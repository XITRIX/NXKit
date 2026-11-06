"""Build a native public-UIKit oracle, including the production iOS corner bridge."""
import plistlib
from pathlib import Path
import subprocess
import sys

here = Path(__file__).resolve().parent
root = here.parent.parent
app = Path(sys.argv[1]).resolve() / 'Concentric.app'
app.mkdir(parents=True, exist_ok=True)
(app / 'Info.plist').write_bytes(plistlib.dumps({
    'CFBundleIdentifier': 'org.nxkit.ConcentricReference',
    'CFBundleExecutable': 'Concentric', 'CFBundleName': 'Concentric',
    'CFBundlePackageType': 'APPL', 'CFBundleVersion': '1',
    'CFBundleShortVersionString': '1.0', 'LSRequiresIPhoneOS': True,
    'UILaunchScreen': {}, 'UIDeviceFamily': [1, 2],
    'UIApplicationSceneManifest': {'UIApplicationSupportsMultipleScenes': False,
                                  'UISceneConfigurations': {}},
}))
subprocess.run([
    'xcrun', '--sdk', 'iphonesimulator', 'clang++', '-std=c++20',
    '-target', 'arm64-apple-ios15.0-simulator', '-fobjc-arc',
    '-I', str(root / 'UIKitCore/include'), str(here / 'ConcentricReference.mm'),
    str(root / 'UIKitCore/lib/platforms/apple/ios/IOSWindowGeometry.mm'),
    '-framework', 'UIKit', '-framework', 'Foundation', '-framework', 'QuartzCore',
    '-framework', 'CoreGraphics', '-o', str(app / 'Concentric'),
], check=True)
subprocess.run(['codesign', '--force', '--sign', '-', str(app)], check=True)
print(app)
