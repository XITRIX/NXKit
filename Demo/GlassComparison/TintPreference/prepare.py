#!/usr/bin/env python3
"""Generate identical native/portable scenes; never alter global preferences."""
from pathlib import Path
import json, plistlib, subprocess, sys, hashlib
from PIL import Image, ImageDraw
out=Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
here=Path(__file__).resolve().parent
app=out/'GlassTintMatrix.app';app.mkdir(exist_ok=True)
cases=[]
palette=[(0,0,0),(128,128,128),(255,255,255),(255,0,0),(0,255,0),(0,0,255)]
for page in range(3):
 image=Image.new('RGB',(768,1408));draw=ImageDraw.Draw(image)
 probes=[]
 for row in range(8):
  for col in range(3):
   x,y=col*128,row*88
   color=palette[page*3+col] if page<2 else ((24,48,80) if col==0 else (180,120,40) if col==1 else (128,128,128))
   draw.rectangle((x*2,y*2,(x+128)*2-1,(y+88)*2-1),fill=color)
   if page==2:
    for px in range(x*2,(x+128)*2,8):
     draw.rectangle((px,y*2,px+3,(y+88)*2-1),fill=(245,245,245))
    draw.text((x*2+20,y*2+70),'Glass text 27',fill=(0,0,0),stroke_width=1)
   small=row<4;w,h=(44,44) if small else (112,64)
   probes.append(dict(frame=[x+(128-w)/2,y+(88-h)/2,w,h],clear=bool(row%2),tinted=bool((row%4)//2),column=col,row=row))
 name=f'background-{page}.png';image.save(app/name)
 for dark in [False,True]:cases.append(dict(id=f'{page}-'+('dark' if dark else 'light'),dark=dark,background=name,probes=probes))
(app/'cases.json').write_text(json.dumps(cases,indent=2)+'\n')
info=plistlib.loads((here.parent/'Info.plist').read_bytes());info.update(CFBundleIdentifier='org.nxkit.GlassTintMatrix',CFBundleExecutable='GlassTintMatrix',CFBundleName='Glass Tint Matrix',MinimumOSVersion='27.0');(app/'Info.plist').write_bytes(plistlib.dumps(info))
sdk=subprocess.check_output(['xcrun','--sdk','iphonesimulator','--show-sdk-path'],text=True).strip()
subprocess.run(['xcrun','clang','-target','arm64-apple-ios27.0-simulator','-isysroot',sdk,'-fobjc-arc','-framework','UIKit','-framework','QuartzCore','-framework','CoreGraphics',str(here/'NativeReference.m'),'-o',str(app/'GlassTintMatrix')],check=True)
subprocess.run(['codesign','--force','--sign','-',str(app)],check=True)
(out/'source-manifest.json').write_text(json.dumps({p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in [here/'NativeReference.m',here/'prepare.py',app/'cases.json']},indent=2)+'\n')
print(app)
