#!/usr/bin/env python3
"""Capture six states of the independent SwiftUI List screen on a simulator."""
from pathlib import Path
import subprocess,sys,json,time,shutil
user_device=sys.argv[1];out=Path(sys.argv[2]).resolve();out.mkdir(parents=True,exist_ok=True)
container=Path(subprocess.check_output(['xcrun','simctl','get_app_container',user_device,'org.nxkit.GlassScreenReference','data'],text=True).strip())/'Documents'
cases=[]
for dark in [False,True]:
 for mode in ['initial','blue','details']:
  case={'id':mode+('-dark' if dark else '-light'),'dark':dark,'tab':2 if mode=='details' else 0,'accent':1 if mode=='details' else 0,'backdrop':1 if mode=='blue' else 0,'details':mode=='details','added':2 if mode=='details' else 0,'reversed':mode=='details','scroll':160 if mode=='blue' else (300 if mode=='details' else 0)}
  for suffix in ['.json','.png']:
   path=container/(case['id']+suffix)
   if path.exists():path.unlink()
  request=container/'request.tmp';request.write_text(json.dumps(case));request.replace(container/'request.json')
  deadline=time.monotonic()+30
  while not (container/(case['id']+'.json')).exists():
   if time.monotonic()>deadline:raise TimeoutError(case['id'])
   time.sleep(.15)
  for suffix in ['.json','.png']:shutil.copy2(container/(case['id']+suffix),out/(case['id']+suffix))
  cases.append(case);print(case['id'],flush=True)
(out/'cases.json').write_text(json.dumps(cases,indent=2)+'\n')
