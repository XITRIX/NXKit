"""Build an isolated Apple UIKit research app; no NXKit code is linked."""
from pathlib import Path
import argparse,plistlib,subprocess
p=argparse.ArgumentParser();p.add_argument('output',type=Path);a=p.parse_args();out=a.output.resolve();app=out/'Probe.app';app.mkdir(parents=True,exist_ok=True)
info={'CFBundleIdentifier':'org.nxkit.GlassSettingsProbe','CFBundleExecutable':'Probe','CFBundleName':'Glass Settings Probe','CFBundlePackageType':'APPL','CFBundleVersion':'1','CFBundleShortVersionString':'1.0','MinimumOSVersion':'27.0','UIDeviceFamily':[1,2],'UILaunchScreen':{},'UIApplicationSceneManifest':{'UIApplicationSupportsMultipleScenes':False},'UISupportedInterfaceOrientations':['UIInterfaceOrientationPortrait']}
(app/'Info.plist').write_bytes(plistlib.dumps(info));sdk=subprocess.check_output(['xcrun','--sdk','iphonesimulator','--show-sdk-path'],text=True).strip()
command=['xcrun','--sdk','iphonesimulator','clang','-target','arm64-apple-ios27.0-simulator','-isysroot',sdk,'-fobjc-arc']
for framework in ['UIKit','QuartzCore','CoreGraphics','Foundation']:command+=['-framework',framework]
command += [str(Path(__file__).with_name('Probe.m')),'-o',str(app/'Probe')];subprocess.run(command,check=True);subprocess.run(['codesign','--force','--sign','-',str(app)],check=True)
print(app)
