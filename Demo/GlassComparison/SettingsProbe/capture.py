"""Capture six process-local appearance/tint cases from the installed probe."""
from pathlib import Path
import subprocess,os,time,uuid,json,sys,argparse
from decode import extract
p=argparse.ArgumentParser();p.add_argument('--device',required=True);p.add_argument('--output',required=True,type=Path);a=p.parse_args();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True);device=a.device;bundle='org.nxkit.GlassSettingsProbe'
container=Path(subprocess.check_output(['xcrun','simctl','get_app_container',device,bundle,'data'],text=True).strip());source=container/'Documents/probe.json'
for light in (False,True):
 for amount in ('system','0','1'):
  run=str(uuid.uuid4());env=os.environ.copy();env['SIMCTL_CHILD_PROBE_RUN_ID']=run
  for key in ['SIMCTL_CHILD_PROBE_LIGHT','SIMCTL_CHILD_PROBE_TINT_AMOUNT']:env.pop(key,None)
  if light:env['SIMCTL_CHILD_PROBE_LIGHT']='1'
  if amount!='system':env['SIMCTL_CHILD_PROBE_TINT_AMOUNT']=amount
  result=subprocess.run(['xcrun','simctl','launch','--terminate-running-process',device,bundle],env=env,capture_output=True,text=True,check=True,timeout=30)
  deadline=time.monotonic()+25
  while time.monotonic()<deadline:
   try:
    data=json.loads(source.read_text())
    if data.get('environment',{}).get('runID')==run:break
   except (ValueError,FileNotFoundError):pass
   time.sleep(.2)
  else:raise RuntimeError('No fresh capture '+run)
  target=out/(('light'if light else'dark')+'-'+amount+'.json');target.write_text(json.dumps(data,indent=2)+'\n');target.with_suffix('.profiles.json').write_text(json.dumps(extract(target),indent=2)+'\n')
  assert len(data['mutationChecks'])==3 and all(m.get('restored')==m.get('before') and m.get('after')==m.get('before')+1 for m in data['mutationChecks']),data['mutationChecks']
  print(target.name,'effective tint',data['environment']['effectiveTint'],'mutation checks',data['mutationChecks'],flush=True)
