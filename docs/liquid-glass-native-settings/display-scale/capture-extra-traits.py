from pathlib import Path
import subprocess,os,time,uuid,json,gzip
p=Path('/private/tmp/nxkit-rim-context/trait-cases');p.mkdir(exist_ok=True);device='F0FF4E71-DF79-41DD-8862-DC161CB409E0';bundle='org.nxkit.NativeGlassPassProbe'
container=Path(subprocess.check_output(['xcrun','simctl','get_app_container',device,bundle,'data'],text=True).strip())
for name,scale,idiom in [('pad1','1',None),('pad2p5','2.5',None)]:
 run=str(uuid.uuid4());env=os.environ.copy();env['SIMCTL_CHILD_PROBE_RUN_ID']=run;env['SIMCTL_CHILD_PROBE_TINT_AMOUNT']='.5';env['SIMCTL_CHILD_PROBE_ALL_STYLES']='1';env['SIMCTL_CHILD_PROBE_SINGLE_COLOR']='1';env['SIMCTL_CHILD_PROBE_INTERACTIVE']='1'
 if scale:env['SIMCTL_CHILD_PROBE_OVERRIDE_SCALE']=scale
 if idiom:env['SIMCTL_CHILD_PROBE_OVERRIDE_IDIOM']=idiom
 subprocess.run(['xcrun','simctl','launch','--terminate-running-process',device,bundle],env=env,check=True,stdout=subprocess.DEVNULL)
 deadline=time.monotonic()+25
 while time.monotonic()<deadline:
  try:
   d=json.loads((container/'Documents/sizes.json').read_text())
   if d['runID']==run:break
  except(FileNotFoundError,ValueError):pass
  time.sleep(.1)
 else:raise RuntimeError('no acknowledgement')
 assert len(d['surfaces'])==4
 (p/(name+'.json.gz')).write_bytes(gzip.compress(json.dumps(d).encode()))
 print('captured',name,d['scale'],d['traitScale'],d['traitIdiom'],flush=True)
subprocess.run(['xcrun','simctl','terminate',device,bundle],check=True)
