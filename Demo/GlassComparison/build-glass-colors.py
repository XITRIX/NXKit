"""Build a public-UIKit foreground color/vibrancy comparison fixture."""
from pathlib import Path
import sys,plistlib,subprocess,hashlib,json
out=Path(sys.argv[1]).resolve();app=out/'LensReference.app';app.mkdir(parents=True,exist_ok=True)
source=Path(__file__).with_name('GlassColorReference.m');snapshot=out/'GlassColorReference.m';snapshot.write_bytes(source.read_bytes())
info=plistlib.loads(source.with_name('Info.plist').read_bytes());info.update(CFBundleIdentifier='org.nxkit.LensReference',CFBundleExecutable='LensReference',CFBundleName='Public Glass Colors');(app/'Info.plist').write_bytes(plistlib.dumps(info))
sdk=subprocess.check_output(['xcrun','--sdk','iphonesimulator','--show-sdk-path'],text=True).strip()
subprocess.run(['xcrun','--sdk','iphonesimulator','clang','-target','arm64-apple-ios26.0-simulator','-isysroot',sdk,'-fobjc-arc','-framework','UIKit','-framework','QuartzCore',str(snapshot),'-o',str(app/'LensReference')],check=True)
subprocess.run(['codesign','--force','--sign','-',str(app)],check=True)
(app/'fixture-source.json').write_text(json.dumps({'sourceSHA256':hashlib.sha256(source.read_bytes()).hexdigest()}))
