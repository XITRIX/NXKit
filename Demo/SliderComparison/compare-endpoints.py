#!/usr/bin/env python3
"""Plot measured track endpoints; native dots come from captured UIKit layers."""
import argparse,csv,gzip,hashlib,json,shutil
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
root=Path(__file__).resolve().parents[2]
p=argparse.ArgumentParser();p.add_argument('before',type=Path);p.add_argument('after',type=Path);p.add_argument('--output',type=Path,default=root/'docs/slider/endpoints/comparison');a=p.parse_args();a.output.mkdir(parents=True,exist_ok=True)
def rows(p):return list(csv.DictReader((p/'samples.csv').open()))
before,after=rows(a.before),rows(a.after)
for name,path in [('before',a.before),('after',a.after)]:shutil.copy2(path/'samples.csv',a.output/(name+'.csv'))
fontpath='/System/Library/Fonts/Supplemental/Arial.ttf'
font=lambda n:ImageFont.truetype(fontpath,n)
im=Image.new('RGB',(1200,1000),'white');d=ImageDraw.Draw(im)
d.text((28,18),'Slider endpoint smoothing: native UIKit vs NXKit',font=font(30),fill='black')
d.text((28,60),'Track-layer geometry in points. Native dots are measured; lines are NXKit captures.',font=font(19),fill='#555555')
colors=[('#d44836','Before'),('#137ac2','After'),('#111111','UIKit 27')]
for i,(c,t) in enumerate(colors):
 x=30+180*i;d.line((x,100,x+35,100),fill=c,width=4);d.text((x+45,88),t,font=font(20),fill=c)
for row,(variant,nativekind,label) in enumerate([('default','default','Default thumb (37pt)'),('thumb-image','custom-thumb','Custom thumb (30pt)'),('thumbless','thumbless','Thumbless')]):
 raw=gzip.decompress((root/f'docs/slider/endpoints/native/snapped/{nativekind}-light/geometry.json.gz').read_bytes());native=json.loads(raw)
 for side in range(2):
  x0=80+side*590;y0=190+row*265;w=485;h=175
  d.text((x0,y0-40),label+(' — minimum' if side==0 else ' — maximum'),font=font(20),fill='black')
  for val in [0,5,10,15,20,25]:
   y=y0+h-val/25*h;d.line((x0,y,x0+w,y),fill='#dddddd');d.text((x0-35,y-9),str(val),font=font(15),fill='#666666')
  for val in [0,.5,1,1.5,2]:
   x=x0+val/2*w;d.text((x-10,y0+h+8),str(val)+'%',font=font(15),fill='#666666')
  d.line((x0,y0,x0,y0+h,x0+w,y0+h),fill='black',width=1)
  for data,color in [(before,colors[0][0]),(after,colors[1][0])]:
   points=[]
   for r in data:
    if r['theme']!='light' or r['variant']!=variant:continue
    u=float(r['fraction']);edge=u if side==0 else 1-u
    if edge> .020001:continue
    endpoint=float(r['fillWidth']) if side==0 else float(r['trackWidth'])-float(r['fillWidth'])
    points.append((x0+edge/.02*w,y0+h-endpoint/25*h))
   d.line(sorted(points),fill=color,width=3)
  for r in native['fresh']:
   u=r['requestedProgress'];edge=u if side==0 else 1-u
   if edge>.020001:continue
   fill=r['tree']['children'][0]['children'][0]['children'][1]['model']['frame'][2]
   value=fill if side==0 else r['track'][2]-fill
   x=x0+edge/.02*w;y=y0+h-value/25*h;d.ellipse((x-3,y-3,x+3,y+3),fill='black')
d.text((28,975),'Horizontal: distance from endpoint. Vertical: filled / unfilled length from that endpoint (pt).',font=font(17),fill='#555555')
im.save(a.output/'endpoints.png')
(a.output/'manifest.json').write_text(json.dumps({n:hashlib.sha256((a.output/n).read_bytes()).hexdigest() for n in ['before.csv','after.csv','endpoints.png']},indent=2)+'\n')
print(a.output/'endpoints.png')
