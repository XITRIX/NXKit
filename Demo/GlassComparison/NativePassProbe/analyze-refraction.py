"""Decode actual framebuffer source coordinates against an independently stated sag law."""
from pathlib import Path
import argparse,json,plistlib,base64
import numpy as np
from PIL import Image
p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--generate-backgrounds',action='store_true');a=p.parse_args()
if a.generate_backgrounds:
 for axis,extent in [('x',402),('y',874)]:
  y,x=np.mgrid[:2622,:1206];v=(x if axis=='x' else y)/3
  data=np.stack([np.rint(255*v/extent),np.rint(255*(v%64)/64),np.full(v.shape,80)],axis=-1).astype('uint8');Image.fromarray(data).save(a.root/f'{axis}-background.png')
frames=[(30,100,64,64),(150,100,120,120),(30,300,280,64),(30,460,64,160),(150,460,160,160)]
results=[]
for axis,extent in [('x',402),('y',874)]:
 for mode in ['inner','inner-small','inner-height']:
  path=a.root/f'{axis}-{mode}.png'
  if not path.exists():continue
  image=np.asarray(Image.open(path)).astype(float)
  errors=[]
  meta=json.loads(path.with_suffix('.json').read_text())
  for idx,(x,y,w,h) in enumerate(frames):
   objects=plistlib.loads(base64.b64decode(meta['mutations'][idx]['__profile']['archiveBase64']))['$objects'];d=objects[4];inputs={objects[k.data]:objects[v.data] for k,v in zip(d['NS.keys'],d['NS.objects'])}
   height=inputs['inputInnerRefractionHeight'] if mode!='inner-height' else 8;amount=inputs['inputInnerRefractionAmount'] if mode!='inner-small' else -8
   # Planar silhouette normals on the centerline isolate the distance law.
   for depth in np.arange(1, min(height,16),1):
    pos=np.array([x+depth,y+h/2]) if axis=='x' else np.array([x+w/2,y+depth]);ij=np.floor(pos*3).astype(int);actual=ij/3+1/6;v=image[ij[1],ij[0]];g=v[1]/255*64;decoded=g+np.round((v[0]/255*extent-g)/64)*64
    distance=(actual[0]-x) if axis=='x' else(actual[1]-y)
    t=np.clip(1-distance/height,0,1);expected=-amount*(1-np.sqrt(1-t*t));observed=decoded-actual[0 if axis=='x' else 1]
    if v[1]>8 and v[1]<247:errors.append(abs(expected-observed))
  results.append({'axis':axis,'mode':mode,'samples':len(errors),'meanErrorPoints':float(np.mean(errors)),'maximumErrorPoints':float(np.max(errors))})
assert all(r['maximumErrorPoints']<.5 and r['meanErrorPoints']<.3 for r in results),results
print(json.dumps(results,indent=2));(a.root/'coordinate-error.json').write_text(json.dumps(results,indent=2)+'\n')
