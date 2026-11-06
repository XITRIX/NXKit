#!/usr/bin/env python3
"""Build the independent SwiftUI List / native navigation and tab reference."""
from pathlib import Path
import hashlib,json,plistlib,subprocess,sys
out=Path(sys.argv[1]).resolve();app=out/'GlassScreenReference.app';app.mkdir(parents=True,exist_ok=True)
source=Path(__file__).with_name('GlassScreenReference.swift')
sdk=subprocess.check_output(['xcrun','--sdk','iphonesimulator','--show-sdk-path'],text=True).strip()
subprocess.run(['xcrun','--sdk','iphonesimulator','swiftc','-parse-as-library','-module-cache-path',str(out/'module-cache'),'-target','arm64-apple-ios26.0-simulator','-sdk',sdk,'-framework','SwiftUI','-framework','UIKit',str(source),'-o',str(app/'GlassScreenReference')],check=True)
info=plistlib.loads(source.with_name('Info.plist').read_bytes());info.update(CFBundleIdentifier='org.nxkit.GlassScreenReference',CFBundleExecutable='GlassScreenReference',CFBundleName='Glass Screen',UIDeviceFamily=[1,2])
info['UIApplicationSceneManifest']={'UIApplicationSupportsMultipleScenes':False,'UISceneConfigurations':{'UIWindowSceneSessionRoleApplication':[{'UISceneConfigurationName':'Default','UISceneDelegateClass':'GlassScreenReference.ReferenceSceneDelegate'}]}}
(app/'Info.plist').write_bytes(plistlib.dumps(info));subprocess.run(['codesign','--force','--sign','-',str(app)],check=True)
(out/'build.json').write_text(json.dumps({'sourceSHA256':hashlib.sha256(source.read_bytes()).hexdigest(),'binarySHA256':hashlib.sha256((app/'GlassScreenReference').read_bytes()).hexdigest(),'sdk':sdk},indent=2)+'\n');print(app)
