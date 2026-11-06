#!/usr/bin/env python3
"""Capture all three process-local preferences on a booted iOS 27 simulator."""
from pathlib import Path
import json, os, subprocess, sys, time
from PIL import Image
udid=sys.argv[1];out=Path(sys.argv[2]).resolve();out.mkdir(parents=True,exist_ok=True)
bundle='org.nxkit.GlassTintMatrix'
def run(*a,**kw):return subprocess.run(['xcrun','simctl',*a],check=True,**kw)
container=Path(subprocess.check_output(['xcrun','simctl','get_app_container',udid,bundle,'data'],text=True).strip())/'Documents'
for amount in ['0','0.25','0.5','0.75','1']:
 for p in container.glob(amount+'-*'):p.unlink()
 ack=container/'ack.txt'
 if ack.exists():ack.unlink()
 env={**os.environ,'SIMCTL_CHILD_MATRIX_AMOUNT':amount}
 run('launch','--terminate-running-process',udid,bundle,env=env,capture_output=True)
 expected=[f'{amount}-{page}-{theme}' for page in range(3) for theme in ['light','dark']]
 for name in expected:
  deadline=time.monotonic()+60
  while not (container/(name+'.json')).exists():
   if time.monotonic()>deadline:raise TimeoutError(name)
   time.sleep(.15)
  meta=json.loads((container/(name+'.json')).read_text())
  if meta['amount']!=float(amount):raise ValueError(f'Native effective preference mismatch: {meta}')
  run('io',udid,'screenshot',str(out/'framebuffer.png'),capture_output=True)
  image=Image.open(out/'framebuffer.png').convert('RGB');scale=meta['scale'];x,y=[round(v*scale) for v in meta['stageOrigin']]
  image.crop((x,y,x+round(384*scale),y+round(704*scale))).save(out/(name+'.png'))
  meta['capture']='simctl framebuffer';(out/(name+'.json')).write_text(json.dumps(meta,indent=2)+'\n')
  (container/'ack.txt').write_text(name)
 print('Captured validated amount',amount,flush=True)
run('terminate',udid,bundle)
