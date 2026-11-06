"""Nonprimary tint matrix captures, with an explicit process-local appearance."""
from pathlib import Path
import argparse,subprocess,os,uuid,json,time,gzip
p=argparse.ArgumentParser();p.add_argument('--device',required=True);p.add_argument('--output',required=True,type=Path);a=p.parse_args();a.output.mkdir(parents=True,exist_ok=True);bundle='org.nxkit.NativeGlassPassProbe';container=Path(subprocess.check_output(['xcrun','simctl','get_app_container',a.device,bundle,'data'],text=True).strip());source=container/'Documents/sizes.json'
cases=[('light-coral',True,[1,69/255,58/255]),('light-grayhalf',True,[.5,.5,.5]),('light-bluehalf',True,[0,.5,1]),('light-custom',True,[.2,.4,.6]),('light-orange',True,[1,.5,0]),('dark-coral',False,[1,69/255,58/255]),('dark-grayhalf',False,[.5,.5,.5])]
for name,light,rgb in cases:
 run=str(uuid.uuid4());env=os.environ.copy();env['SIMCTL_CHILD_PROBE_RUN_ID']=run;env['SIMCTL_CHILD_PROBE_SINGLE_COLOR']='1';env['SIMCTL_CHILD_PROBE_TINT_RGB']=','.join(map(str,rgb));env['SIMCTL_CHILD_PROBE_TINT_AMOUNT']='.5'
 if light:env['SIMCTL_CHILD_PROBE_LIGHT']='1'
 subprocess.run(['xcrun','simctl','launch','--terminate-running-process',a.device,bundle],env=env,stdout=subprocess.DEVNULL,check=True,timeout=30);deadline=time.monotonic()+25
 while time.monotonic()<deadline:
  try:
   d=json.loads(source.read_text())
   if d['runID']==run:break
  except(ValueError,FileNotFoundError):pass
  time.sleep(.2)
 else:raise RuntimeError('No fresh curve capture')
 d['explicitTintRGB']=rgb;d['explicitLight']=light;assert len(d['surfaces'])==3
 (a.output/(name+'.json.gz')).write_bytes(gzip.compress(json.dumps(d).encode()));print(name,flush=True)
