"""Verify fresh versus retained sizing, with and without animation.
Usage: probe-resize.py DEVICE_ID native-app-path output-directory [--dark]
"""
import json, subprocess, time, sys
from pathlib import Path
device, app, output = sys.argv[1:4]
flags = sys.argv[4:]
p=Path(output).resolve();p.mkdir(parents=True, exist_ok=True)
bundle='org.nxkit.NativeGlassReference'
def sim(*a):return subprocess.check_output(['xcrun','simctl',*a],text=True).strip()
sim('install',device,str(Path(app).resolve()))
doc=Path(sim('get_app_container',device,bundle,'data'))/'Documents'
sim('launch','--terminate-running-process',device,bundle,*flags)
time.sleep(2)
def capture(name):
 sim('io',device,'screenshot',str(p/(name+'.png')))
 metadata=json.loads((doc/'capture.json').read_text());metadata['arguments']=metadata['arguments'][1:]
 (p/(name+'.json')).write_text(json.dumps(metadata,indent=2)+'\n');print(name,flush=True)
capture('fresh')
for i,(size,animate) in enumerate([(1,False),(0,False),(1,False),(0,True),(1,True)]):
 request={'id':str(i),'size':size,'tint':'none','animate':animate}
 (doc/'request.tmp').write_text(json.dumps(request));(doc/'request.tmp').replace(doc/'request.json')
 for _ in range(200):
  if json.loads((doc/'capture.json').read_text()).get('id')==str(i):break
  time.sleep(.05)
 else:raise RuntimeError('request failed')
 capture(f'{i}-{size}-{animate}')
