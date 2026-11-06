"""Measure signed-distance interpolation without face/refraction confounders."""
from pathlib import Path
import subprocess,os,time,uuid,json,argparse
p=argparse.ArgumentParser();p.add_argument('--device',required=True);p.add_argument('--output',required=True,type=Path);a=p.parse_args();a.output.mkdir(parents=True,exist_ok=True);bundle='org.nxkit.NativeGlassPassProbe';container=Path(subprocess.check_output(['xcrun','simctl','get_app_container',a.device,bundle,'data'],text=True).strip());source=container/'Documents/pass.json'
for name,changes in [('zero',{'inputBlurRadius':0}),('one',{'inputBlurRadius':5}),('ramp',{'inputBlurRadius':5,'inputBlurDistance0':-32,'inputBlurDistance1':-24,'inputBlurDistance2':-8,'inputBlurDistance3':0,'inputBlurOpacity0':0,'inputBlurOpacity1':1,'inputBlurOpacity2':0,'inputBlurOpacity3':1})]:
 run=str(uuid.uuid4());env=os.environ.copy();env.update(SIMCTL_CHILD_PASS_RUN_ID=run,SIMCTL_CHILD_PASS_TINT_AMOUNT='.5',SIMCTL_CHILD_PASS_CHANGES=json.dumps(changes));subprocess.run(['xcrun','simctl','launch','--terminate-running-process',a.device,bundle],env=env,stdout=subprocess.DEVNULL,check=True,timeout=30);deadline=time.monotonic()+25
 while time.monotonic()<deadline:
  try:
   d=json.loads(source.read_text())
   if d['runID']==run:break
  except(ValueError,FileNotFoundError):pass
  time.sleep(.1)
 else:raise RuntimeError('No fresh capture')
 assert len(d['mutations'])==8 and not any('error'in v for m in d['mutations']for v in m.values()),d
 target=a.output/name;target.with_suffix('.json').write_text(json.dumps(d,indent=2)+'\n');subprocess.run(['xcrun','simctl','io',a.device,'screenshot',str(target.with_suffix('.png'))],check=True,capture_output=True,timeout=20);print(name,flush=True)
