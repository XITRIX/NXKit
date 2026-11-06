"""Compare shared gallery checkpoints without registration or exposure changes.
Usage: compare-gallery.py NATIVE_SERIES RASTERS OUTPUT [BEFORE_RASTERS]
Requires Pillow/numpy. Metrics describe appearance; they do not assert pixel parity.
"""
import json
from pathlib import Path
import sys
import numpy as np
from PIL import Image,ImageDraw
native,rasters,output=map(Path,sys.argv[1:4]);before=Path(sys.argv[4]) if len(sys.argv)>4 else None
output.mkdir(parents=True,exist_ok=True)
rows=json.loads((native/'series.json').read_text()); results=[]; pictures={}
xx,yy=np.meshgrid((np.arange(1440)+.5)/2,(np.arange(880)+.5)/2)
for row in rows:
 name=row['name']; meta=json.loads((native/name/'capture.json').read_text()); scale=meta['scale']
 if scale!=2 or meta['width']!=720: raise RuntimeError('Requires an unscaled 2x stage')
 x,y=round(meta['x']*scale),round(meta['y']*scale)
 a=Image.open(native/name/'screen.png').convert('RGB').crop((x,y,x+1440,y+880))
 b=Image.open(rasters/(name+'.ppm')).convert('RGB')
 if a.size!=b.size: raise RuntimeError('Mismatched capture sizes')
 rmeta=json.loads((rasters/(name+'.ppm.json')).read_text())
 if len(rmeta['elements'])!=len(meta['elements']) or any(abs(a[k]-b[k])>.001 for a,b in zip(rmeta['elements'],meta['elements']) for k in a): raise RuntimeError('Mismatched scene geometry: '+name)
 mask=np.zeros(xx.shape,dtype=bool); foreground=np.zeros(xx.shape,dtype=bool)
 for e in meta['elements']:
  cx=e['x']+e['width']/2;cy=e['y']+e['height']/2
  # A conservative central body region avoids both private corner splines and
  # the perimeter. Exclude every plus, including a neighbor's overlaid content.
  mask|=(abs(xx-cx)<e['width']*.3)&(abs(yy-cy)<e['height']*.3)
  foreground|=(abs(xx-cx)<13)&(abs(yy-cy)<13)
 mask&=~foreground
 aa=np.asarray(a,dtype=float);bb=np.asarray(b,dtype=float)
 result=dict(row,bodyPixels=int(mask.sum()),bodyMAE=float(abs(aa-bb)[mask].mean()))
 c=None
 if before and (before/(name+'.ppm')).exists():
  c=Image.open(before/(name+'.ppm')).convert('RGB');result['beforeBodyMAE']=float(abs(aa-np.asarray(c,dtype=float))[mask].mean())
 if row['case']==12 and row['progress']==1:
  # A small overlap patch below both foreground symbols. Native captures
  # define ownership; compare both color orderings in light and dark themes.
  na=aa[472:480,656:664].mean(axis=(0,1));nb=bb[472:480,656:664].mean(axis=(0,1))
  result['nativeOverlapRGB']=na.tolist();result['nxkitOverlapRGB']=nb.tolist()
  result['sameTintOwner']=bool((na[0]>na[2])==(nb[0]>nb[2]))
  if not result['sameTintOwner']:raise RuntimeError('Tint ownership differs from UIKit: '+name)
 results.append(result);pictures[name]=(a,b,c)
summary=[]
for key in sorted(set((r['case'],r['dark']) for r in rows)):
 states=[r for r in results if (r['case'],r['dark'])==key]
 s=dict(case=key[0],dark=key[1],checkpoints=len(states),bodyMAE=float(np.mean([r['bodyMAE'] for r in states])))
 if before and any('beforeBodyMAE' in r for r in states):s['beforeBodyMAE']=float(np.mean([r['beforeBodyMAE'] for r in states if 'beforeBodyMAE' in r]))
 summary.append(s)
report=dict(nativeOS=meta['os'],states=results,summary=summary,bodyMAE=float(np.mean([r['bodyMAE'] for r in results])))
if before: report['beforeBodyMAE']=float(np.mean([r['beforeBodyMAE'] for r in results if 'beforeBodyMAE' in r]))
default_states=[r for r in results if r['case']!=11]
if default_states:
 report['defaultPolicyBodyMAE']=float(np.mean([r['bodyMAE'] for r in default_states]))
 if before:report['beforeDefaultPolicyBodyMAE']=float(np.mean([r['beforeBodyMAE'] for r in default_states if 'beforeBodyMAE' in r]))
(output/'metrics.json').write_text(json.dumps(report,indent=2)+'\n')
if all(r['case']>=13 for r in rows):
 selected=[r for r in rows if r['backdrop']==0]
 width,height=600,300;sheet=Image.new('RGB',(width*2,height*len(selected)),(245,245,245));draw=ImageDraw.Draw(sheet)
 for i,r in enumerate(selected):
  a,b,_=pictures[r['name']]
  for col,im in enumerate([a,b]):
   crop=im.crop((0,0,1440,260) if r['case']>=15 else (400,280,1040,580))
   crop.thumbnail((width-24,height-48));sheet.paste(crop,(col*width+12,i*height+38))
   title={13:'clear buttons',14:'tinted buttons',15:'mock navigation buttons',16:'container navigation buttons'}[r['case']]
   draw.text((col*width+12,i*height+12),f'{"UIKit" if col==0 else "NXKit"} | {title} | {"dark" if r["dark"] else "light"}',fill='black')
 sheet.save(output/'fixed-buttons.png')
 navigation=[r for r in selected if r['case']>=15]
 if navigation:
  sheet=Image.new('RGB',(1200,170*len(navigation)),(245,245,245));draw=ImageDraw.Draw(sheet)
  for i,r in enumerate(navigation):
   a,b,_=pictures[r['name']]
   for col,im in enumerate([a,b]):
    crop=im.crop((0,0,1440,260));crop.thumbnail((576,122));sheet.paste(crop,(col*600+12,i*170+38))
    draw.text((col*600+12,i*170+12),f'{"UIKit" if col==0 else "NXKit"} | {"container" if r["case"]==16 else "separate"} | {"dark" if r["dark"] else "light"}',fill='black')
  sheet.save(output/'navigation-buttons.png')
elif rows[0]['case']==12:
 selected=[r for r in rows if r['progress']==1]
 width,height=420,270;sheet=Image.new('RGB',(width*2,height*len(selected)),(245,245,245));draw=ImageDraw.Draw(sheet)
 for i,r in enumerate(selected):
  a,b,_=pictures[r['name']]
  for col,im in enumerate([a,b]):
   crop=im.crop((520,325,920,555));crop.thumbnail((width-24,height-35));sheet.paste(crop,(col*width+12,i*height+32))
   draw.text((col*width+12,i*height+10),f'{"UIKit" if col==0 else "NXKit"} | {"dark" if r["dark"] else "light"} | {"blue front" if r.get("reverse") else "coral front"}',fill='black')
 sheet.save(output/'two-tints.png')
 frames=[]
 for r in rows:
  if r['dark']:continue
  a,b,_=pictures[r['name']];f=Image.new('RGB',(960,320),'white');d=ImageDraw.Draw(f)
  for j,im in enumerate([a,b]):
   crop=im.crop((240,290,1180,590));crop.thumbnail((480,280));f.paste(crop,(j*480,35));d.text((j*480+10,10),'UIKit' if not j else 'NXKit',fill='black')
  frames.append(f)
 frames[0].save(output/'two-tints.gif',save_all=True,append_images=frames[1:],duration=650,loop=0)
else:
 chosen=[next(r for r in rows if r['case']==case and r['dark']==dark and r['progress']==0) for case,dark in [(0,0),(1,0),(2,0),(3,0),(1,1),(3,1)]]
 columns=3 if before else 2;w,h=480,330;sheet=Image.new('RGB',(w*columns,h*len(chosen)),(245,245,245));draw=ImageDraw.Draw(sheet)
 for i,r in enumerate(chosen):
  a,b,c=pictures[r['name']];items=[('UIKit',a),('NXKit before',c),('NXKit now',b)] if before else [('UIKit',a),('NXKit',b)]
  for j,(title,im) in enumerate(items):
   im=im.resize((480,293));sheet.paste(im,(j*w,i*h+30));draw.text((j*w+8,i*h+9),f'{title} | demo {r["case"]} | {"dark" if r["dark"] else "light"}',fill='black')
 sheet.save(output/'gallery.png')
print(json.dumps({k:v for k,v in report.items() if k not in ['states','summary']},indent=2))
for r in summary:print(r)
