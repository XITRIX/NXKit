#!/usr/bin/env python3
from pathlib import Path
import subprocess,sys,time,shutil,json
container=Path(subprocess.check_output(['xcrun','simctl','get_app_container',sys.argv[1],'org.nxkit.TextReference','data'],text=True).strip())/'Documents'
out=Path(sys.argv[2]);out.mkdir(parents=True,exist_ok=True)
expected=json.loads((out.parent/'build.json').read_text())['sourceSHA256']
for scale in [1,2,3]:
 for theme in ['light','dark']:
  for suffix in ['.json','.png','-baseline.png']:
   name=f'{scale}x-{theme}{suffix}';deadline=time.monotonic()+30
   while not (container/name).exists() or (suffix=='.json' and json.loads((container/name).read_text()).get('sourceSHA256')!=expected):
    if time.monotonic()>deadline:raise TimeoutError(name)
    time.sleep(.2)
   shutil.copy2(container/name,out/name)
print(out)
