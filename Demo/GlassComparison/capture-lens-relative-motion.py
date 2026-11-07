"""Run bounded native own/ancestor motion probes; private API stays in the oracle."""
from pathlib import Path
import hashlib,json,os,plistlib,shutil,subprocess,sys,time
device=sys.argv[1];output=Path(sys.argv[2]).resolve();output.mkdir(parents=True,exist_ok=True)
source=Path(__file__).with_name('LensRelativeMotionReference.m')
app=output/'LensRelativeMotion.app';app.mkdir(exist_ok=True)
info=plistlib.loads(Path(__file__).with_name('Info.plist').read_bytes())
info.update(CFBundleIdentifier='org.nxkit.LensRelativeMotion',CFBundleName='Lens Relative Motion',CFBundleExecutable='LensRelativeMotion',CADisableMinimumFrameDurationOnPhone=True)
(app/'Info.plist').write_bytes(plistlib.dumps(info));shutil.copy2(source,output/source.name)
sdk=subprocess.check_output(['xcrun','--sdk','iphonesimulator','--show-sdk-path'],text=True).strip()
subprocess.run(['xcrun','--sdk','iphonesimulator','clang','-target','arm64-apple-ios26.0-simulator','-isysroot',sdk,'-fobjc-arc','-framework','UIKit','-framework','QuartzCore','-framework','Foundation','-framework','CoreGraphics',str(source.resolve()),'-o',str(app/'LensRelativeMotion')],check=True)
subprocess.run(['codesign','--force','--sign','-',str(app)],check=True)
subprocess.run(['xcrun','simctl','install',device,str(app)],check=True)
container=Path(subprocess.check_output(['xcrun','simctl','get_app_container',device,info['CFBundleIdentifier'],'data'],text=True).strip())
path=container/'Documents/relative-motion.json';path.unlink(missing_ok=True)
try:
    env=os.environ.copy();mode=sys.argv[3] if len(sys.argv)>3 else 'lifted';env['SIMCTL_CHILD_RELATIVE_MOTION_LIFTED']={'resting':'0','lowering':'lowering','lifted':'1'}[mode]
    subprocess.run(['xcrun','simctl','launch','--terminate-running-process',device,info['CFBundleIdentifier']],env=env,check=True,timeout=30)
    deadline=time.monotonic()+30
    while not path.exists():
        assert time.monotonic()<deadline,'Native motion capture timeout';time.sleep(.1)
    shutil.copy2(path,output/'relative-motion.json');data=json.loads(path.read_text());summary=[]
    for style in [0,1]:
        for name in dict.fromkeys(f['name'] for f in data['frames']):
            rows=[f for f in data['frames'] if f['style']==style and f['name']==name and .8<f['t']<3.5]
            error=max(max(abs(f['transform'][i]-v) for i,v in enumerate([1,0,0,1,0,0])) for f in rows)
            positions=[f['integratorPosition'][0] for f in rows if isinstance(f['integratorPosition'],list)]
            summary.append(dict(name=name,style=style,samples=len(rows),maxElasticAffineError=error,integratorXRange=max(positions)-min(positions) if positions else None,flexSources=rows[-1]['flexSources'],activation=rows[-1]['activation']))
    manifest=dict(os=data['os'],scale=data['scale'],mode=mode,lifted=data['lifted'],device=device,sourceSHA256=hashlib.sha256(source.read_bytes()).hexdigest(),resultSHA256=hashlib.sha256(path.read_bytes()).hexdigest(),summary=summary)
    (output/'summary.json').write_text(json.dumps(manifest,indent=2)+'\n');print(json.dumps(manifest,indent=2))
finally:subprocess.run(['xcrun','simctl','terminate',device,info['CFBundleIdentifier']],check=False)
