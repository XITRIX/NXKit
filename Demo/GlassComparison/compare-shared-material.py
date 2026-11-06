"""Usage: python compare-shared-material.py MATRIX BEFORE AFTER OUTPUT.
Requires Pillow/NumPy; geometry is native metadata, with no fitted alignment.
"""
from pathlib import Path
import json,sys,numpy as np
from PIL import Image,ImageDraw
sys.path.insert(0,'Demo/GlassComparison');from comparison_images import load_srgb
matrix,before,after,out=map(Path,sys.argv[1:5]);out.mkdir(exist_ok=True,parents=True)
rows=[]
yy,xx=np.mgrid[:880,:1440];xx=(xx+.5)/2;yy=(yy+.5)/2
for entry in json.load(open(matrix/'native/series.json')):
 req=entry['request'];name=req['id'];path=after/f'{name}.ppm'
 if not path.exists():raise FileNotFoundError(path)
 a=np.asarray(load_srgb(matrix/'native'/f'{name}.png'),float);b=np.asarray(load_srgb(before/(name+'.ppm')),float);c=np.asarray(load_srgb(path),float);mask=np.zeros(xx.shape,bool);elems=[]
 for i,(x,y,w,h,r) in enumerate(entry['frames']):
  qx=abs(xx-x-w/2)-(w/2-r);qy=abs(yy-y-h/2)-(h/2-r)
  d=np.hypot(np.maximum(qx,0),np.maximum(qy,0))+np.minimum(np.maximum(qx,qy),0)-r
  m=d < -20;mask|=m
  elems.append(dict(index=i,size=[w,h],pixels=int(m.sum()),nativeRGB=a[m].mean(axis=0).tolist(),beforeRGB=b[m].mean(axis=0).tolist(),afterRGB=c[m].mean(axis=0).tolist(),beforeMAE=float(abs(b-a)[m].mean()),afterMAE=float(abs(c-a)[m].mean())))
 rows.append(dict(req,elements=elems))
 panels=[]
 for label,data in [('Native',a),('Current separate size',b),('Candidate shared size',c),('Body error x8',np.where(mask[...,None],np.minimum(abs(c-a)*8,255),0))]:
  p=Image.new('RGB',(720,464),'#202020');p.paste(Image.fromarray(data.astype('uint8')).resize((720,440)),(0,24));ImageDraw.Draw(p).text((6,5),label+' | '+name,fill='white');panels.append(p)
 sheet=Image.new('RGB',(1440,928),'#202020')
 for i,p in enumerate(panels):sheet.paste(p,(i%2*720,i//2*464))
 sheet.save(out/f'{name}.png')
summary=dict(method='Native observed geometry; unchanged own lens/shape; body SDF<-20pt; no fitted alignment/color corrections. Before uses frozen shadow-only adapter (uniform gray/white neutral material unchanged by independent combined optical corrections).',records=rows)
for label,predicate in [('all',lambda r:True),('light',lambda r:r['dark']==0),('dark',lambda r:r['dark']==1)]:
 e=[v for r in rows if predicate(r) for v in r['elements']];summary[label]=dict(elements=len(e),beforeMAE=float(np.mean([v['beforeMAE'] for v in e])),afterMAE=float(np.mean([v['afterMAE'] for v in e])))
(out/'metrics.json').write_text(json.dumps(summary,indent=2)+'\n');print(json.dumps({k:v for k,v in summary.items() if k!='records'},indent=2))
