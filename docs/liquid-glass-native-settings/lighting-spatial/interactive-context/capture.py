from pathlib import Path
import subprocess,os,time,uuid,json
p=Path('/private/tmp/nxkit-rim-context');device='31F29D37-A362-42F2-BAE5-5214B1F20486';bundle='org.nxkit.NativeGlassPassProbe'
container=Path(subprocess.check_output(['xcrun','simctl','get_app_container',device,bundle,'data'],text=True).strip())
for interactive in [False,True]:
 run=str(uuid.uuid4());env=os.environ.copy();env['SIMCTL_CHILD_PROBE_RUN_ID']=run;env['SIMCTL_CHILD_PROBE_TINT_AMOUNT']='.5'
 if interactive:env['SIMCTL_CHILD_PROBE_INTERACTIVE']='1'
 subprocess.run(['xcrun','simctl','launch','--terminate-running-process',device,bundle],env=env,check=True)
 deadline=time.monotonic()+25
 while time.monotonic()<deadline:
  try:
   d=json.loads((container/'Documents/sizes.json').read_text())
   if d['runID']==run:break
  except (FileNotFoundError,ValueError):pass
  time.sleep(.1)
 else:raise RuntimeError('no acknowledgement')
 (p/('interactive'if interactive else'normal')).with_suffix('.json').write_text(json.dumps(d,indent=2))
 print('captured',interactive,flush=True)
subprocess.run(['xcrun','simctl','terminate',device,bundle],check=True)
