"""Build native UISlider oracle, using only the installed UIKit SDK."""
from pathlib import Path
import hashlib, json, plistlib, subprocess, sys
output=Path(sys.argv[1]).resolve(); output.mkdir(parents=True,exist_ok=True)
source=Path(__file__).resolve().parent
app=output/'SliderReference.app';app.mkdir(exist_ok=True)
info=plistlib.loads((source.parent/'GlassComparison/Info.plist').read_bytes())
info.update(CFBundleIdentifier='org.nxkit.SliderReference',CFBundleName='Slider Reference',CFBundleExecutable='SliderReference')
(app/'Info.plist').write_bytes(plistlib.dumps(info))
sdk=subprocess.check_output(['xcrun','--sdk','iphonesimulator','--show-sdk-path'],text=True).strip()
subprocess.run(['xcrun','--sdk','iphonesimulator','clang','-target','arm64-apple-ios26.0-simulator','-isysroot',sdk,'-fobjc-arc','-framework','UIKit','-framework','QuartzCore','-framework','Foundation','-framework','CoreGraphics',str(source/'SliderReference.m'),'-o',str(app/'SliderReference')],check=True)
subprocess.run(['codesign','--force','--sign','-',str(app)],check=True)
(output/'source.json').write_text(json.dumps({'sourceSHA256':hashlib.sha256((source/'SliderReference.m').read_bytes()).hexdigest(),'executableSHA256':hashlib.sha256((app/'SliderReference').read_bytes()).hexdigest()},indent=2)+'\n')
print(app)
