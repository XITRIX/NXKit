#!/usr/bin/env python3
"""Plot retained native release samples against deterministic portable replay."""
import argparse,csv,hashlib,json,shutil
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
root=Path(__file__).resolve().parents[2]
p=argparse.ArgumentParser();p.add_argument('capture',type=Path);a=p.parse_args();out=root/'docs/slider/lift-motion/release-timing';out.mkdir(parents=True,exist_ok=True)
shutil.copy2(a.capture/'curves.csv',out/'curves.csv');rows=list(csv.DictReader((out/'curves.csv').open()))
im=Image.new('RGB',(1000,590),'white');d=ImageDraw.Draw(im);font=lambda n:ImageFont.truetype('/System/Library/Fonts/Supplemental/Arial.ttf',n)
d.text((25,16),'Slider release: lowering overlaps deceleration',font=font(28),fill='black')
d.text((25,56),'Native presentation samples vs NXKit replay of the same OS touch inputs',font=font(18),fill='#555555')
for i,trace in enumerate(['velocity/velocity-200','velocity/velocity-400']):
 x0=65+i*485;y0=150;w=405;h=300
 d.text((x0,y0-45),trace.rsplit('/',1)[1],font=font(21),fill='black')
 for width in [35,40,45,50,55,60]:
  y=y0+h-(width-30)/30*h;d.line((x0,y,x0+w,y),fill='#dddddd');d.text((x0-30,y-8),str(width),font=font(14),fill='#666666')
 for ms in range(0,501,100):
  x=x0+ms/500*w;d.text((x-10,y0+h+10),str(ms),font=font(14),fill='#666666')
 d.line((x0,y0,x0,y0+h,x0+w,y0+h),fill='black')
 subset=[r for r in rows if r['trace']==trace and r['fps']=='120'];points=[(x0,y0+h-(57.35-30)/30*h)]
 for r in subset:points.append((x0+float(r['time'])/.5*w,y0+h-(float(r['width'])-30)/30*h))
 points.append((x0+w,y0+h-(37-30)/30*h));d.line(points,fill='#137ac2',width=3)
 for r in subset:
  x=x0+float(r['time'])/.5*w;y=y0+h-(float(r['nativeWidth'])-30)/30*h;d.ellipse((x-3,y-3,x+3,y+3),fill='black')
 x=x0+.4/.5*w;d.line((x,y0,x,y0+h),fill='#888888',width=1)
 d.text((x0,y0+h+40),'Milliseconds after release commit',font=font(17),fill='#555555')
d.text((25,535),'Blue: NXKit. Black dots: UIKit. Gray line: portable value retirement completes.',font=font(18),fill='#333333')
d.text((25,562),'Vertical: knob width (pt). Native time alignment subtracts the measured 16.5ms commit interval.',font=font(16),fill='#666666')
im.save(out/'comparison.png')
error=max(abs(float(r['width'])-float(r['nativeWidth'])) for r in rows)
(out/'summary.json').write_text(json.dumps({'nativeOS':'iOS27.0/24A434 Simulator','independentNativeSamples':31,'replayRefreshRates':[60,120],'maximumWidthErrorPoints':error,'releaseCommitAlignmentSeconds':.0165,'physicalDeviceVerified':False,'csvSHA256':hashlib.sha256((out/'curves.csv').read_bytes()).hexdigest()},indent=2)+'\n')
print(out/'comparison.png')
