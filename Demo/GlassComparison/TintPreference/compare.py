#!/usr/bin/env python3
"""Measure independent native and retained-renderer interior pixels and residuals."""
from pathlib import Path
import json,sys
import numpy as np
from PIL import Image, ImageDraw
native,portable,out=map(Path,sys.argv[1:4]);out.mkdir(parents=True,exist_ok=True)
rows=[]
for meta in sorted(native.glob('*.json')):
 m=json.loads(meta.read_text());case=m['case'];amount=m['amount'];stem=f'{amount:g}-{case["id"]}'
 n=np.asarray(Image.open(native/(meta.stem+'.png')).convert('RGB'),dtype=float);p=np.asarray(Image.open(portable/(stem+'.ppm')).convert('RGB'),dtype=float)
 ns=m['scale'];ps=3
 for q in case['probes']:
  x,y,w,h=q['frame'];cx,cy=x+w/2,y+h/2;r=3
  nr=n[round((cy-r)*ns):round((cy+r)*ns),round((cx-r)*ns):round((cx+r)*ns)].mean((0,1))
  pr=p[round((cy-r)*ps):round((cy+r)*ps),round((cx-r)*ps):round((cx+r)*ps)].mean((0,1))
  rows.append(dict(amount=amount,theme='dark' if case['dark'] else 'light',page=case['id'].split('-')[0],style='clear' if q['clear'] else 'regular',tinted=q['tinted'],size=[w,h],column=q['column'],native=nr.tolist(),portable=pr.tolist(),mae=float(np.abs(nr-pr).mean())))
summary={}
for amount in sorted({r["amount"] for r in rows}):
 selected=[r for r in rows if r['amount']==amount and r['page']!='2'];summary[str(amount)]={'meanChannelError':float(np.mean([r['mae'] for r in selected])),'maxChannelError':float(max(max(abs(np.array(r['native'])-r['portable'])) for r in selected))}
(out/'measurements.json').write_text(json.dumps(rows,indent=2)+'\n');(out/'summary.json').write_text(json.dumps(summary,indent=2)+'\n');print(json.dumps(summary,indent=2))
for theme in ['dark','light']:
 for style in ['regular','clear']:
  for tinted in [False,True]:
   samples=[r for r in rows if r['theme']==theme and r['style']==style and r['tinted']==tinted and r['page']=='0' and r['size'][0]==44]
   print(theme,style,'blue' if tinted else 'neutral',[(r['amount'],r['column'],[round(x) for x in r['native']]) for r in samples])
# Compare high-frequency transmission without treating UIKit's drawHierarchy
# reconstruction as a blur reference.
texture=[]
for meta in sorted(native.glob('*.json')):
 m=json.loads(meta.read_text());c=m['case']
 if c['id'][0]!='2':continue
 n=np.asarray(Image.open(native/(meta.stem+'.png')).convert('RGB'),dtype=float)
 p=np.asarray(Image.open(portable/(f'{m["amount"]:g}-{c["id"]}.ppm')).convert('RGB'),dtype=float)
 for q in c['probes']:
  if q['column']!=0:continue
  x,y,w,h=q['frame'];cx,cy=x+w/2,y+h/2
  def amplitude(im,scale):return im[round((cy-10)*scale):round((cy+10)*scale),round((cx-12)*scale):round((cx+12)*scale)].std((0,1)).mean()
  texture.append(dict(amount=m['amount'],dark=c['dark'],clear=q['clear'],tinted=q['tinted'],size=[w,h],native=float(amplitude(n,m['scale'])),portable=float(amplitude(p,3))))
textureError=[abs(r['native']-r['portable']) for r in texture]
if textureError:print('Texture amplitude error: mean',float(np.mean(textureError)),'max',float(max(textureError)))
(out/'texture.json').write_text(json.dumps(texture,indent=2)+'\n')
# Native and NXKit columns in each preference group; black and gray controls.
for theme in ['dark','light']:
 sheet=Image.new('RGB',(1010,420),(236,236,236));draw=ImageDraw.Draw(sheet)
 draw.text((10,10),f'iOS 27.1 / NXKit - {theme}; columns: native, NXKit; background: black, gray',fill='black')
 for i,amount in enumerate([0,.5,1]):
  left=140+i*290;draw.text((left,35),f'System {amount:.0%}',fill='black')
  key=f'{amount:g}-0-{theme}';n=Image.open(native/(key+'.png')).convert('RGB');p=Image.open(portable/(key+'.ppm')).convert('RGB')
  for j,row in enumerate([4,5,6,7]):
   top=64+j*86
   if i==0:draw.text((8,top+30),['regular / neutral','clear / neutral','regular / blue','clear / blue'][j],fill='black')
   for col,im in enumerate([n,p]):
    crop=im.crop((0,row*88*3,256*3,(row*88+88)*3)).resize((140,77));sheet.paste(crop,(left+col*142,top))
 sheet.save(out/(theme+'-matrix.png'))
flat=[r for r in rows if r['page']!='2']
if np.mean([r['mae'] for r in flat])>3 or any(max(abs(np.array(r['native'])-r['portable']))>16 for r in flat):
 raise SystemExit('FAIL: native flat-color residual exceeds the measured tolerance')

if textureError and (np.mean(textureError)>3 or max(textureError)>12):
 raise SystemExit('FAIL: composited texture transmission exceeds measured tolerance')
