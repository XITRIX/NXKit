"""Build independent UIKit app and retained NX renderer without modifying a shared build.
Usage: python build-optical.py RASTER_BUILD OUTPUT [--native-only|--renderer-only]
"""
from pathlib import Path
import sys,subprocess,shlex,plistlib,shutil
build,out=map(lambda p:Path(p).resolve(),sys.argv[1:3]);out.mkdir(exist_ok=True,parents=True)
source=Path(__file__).resolve().parent
if '--renderer-only' not in sys.argv:
 app=out/'OpticalReference.app';app.mkdir(exist_ok=True)
 info=plistlib.loads((source/'Info.plist').read_bytes());info.update(CFBundleIdentifier='org.nxkit.OpticalReference',CFBundleName='Optical Reference',CFBundleExecutable='OpticalReference');(app/'Info.plist').write_bytes(plistlib.dumps(info))
 for file in (out/'backgrounds').glob('*.png'):shutil.copy2(file,app/file.name)
 sdk=subprocess.check_output(['xcrun','--sdk','iphonesimulator','--show-sdk-path'],text=True).strip()
 subprocess.run(['xcrun','--sdk','iphonesimulator','clang','-target','arm64-apple-ios26.0-simulator','-isysroot',sdk,'-fobjc-arc','-framework','UIKit','-framework','QuartzCore','-framework','Foundation',str(source/'OpticalReference.m'),'-o',str(app/'OpticalReference')],check=True)
 subprocess.run(['codesign','--force','--sign','-',str(app)],check=True)
if '--native-only' not in sys.argv:
 commands=subprocess.check_output(['ninja','-C',str(build),'-t','commands','ContainerTests'],text=True).splitlines()
 args=shlex.split(next(l for l in commands if 'GlassContainerTests.cpp.o -c ' in l));args[args.index('-o')+1]=str(out/'OpticalRenderer.o');args[-1]=str(source/'OpticalRenderer.cpp')
 # Dependencies belong to standalone output, rather than mutating shared Ninja bookkeeping.
 for flag in ['-MF','-MT']:
  if flag in args:at=args.index(flag);del args[at:at+2]
 args=[a for a in args if a not in ['-MD','-MMD']]
 subprocess.run(args,cwd=build,check=True)
 args=shlex.split(commands[-1].split(' && ')[1]);args=[str(out/'OpticalRenderer.o') if a.endswith('GlassContainerTests.cpp.o') else str(out/'libOpticalCore.a') if a=='UIKit/libXITRIXUIKit.a' else a for a in args];args[args.index('-o')+1]=str(out/'OpticalRenderer')
 selected=next((Path(a.split('=',1)[1]).resolve() for a in sys.argv if a.startswith('--core-archive=')),build/'UIKit/libXITRIXUIKit.a')
 shutil.copy2(selected,out/'libOpticalCore.a');subprocess.run(args,cwd=build,check=True)
