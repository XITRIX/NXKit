"""Compare native framebuffer crops to before/after retained raster scenes.
No image registration, geometry fitting, or color fitting is performed.
Run with Apple /usr/bin/python3 (Pillow/numpy) on this development machine.
"""
import argparse,json
from pathlib import Path
import numpy as np
from PIL import Image,ImageDraw

p=argparse.ArgumentParser()
for name in ('native','before','after','output'):p.add_argument('--'+name,type=Path,required=True)
a=p.parse_args();a.output.mkdir(parents=True,exist_ok=True)
rows=[];panels=[]
for path in sorted(a.native.glob('*.json')):
 if path.name=='series.json':continue
 d=json.loads(path.read_text());name=path.stem
 before=a.before/(name+'.ppm');after=a.after/(name+'.ppm')
 if not before.exists() or not after.exists():continue
 reference=Image.open(a.native/(name+'.png')).convert('RGB')
 scale=d['scale']
 x,y,w,h=next(v['frame'] for v in d['hierarchy'] if v['identifier']=='manual-lens')
 yy,xx=np.indices((reference.height,reference.width));px=(xx+.5)/scale-x;py=(yy+.5)/scale-y
 r=min(w,h)/2;qx=np.abs(px-w/2)-(w/2-r);qy=np.abs(py-h/2)-(h/2-r)
 sdf=np.minimum(np.maximum(qx,qy),0)+np.hypot(np.maximum(qx,0),np.maximum(qy,0))-r
 masks={'body':sdf<0,'edge':(sdf<0)&(sdf>-12),'center':sdf<-12,'exterior':(sdf>=0)&(sdf<8)}
 images=[reference,Image.open(before).convert('RGB'),Image.open(after).convert('RGB')]
 native=np.asarray(reference).astype(float);row={'id':name}
 for label,im in zip(('before','after'),images[1:]):
  if im.size!=reference.size:raise ValueError((name,im.size,reference.size))
  error=np.abs(np.asarray(im).astype(float)-native)
  row[label]={key:float(error[mask].mean()) for key,mask in masks.items()}
 rows.append(row)
 crop=(int((x-10)*scale),int((y-12)*scale),int((x+w+10)*scale),int((y+h+12)*scale))
 panel=Image.new('RGB',(3*300,196),'#e7e7e7');draw=ImageDraw.Draw(panel)
 draw.text((4,4),name,fill='black')
 for i,im in enumerate(images):
  tile=im.crop(crop);tile.thumbnail((280,160));panel.paste(tile,(i*300+4,25))
 panels.append(panel)
(a.output/'metrics.json').write_text(json.dumps(rows,indent=2)+'\n')
sheet=Image.new('RGB',(900,24+196*len(panels)),'#e7e7e7');draw=ImageDraw.Draw(sheet)
for i,title in enumerate(('UIKit','Before','Native settings')):draw.text((i*300+4,4),title,fill='black')
for i,panel in enumerate(panels):sheet.paste(panel,(0,24+i*196))
sheet.save(a.output/'comparison.png')
print(json.dumps({'cases':len(rows),'mean':{label:{region:float(np.mean([r[label][region] for r in rows])) for region in masks} for label in ('before','after')}},indent=2))
