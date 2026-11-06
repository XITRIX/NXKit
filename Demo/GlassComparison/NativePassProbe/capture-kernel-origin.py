"""Native blur-grid phase oracle with source fixed in window coordinates."""
import subprocess,json,os,uuid,time
from pathlib import Path
import argparse
p=argparse.ArgumentParser();p.add_argument('--device',required=True);p.add_argument('--output',required=True,type=Path);a=p.parse_args()
D=a.device;B='org.nxkit.NativeGlassPassProbe';out=a.output;out.mkdir(parents=True,exist_ok=True)
c=Path(subprocess.check_output(['xcrun','simctl','get_app_container',D,B,'data'],text=True).strip())/'Documents/pass.json'
for x in [0,24,25]:
 run=str(uuid.uuid4());env=os.environ|{'SIMCTL_CHILD_PASS_RUN_ID':run,'SIMCTL_CHILD_PASS_TINT_AMOUNT':'1','SIMCTL_CHILD_PASS_CHANGES':'{"inputBlurRadius":5}','SIMCTL_CHILD_PASS_GLASS_X':str(x)}
 subprocess.run(['xcrun','simctl','launch','--terminate-running-process',D,B],env=env,check=True,capture_output=True);deadline=time.monotonic()+25
 while time.monotonic()<deadline:
  try:
   d=json.loads(c.read_text())
   if d['runID']==run:break
  except (FileNotFoundError,ValueError):pass
  time.sleep(.1)
 else:raise RuntimeError('Missing fresh capture')
 assert d['glassX']==str(x) and len(d['mutations'])==8
 p=out/f'x-{x}';p.with_suffix('.json').write_text(json.dumps(d,indent=2)+'\n');subprocess.run(['xcrun','simctl','io',D,'screenshot',str(p.with_suffix('.png'))],check=True,capture_output=True);print(x,flush=True)
