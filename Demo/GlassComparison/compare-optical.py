"""Strictly aligned UIKit/NXKit comparison. No translation, color fitting, or exposure correction.
Usage: compare-optical.py MATRIX OUTPUT [--render=EXECUTABLE] [--shader=FILE] [--baseline=DIR]
Creates exact and explicitly thresholded transparent residuals plus full-resolution RGB differences.
"""
from pathlib import Path
import json,subprocess,sys
import numpy as np
from PIL import Image,ImageDraw
from comparison_images import load_srgb
matrix,out=map(lambda p:Path(p).resolve(),sys.argv[1:3]);out.mkdir(exist_ok=True,parents=True)
options=dict(a[2:].split('=',1) for a in sys.argv[3:]);native=matrix/'native';records=json.loads((native/'series.json').read_text())
manifest=out/'manifest.txt';lines=[]
for m in records:
 r=m['request'];frames=m['frames'];name=r['id']
 lines.append(' '.join([json.dumps(str(out/(name+'.ppm'))),json.dumps(str(matrix/'backgrounds'/(r['background']+'.png'))),str(r['dark']),r['tint']]+[str(v) for f in frames for v in f]))
manifest.write_text('\n'.join(lines)+'\n')
if 'render' in options:
 renderManifest=manifest
 if 'only-missing' in options:
  renderManifest=out/'missing-manifest.txt';renderManifest.write_text('\n'.join(line for line,m in zip(lines,records) if not (out/(m['request']['id']+'.ppm')).exists())+'\n')
 if renderManifest.read_text().strip():subprocess.run([options['render'],str(renderManifest)]+([options['shader']] if 'shader' in options else []),check=True)
y,x=np.mgrid[:880,:1440];x=(x+.5)/2;y=(y+.5)/2
results=[];table=[]
for m in records:
 r=m['request'];name=r['id'];a=np.asarray(load_srgb(native/(name+'.png')),dtype=float);b=np.asarray(Image.open(out/(name+'.ppm')).convert('RGB'),dtype=float)
 beforeImages={}
 for label,option in [('before','baseline'),('calibratedBefore','calibrated-baseline')]:
  if option in options:
   path=Path(options[option])/(name+'.ppm')
   if path.exists():beforeImages[label]=np.asarray(load_srgb(path),dtype=float)
 diff=abs(a-b);maxdiff=diff.max(axis=2);allglass=np.zeros(x.shape,bool)
 metrics=[]
 for i,f in enumerate(m['frames']):
  fx,fy,w,h,rad=f;cx=fx+w/2;cy=fy+h/2
  # Exact for circles/pill; for the continuous-corner card use conservative regions.
  qx=abs(x-cx)-(w/2-rad);qy=abs(y-cy)-(h/2-rad)
  sdf=np.sqrt(np.maximum(qx,0)**2+np.maximum(qy,0)**2)+np.minimum(np.maximum(qx,qy),0)-rad
  body=(sdf<-min(12,min(w,h)*.2));rim=(sdf<-.75)&(sdf>-2.75);refraction=(sdf<-3)&(sdf>-12)
  if i==4:
   # Corner-curve approximation must never contaminate rim or optical metrics.
   straight=(abs(x-cx)<w/2-2*rad-2)|(abs(y-cy)<h/2-2*rad-2);rim&=straight;refraction&=straight
  allglass|=sdf<2
  result=dict(index=i,size=[w,h],nativeCenterRGB=a[(abs(x-cx)<3)&(abs(y-cy)<3)].mean(axis=0).tolist(),nxkitCenterRGB=b[(abs(x-cx)<3)&(abs(y-cy)<3)].mean(axis=0).tolist())
  for key,mask in [('body',body),('rim',rim),('refraction',refraction)]:
   result[key+'MAE']=float(diff[mask].mean());result[key+'Pixels']=int(mask.sum())
   for label,old in beforeImages.items():result[label+key.capitalize()+'MAE']=float(abs(a-old)[mask].mean())
  metrics.append(result)
 baseline=Path(options['baseline'])/(name+'.ppm') if 'baseline' in options else None
 row=dict(r,elements=metrics,outsideMAE=float(diff[~allglass].mean()),glassMAE=float(diff[allglass].mean()),exactMatchingPixels=int((maxdiff==0).sum()),within2MatchingPixels=int((maxdiff<=2).sum()),glassPixels=int(allglass.sum()),glassExactMatchingPixels=int(((maxdiff==0)&allglass).sum()),glassWithin2MatchingPixels=int(((maxdiff<=2)&allglass).sum()))
 if baseline and baseline.exists():
  old=np.asarray(Image.open(baseline).convert('RGB'),dtype=float);row['beforeGlassMAE']=float(abs(a-old)[allglass].mean())
 results.append(row)
 # Suppression means the residual itself becomes transparent where all channels agree.
 for threshold,label in [(0,'exact'),(2,'within2')]:
  rgba=np.concatenate([b.astype('uint8'),np.where(maxdiff>threshold,255,0).astype('uint8')[...,None]],axis=2);Image.fromarray(rgba).save(out/(name+'-unmatched-'+label+'.png'))
 Image.fromarray(diff.clip(0,255).astype('uint8')).save(out/(name+'-absolute.png'))
 # Red/cyan channels expose edge displacement without fitting either renderer.
 overlay=np.stack([a.mean(axis=2),b.mean(axis=2),b.mean(axis=2)],axis=2).astype('uint8');Image.fromarray(overlay).save(out/(name+'-red-cyan.png'))
 if r['background'] in ['gray','lines','stripes'] and r['tint']=='none':table.append((name,a.astype(np.uint8),b.astype(np.uint8),diff.astype(np.uint8)))
summary=dict(captureCount=len(results),elementCount=len(results)*5,alignment='Observed scene/frame metadata; no fitted transforms',colorSpace='Native framebuffer sRGB declaration retained; any embedded ICC converted through shared LittleCMS loader; no fitted color adjustments',threshold='Within2 removes a pixel only if every sRGB channel differs by at most 2/255',states=results)
summary['regionDefinition']={'exteriorShadowScored': False, 'body': 'More than min(12, 0.2*shortSide) points inside the rounded boundary', 'rim': '0.75–2.75 points inside the boundary', 'refraction': '3–12 points inside the boundary', 'cardCorners': 'Rim and refraction masks conservatively exclude twice the corner radius plus 2 points around each corner', 'weighting': 'Equal mean over each geometry and scene; not area-weighted across sizes', 'auxiliaryMetrics': 'glassMAE includes the visualization union up to 2 points outside the boundary; outsideMAE covers its complement. Neither is a complete exterior-shadow score.'}
summary['aggregate']={}
for field in ['bodyMAE','rimMAE','refractionMAE','beforeBodyMAE','beforeRimMAE','beforeRefractionMAE','calibratedBeforeBodyMAE','calibratedBeforeRimMAE','calibratedBeforeRefractionMAE']:
 values=[e[field] for r in results for e in r['elements'] if field in e]
 if values:summary['aggregate'][field]=float(np.mean(values))
summary['byBackground']=[dict(background=bg,**{field:float(np.mean([e[field] for r in results if r['background']==bg for e in r['elements']])) for field in ['bodyMAE','rimMAE','refractionMAE']}) for bg in sorted(set(r['background'] for r in results))]
(out/'metrics.json').write_text(json.dumps(summary,indent=2)+'\n')
# Compact human view; full-resolution captures and transparent residuals remain beside it.
width=480;h=330;sheet=Image.new('RGB',(width*3,h*len(table)),(235,235,235));draw=ImageDraw.Draw(sheet)
for i,(name,a,b,diff) in enumerate(table):
 for j,(label,image) in enumerate([('UIKit',a),('NXKit',b),('Absolute difference x4',np.minimum(diff.astype(float)*4,255))]):
  picture=Image.fromarray(image.astype('uint8')).resize((480,293));sheet.paste(picture,(j*width,i*h+30));draw.text((j*width+8,i*h+8),label+' | '+name,fill='black')
if table:sheet.save(out/'matrix.png')
print(json.dumps(dict(captures=len(results),bodyMAE=float(np.mean([e['bodyMAE'] for r in results for e in r['elements']])),rimMAE=float(np.mean([e['rimMAE'] for r in results for e in r['elements']])))))
