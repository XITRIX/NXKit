"""Finite native private-lens timing oracle; no gesture synthesis or spec changes."""
from pathlib import Path
import hashlib,json,plistlib,shutil,subprocess,sys,time
source=Path(__file__).resolve().parent;device=sys.argv[1];out=Path(sys.argv[2]).resolve();out.mkdir(parents=True,exist_ok=True)
app=out/'MinimumLiftReference.app';app.mkdir(exist_ok=True)
info=plistlib.loads((source.parent/'GlassComparison/Info.plist').read_bytes());info.update(CFBundleIdentifier='org.nxkit.MinimumLiftReference',CFBundleName='Minimum Lens Timing',CFBundleExecutable='MinimumLiftReference',CADisableMinimumFrameDurationOnPhone=True);(app/'Info.plist').write_bytes(plistlib.dumps(info));shutil.copy2(source/'MinimumLiftReference.m',out/'MinimumLiftReference.m')
sdk=subprocess.check_output(['xcrun','--sdk','iphonesimulator','--show-sdk-path'],text=True).strip();subprocess.run(['xcrun','--sdk','iphonesimulator','clang','-target','arm64-apple-ios26.0-simulator','-isysroot',sdk,'-fobjc-arc','-framework','UIKit','-framework','QuartzCore','-framework','Foundation',str(out/'MinimumLiftReference.m'),'-o',str(app/'MinimumLiftReference')],check=True);subprocess.run(['codesign','--force','--sign','-',str(app)],check=True);subprocess.run(['xcrun','simctl','install',device,str(app)],check=True)
container=Path(subprocess.check_output(['xcrun','simctl','get_app_container',device,info['CFBundleIdentifier'],'data'],text=True).strip());path=container/'Documents/minimum-lift.json';path.unlink(missing_ok=True);subprocess.run(['xcrun','simctl','launch','--terminate-running-process',device,info['CFBundleIdentifier']],check=True)
deadline=time.monotonic()+15
while not path.exists():
 if time.monotonic()>deadline:raise TimeoutError('Native minimum lens trace did not complete')
 time.sleep(.05)
shutil.copy2(path,out/'minimum-lift.json');subprocess.run(['xcrun','simctl','terminate',device,info['CFBundleIdentifier']],check=True)
d=json.loads(path.read_text());summary=[]
for c in d['configuration']:
 es=[e for e in d['events']if e['id']==c['id']];up=next(e['t']for e in es if e['kind']=='call'and e['lifted']);down=next(e['t']for e in es if e['kind']=='call'and not e['lifted']);applied=next(e['t']for e in es if e['kind']=='alongside'and not e['lifted']);summary.append({'id':c['id'],'style':c['style'],'downRequestMS':(down-up)*1000,'downAlongsideMS':(applied-up)*1000,'deferredMS':(applied-down)*1000})
manifest={'device':device,'sourceSHA256':hashlib.sha256((out/'MinimumLiftReference.m').read_bytes()).hexdigest(),'traceSHA256':hashlib.sha256(path.read_bytes()).hexdigest(),'os':d['os'],'scale':d['scale'],'scope':d['scope'],'summary':summary};(out/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n');print(json.dumps(manifest))
