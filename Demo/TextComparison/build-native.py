#!/usr/bin/env python3
"""Build a native UILabel fixture using the repository's identical Roboto test font."""
from pathlib import Path
import hashlib,json,plistlib,subprocess,sys,shutil
root=Path(__file__).resolve().parents[2];out=Path(sys.argv[1]).resolve();app=out/'TextReference.app';app.mkdir(parents=True,exist_ok=True)
source=Path(__file__).with_name('Reference.swift');font=root/'Submodules/skia/resources/fonts/Roboto-Regular.ttf'
sdk=subprocess.check_output(['xcrun','--sdk','iphonesimulator','--show-sdk-path'],text=True).strip()
subprocess.run(['xcrun','--sdk','iphonesimulator','swiftc','-parse-as-library','-module-cache-path',str(out/'module-cache'),'-target','arm64-apple-ios26.0-simulator','-sdk',sdk,str(source),'-o',str(app/'TextReference')],check=True)
info=plistlib.loads((root/'Demo/GlassComparison/Info.plist').read_bytes());info.update(CFBundleIdentifier='org.nxkit.TextReference',CFBundleExecutable='TextReference',CFBundleName='Text Reference',NXKitTextFixtureSourceSHA256=hashlib.sha256(source.read_bytes()).hexdigest())
info['UIApplicationSceneManifest']={'UIApplicationSupportsMultipleScenes':False,'UISceneConfigurations':{'UIWindowSceneSessionRoleApplication':[{'UISceneConfigurationName':'Default','UISceneDelegateClass':'TextReference.Scene'}]}}
(app/'Info.plist').write_bytes(plistlib.dumps(info));shutil.copy2(font,app/font.name)
subprocess.run(['codesign','--force','--sign','-',str(app)],check=True)
(out/'build.json').write_text(json.dumps({'sourceSHA256':hashlib.sha256(source.read_bytes()).hexdigest(),'fontSHA256':hashlib.sha256(font.read_bytes()).hexdigest(),'binarySHA256':hashlib.sha256((app/'TextReference').read_bytes()).hexdigest(),'sdk':sdk},indent=2)+'\n');print(app)
