"""Two native processes resolve literal and semantic tint matrices in both styles."""
from pathlib import Path
import argparse,subprocess,os,uuid,json,time,gzip
p=argparse.ArgumentParser();p.add_argument('--device',required=True);p.add_argument('--output',required=True,type=Path);a=p.parse_args();a.output.mkdir(parents=True,exist_ok=True);bundle='org.nxkit.NativeGlassPassProbe';container=Path(subprocess.check_output(['xcrun','simctl','get_app_container',a.device,bundle,'data'],text=True).strip());source=container/'Documents/palette.json'
for light in [False,True]:
 run=str(uuid.uuid4());env=os.environ.copy();env['SIMCTL_CHILD_PROBE_RUN_ID']=run;env['SIMCTL_CHILD_PROBE_TINT_AMOUNT']='.5'
 if light:env['SIMCTL_CHILD_PROBE_LIGHT']='1'
 subprocess.run(['xcrun','simctl','launch','--terminate-running-process',a.device,bundle],env=env,stdout=subprocess.DEVNULL,check=True,timeout=30);deadline=time.monotonic()+30
 while time.monotonic()<deadline:
  try:
   d=json.loads(source.read_text())
   if d['runID']==run:break
  except(ValueError,FileNotFoundError):pass
  time.sleep(.2)
 else:raise RuntimeError('No fresh palette capture')
 def count(x):
  if isinstance(x,dict):return int(x.get('name')=='glassBackground'and'archiveBase64'in x)+sum(count(v)for v in x.values())
  if isinstance(x,list):return sum(count(v)for v in x)
  return 0
 assert count(d['layerTree'])==len(d['surfaces']),count(d['layerTree'])
 name='light'if light else'dark';(a.output/(name+'.json.gz')).write_bytes(gzip.compress(json.dumps(d).encode()));print(name,len(d['surfaces']),flush=True)
