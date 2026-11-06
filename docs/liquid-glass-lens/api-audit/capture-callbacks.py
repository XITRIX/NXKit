"""Build and run one bounded private-lens callback/configuration research case."""
from pathlib import Path
import hashlib, json, plistlib, shutil, subprocess, sys, time
device=sys.argv[1]; output=Path(sys.argv[2]).resolve(); output.mkdir(parents=True,exist_ok=True)
source=Path(__file__).resolve().parent
app=output/'LensCallbackReference.app'; app.mkdir(exist_ok=True)
info=plistlib.loads(Path('Demo/GlassComparison/Info.plist').read_bytes())
info.update(CFBundleIdentifier='org.nxkit.LensCallbackReference',CFBundleName='Lens Callback Research',CFBundleExecutable='LensCallbackReference')
(app/'Info.plist').write_bytes(plistlib.dumps(info)); shutil.copy2(source/'CallbackReference.m',output/'CallbackReference.m')
sdk=subprocess.check_output(['xcrun','--sdk','iphonesimulator','--show-sdk-path'],text=True).strip()
subprocess.run(['xcrun','--sdk','iphonesimulator','clang','-target','arm64-apple-ios26.0-simulator','-isysroot',sdk,'-fobjc-arc','-framework','UIKit','-framework','QuartzCore','-framework','Foundation',str(output/'CallbackReference.m'),'-o',str(app/'LensCallbackReference')],check=True)
subprocess.run(['codesign','--force','--sign','-',str(app)],check=True)
subprocess.run(['xcrun','simctl','install',device,str(app)],check=True)
container=Path(subprocess.check_output(['xcrun','simctl','get_app_container',device,info['CFBundleIdentifier'],'data'],text=True).strip())
path=container/'Documents/callbacks.json'; path.unlink(missing_ok=True)
subprocess.run(['xcrun','simctl','launch','--terminate-running-process',device,info['CFBundleIdentifier']],check=True)
deadline=time.monotonic()+15
while not path.exists():
    assert time.monotonic()<deadline,'native callback timeout'; time.sleep(.05)
shutil.copy2(path,output/'callbacks.json')
subprocess.run(['xcrun','simctl','terminate',device,info['CFBundleIdentifier']],check=True)
d=json.loads(path.read_text()); print(json.dumps({'device':device,'sourceSHA256':hashlib.sha256((source/'CallbackReference.m').read_bytes()).hexdigest(),'resultSHA256':hashlib.sha256(path.read_bytes()).hexdigest(),**d}))
