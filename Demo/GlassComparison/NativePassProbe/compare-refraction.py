"""Production retained-rendering regression against actual UIKit source coordinates."""
import argparse,json,subprocess,tempfile,plistlib,base64
from pathlib import Path
import numpy as np
from PIL import Image
p=argparse.ArgumentParser();p.add_argument('--renderer',type=Path,required=True);p.add_argument('--reference',type=Path,required=True);p.add_argument('--output',type=Path);a=p.parse_args();results=[]
frames=[(30,100,64,64),(150,100,120,120),(30,300,280,64),(30,460,64,160),(150,460,160,160)]
with tempfile.TemporaryDirectory(prefix='nxkit-refraction-')as tmp:
 for axis,extent in [('x',402),('y',874)]:
  for mode in ['identity','inner','inner-small','inner-height']:
   output=Path(tmp)/f'{axis}-{mode}.ppm';subprocess.run([str(a.renderer),str(a.reference/f'{axis}-background.png'),str(output),mode],check=True)
   actual=np.asarray(Image.open(output)).astype(float);native=np.asarray(Image.open(a.reference/f'{axis}-{mode}.png')).astype(float);meta=json.loads((a.reference/f'{axis}-{mode}.json').read_text());errors=[]
   for idx,(x,y,w,h)in enumerate(frames):
    objects=plistlib.loads(base64.b64decode(meta['mutations'][idx]['__profile']['archiveBase64']))['$objects'];d=objects[4];inputs={objects[k.data]:objects[v.data]for k,v in zip(d['NS.keys'],d['NS.objects'])};height=8 if mode=='inner-height'else inputs['inputInnerRefractionHeight'];amount=0 if mode=='identity'else(-8 if mode=='inner-small'else inputs['inputInnerRefractionAmount'])
    for depth in np.arange(1,min(w,h)/2-1,1/3):
     pos=np.array([x+depth,y+h/2])if axis=='x'else np.array([x+w/2,y+depth]);ij=np.floor(pos*3).astype(int);coordinate=ij[0 if axis=='x'else 1]/3+1/6;distance=coordinate-(x if axis=='x'else y);t=np.clip(1-distance/height,0,1);source=coordinate-amount*(1-np.sqrt(1-t*t))
     # Coordinate ramps have a deliberate discontinuity every64pt. Reject the
     # capture footprint crossing it rather than decoding a blend as position.
     if min(source%64,64-source%64)<4:continue
     def decode(v):
      g=v[1]/255*64;return g+np.round((v[0]/255*extent-g)/64)*64
     errors.append(abs(decode(actual[ij[1],ij[0]])-decode(native[ij[1],ij[0]])))
   row={'axis':axis,'mode':mode,'samples':len(errors),'meanErrorPoints':float(np.mean(errors)),'maximumErrorPoints':float(np.max(errors))};results.append(row);print(json.dumps(row),flush=True)
if a.output:a.output.write_text(json.dumps(results,indent=2)+'\n')
assert all(r['samples']>200 and r['meanErrorPoints']<.35 and r['maximumErrorPoints']<.9 for r in results),results
