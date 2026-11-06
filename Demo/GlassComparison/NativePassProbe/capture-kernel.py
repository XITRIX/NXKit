"""Eight fresh-process isolated native blur captures; no persistent settings."""
from pathlib import Path
import subprocess,os,time,uuid,json,argparse
p=argparse.ArgumentParser();p.add_argument('--device',required=True);p.add_argument('--output',required=True,type=Path);a=p.parse_args();a.output.mkdir(parents=True,exist_ok=True);bundle='org.nxkit.NativeGlassPassProbe';container=Path(subprocess.check_output(['xcrun','simctl','get_app_container',a.device,bundle,'data'],text=True).strip());source=container/'Documents/pass.json'
for tint,label in [('.5','half'),('1','one')]:
 for radius in [0,1,2,5]:
  run=str(uuid.uuid4());env=os.environ.copy();env['SIMCTL_CHILD_PASS_RUN_ID']=run;env['SIMCTL_CHILD_PASS_TINT_AMOUNT']=tint;env['SIMCTL_CHILD_PASS_CHANGES']=json.dumps({'inputBlurRadius':radius})
  subprocess.run(['xcrun','simctl','launch','--terminate-running-process',a.device,bundle],env=env,stdout=subprocess.DEVNULL,check=True,timeout=30);deadline=time.monotonic()+25
  while time.monotonic()<deadline:
   try:
    d=json.loads(source.read_text())
    if d['runID']==run:break
   except(ValueError,FileNotFoundError):pass
   time.sleep(.1)
  else:raise RuntimeError('No fresh kernel capture')
  assert len(d['mutations'])==8 and not any('error'in v for m in d['mutations']for v in m.values()),d
  name=f'{label}-radius-{radius}';target=a.output/name;target.with_suffix('.json').write_text(json.dumps(d,indent=2)+'\n');subprocess.run(['xcrun','simctl','io',a.device,'screenshot',str(target.with_suffix('.png'))],check=True,capture_output=True,timeout=20);print(name,flush=True)
