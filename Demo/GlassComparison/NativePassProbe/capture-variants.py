"""Bounded extra clear+tint and explicit accent-color captures."""
from pathlib import Path
import argparse,subprocess,os,uuid,json,time,gzip
p=argparse.ArgumentParser();p.add_argument('--device',required=True);p.add_argument('--output',required=True,type=Path);p.add_argument('--variant',choices=['allStyles','colors'],required=True);a=p.parse_args();a.output.mkdir(parents=True,exist_ok=True);bundle='org.nxkit.NativeGlassPassProbe';container=Path(subprocess.check_output(['xcrun','simctl','get_app_container',a.device,bundle,'data'],text=True).strip());source=container/'Documents/sizes.json'
for light in [False,True]:
 cases=[('0','0'),('quarter','.25'),('half','.5'),('threequarter','.75'),('1','1')]if a.variant=='allStyles'else[('red','1,0,0'),('green','0,1,0')]
 for label,value in cases:
  run=str(uuid.uuid4());env=os.environ.copy();env['SIMCTL_CHILD_PROBE_RUN_ID']=run
  if a.variant=='allStyles':env['SIMCTL_CHILD_PROBE_ALL_STYLES']='1';env['SIMCTL_CHILD_PROBE_TINT_AMOUNT']=value
  else:env['SIMCTL_CHILD_PROBE_SINGLE_COLOR']='1';env['SIMCTL_CHILD_PROBE_TINT_RGB']=value;env['SIMCTL_CHILD_PROBE_TINT_AMOUNT']='.5'
  if light:env['SIMCTL_CHILD_PROBE_LIGHT']='1'
  subprocess.run(['xcrun','simctl','launch','--terminate-running-process',a.device,bundle],env=env,stdout=subprocess.DEVNULL,check=True,timeout=30);deadline=time.monotonic()+25
  while time.monotonic()<deadline:
   try:
    d=json.loads(source.read_text())
    if d['runID']==run:break
   except(ValueError,FileNotFoundError):pass
   time.sleep(.2)
  else:raise RuntimeError('No fresh variant capture')
  def count(x):
   if isinstance(x,dict):return int(x.get('name')=='glassBackground'and'archiveBase64'in x)+sum(count(v)for v in x.values())
   if isinstance(x,list):return sum(count(v)for v in x)
   return 0
  expected=56 if a.variant=='allStyles'else 3;assert len(d['surfaces'])==expected and count(d['layerTree'])==expected
  name=('light'if light else'dark')+'-'+label;(a.output/(name+'.json.gz')).write_bytes(gzip.compress(json.dumps(d).encode()));print(name,flush=True)
