"""Native exterior-only shadow comparison; no fitted image/color registration.
Usage: python compare-shadows.py MATRIX BEFORE_RENDER_DIR AFTER_RENDER_DIR OUTPUT
"""
from pathlib import Path
import sys,json,numpy as np
from PIL import Image,ImageDraw
sys.path.insert(0,str(Path('Demo/GlassComparison').resolve()))
from comparison_images import load_srgb
matrix,baseline,current,out=map(Path,sys.argv[1:5]);out.mkdir(exist_ok=True,parents=True)
records=json.load(open(matrix/'native/series.json'));yy,xx=np.mgrid[:880,:1440];xx=(xx+.5)/2;yy=(yy+.5)/2;rows=[]
for entry in records:
 req=entry['request'];name=req['id'];a=np.asarray(load_srgb(matrix/'native'/f'{name}.png'),float);b=np.asarray(load_srgb(baseline/f'{name}.ppm'),float);c=np.asarray(load_srgb(current/f'{name}.ppm'),float)
 fields=[];normals=[];valid=[]
 for index,(x,y,w,h,r) in enumerate(entry['frames']):
  if w<=0 or h<=0:continue
  valid.append((index,(x,y,w,h,r)))
  qx=abs(xx-x-w/2)-(w/2-r);qy=abs(yy-y-h/2)-(h/2-r);fields.append(np.hypot(np.maximum(qx,0),np.maximum(qy,0))+np.minimum(np.maximum(qx,qy),0)-r)
  vx=np.maximum(qx,0);vy=np.maximum(qy,0);length=np.maximum(np.hypot(vx,vy),1e-6)
  nx=np.where(np.maximum(qx,qy)>0,np.sign(xx-x-w/2)*vx/length,np.where(qx>qy,np.sign(xx-x-w/2),0))
  ny=np.where(np.maximum(qx,qy)>0,np.sign(yy-y-h/2)*vy/length,np.where(qx>qy,0,np.sign(yy-y-h/2)))
  normals.append(np.stack([nx,ny],axis=-1))
 distance=fields[0].copy();normal=normals[0].copy()
 if req.get('group'):
  for other,n in zip(fields[1:],normals[1:]):
   unit=normal/np.maximum(np.linalg.norm(normal,axis=-1)[...,None],1e-6)
   k=8*np.clip(.5-.5*np.sum(n*unit,axis=-1),0,1);h=np.where(k>1e-4,np.clip(.5+.5*(other-distance)/np.maximum(k,1e-6),0,1),np.where(distance<=other,1,0))
   distance=other*(1-h)+distance*h-k*h*(1-h);normal=n*(1-h[...,None])+normal*h[...,None]
 else:distance=np.min(fields,axis=0)
 exterior=distance>1.5;metrics=[];total=np.zeros(xx.shape,bool)
 for field,(i,frame) in zip(fields,valid):
  x,y,w,h,r=frame;mask=exterior&(field<40)
  if i==4:mask&=(abs(xx-x-w/2)<w/2-2*r-2)|(abs(yy-y-h/2)<h/2-2*r-2)
  for j,(ox,oy,ow,oh,orr) in valid:
   if j!=i:mask&=~((xx>ox-1)&(xx<ox+ow+1)&(yy>oy-1)&(yy<oy+oh+1))
  total|=mask
  metrics.append(dict(index=i,size=[w,h],pixels=int(mask.sum()),beforeMAE=float(abs(b-a)[mask].mean()),afterMAE=float(abs(c-a)[mask].mean()),beforeP95=float(np.percentile(abs(b-a)[mask],95)),afterP95=float(np.percentile(abs(c-a)[mask],95))))
 rows.append(dict(req,elements=metrics))
 if req['background'] in ['gray','white','red','green','blue','yellow'] and req['tint']=='none':
  Image.fromarray((total*255).astype('uint8')).save(out/f'{name}-exterior-mask.png')
  # Residual overlay: red = candidate too bright (weak shadow); blue = too dark.
  delta=(c-a).mean(axis=-1);overlay=np.zeros((*total.shape,4),dtype='uint8')
  overlay[...,0]=np.where(delta>0,255,0);overlay[...,2]=np.where(delta<0,255,0)
  overlay[...,3]=np.where(total,np.minimum(abs(delta)*32,255),0).astype('uint8')
  Image.fromarray(overlay).save(out/f'{name}-residual-overlay.png')
  panels=[]
  for label,arr in [('Native',a),('NXKit before',b),('NXKit after',c),('Exterior absolute difference x8',np.where(total[...,None],np.minimum(abs(c-a)*8,255),0))]:
   im=Image.fromarray(arr.astype('uint8'));p=Image.new('RGB',(720,464),'#202020');p.paste(im.resize((720,440)),(0,24));ImageDraw.Draw(p).text((5,5),label+' | '+name,fill='white');panels.append(p)
  sheet=Image.new('RGB',(1440,928),'#202020')
  for j,p in enumerate(panels):sheet.paste(p,((j%2)*720,(j//2)*464))
  sheet.save(out/f'{name}.png')
summary=dict(method='Native observed geometry; exterior1.5–40pt; group exclusions follow smooth-union zero contour from rounded boundaries; excludes all glass and conservative continuous-card corner bands; no fitted registration/color correction.',records=rows,
 beforeMAE=float(np.mean([v['beforeMAE'] for r in rows for v in r['elements']])),afterMAE=float(np.mean([v['afterMAE'] for r in rows for v in r['elements']])))
summary['byBackground']=[dict(background=bg,beforeMAE=float(np.mean([v['beforeMAE'] for r in rows if r['background']==bg for v in r['elements']])),afterMAE=float(np.mean([v['afterMAE'] for r in rows if r['background']==bg for v in r['elements']]))) for bg in sorted(set(r['background'] for r in rows))]
summary['byTheme']=[dict(dark=d,beforeMAE=float(np.mean([v['beforeMAE'] for r in rows if r['dark']==d for v in r['elements']])),afterMAE=float(np.mean([v['afterMAE'] for r in rows if r['dark']==d for v in r['elements']]))) for d in [0,1]]
(out/'metrics.json').write_text(json.dumps(summary,indent=2));print(json.dumps({k:v for k,v in summary.items() if k!='records'},indent=2))
