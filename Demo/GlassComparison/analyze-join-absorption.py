"""Diagnostic seam analysis with local native/current body controls.
Usage: python analyze-join-absorption.py CAPTURES GROUP_REPORT OUTPUT.json [CANDIDATE_RASTERS]
The body mapping removes endpoint color bias for this diagnostic only. It is
never applied to comparison figures or counted as renderer parity. The mask
matches this fixture's 52-point continuous circles and diagonal transforms.
"""
from pathlib import Path
import sys
import json,numpy as np
from PIL import Image
from comparison_images import load_srgb, COLOR_MANAGEMENT
p=Path(sys.argv[1]);q=Path(sys.argv[2]);output=Path(sys.argv[3]);candidate=Path(sys.argv[4]) if len(sys.argv)>4 else None;meta=json.loads((q/'metrics.json').read_text())['checkpoints'];results=[]
def controls(image,rect):
 x,y,w,h=rect;return np.mean([image[round((y+h*(.5+dy/52))*2),round((x+w*(.5+dx/52))*2),:3] for dx in [-10,-8,8,10] for dy in [-10,-8,8,10]],axis=0)
for rec in meta:
 if rec['checkpoint'] not in ['held','pulled','release-5']:continue
 n=np.asarray(load_srgb(p/rec['case']/(rec['checkpoint']+'.png')),dtype=float)
 c=np.asarray(load_srgb(q/'after'/f"{rec['case']}-{rec['checkpoint']}.ppm"),dtype=float)
 nb,nf=[controls(n,r) for r in rec['rectangles']];cb,cf=[controls(c,r) for r in rec['rectangles']]
 yy,xx=np.mgrid[400:480,688:748];xy=np.stack(((xx+.5)/2,(yy+.5)/2),axis=-1)
 def distance(rect):
  x,y,w,h=rect;z=(xy-np.array([x+w/2,y+h/2]))/np.array([w/52,h/52])
  uv=np.abs(z)/26;norm=np.sum(uv**2.05,axis=-1)**(1/2.05)
  local_gradient=np.sign(z)*(uv/np.maximum(norm[...,None],1e-9))**1.05/26
  local_magnitude=np.maximum(np.linalg.norm(local_gradient,axis=-1),1e-9)
  local_distance=(norm-1)/local_magnitude;normal=local_gradient/local_magnitude[...,None]
  grad=normal/np.array([w/52,h/52]);m=np.maximum(np.linalg.norm(grad,axis=-1),1e-9)
  return local_distance/m,grad/m[...,None]
 da,na=distance(rec['rectangles'][0]);db,nbn=distance(rec['rectangles'][1]);k=8*np.clip(.5-.5*np.sum(na*nbn,axis=-1),0,1)
 h=np.clip(.5+.5*(db-da)/np.maximum(k,1e-4),0,1);field=db*(1-h)+da*h-k*h*(1-h)
 mask=(db>0)&(db<10)&(field<-.05)
 if not mask.any():results.append(dict(case=rec['case'],phase=rec['checkpoint'],pixels=0));continue
 nn=n[yy,xx][mask];cc=c[yy,xx][mask]
 w=np.clip(np.sum((cc-cb)*(cf-cb),axis=-1)/np.sum((cf-cb)**2),0,1)
 normalized=cc+(1-w[:,None])*(nb-cb)+w[:,None]*(nf-cf)
 beta=np.clip(np.sum((normalized-nn)*normalized)/np.sum(normalized**2),0,.5)
 before=np.mean(np.abs(normalized-nn));after=np.mean(np.abs(normalized*(1-beta)-nn));fixed=np.mean(np.abs(normalized*.9-nn))
 results.append(dict(case=rec['case'],phase=rec['checkpoint'],pixels=int(mask.sum()),bodyNative=[nb.tolist(),nf.tolist()],bodyCurrent=[cb.tolist(),cf.tolist()],normalizedSeamMAE=float(before),leastSquaresAbsorption=float(beta),leastSquaresScalarMAE=float(after),fixedTenPercentMAE=float(fixed),meanResidualRGB=np.mean(normalized-nn,axis=0).tolist()))
 if candidate is not None:
  image=np.asarray(load_srgb(candidate/f"{rec['case']}-{rec['checkpoint']}.ppm"),dtype=float)
  xb,xf=[controls(image,r) for r in rec['rectangles']];values=image[yy,xx][mask]
  ww=np.clip(np.sum((values-xb)*(xf-xb),axis=-1)/np.sum((xf-xb)**2),0,1)
  mapped=values+(1-ww[:,None])*(nb-xb)+ww[:,None]*(nf-xf)
  results[-1]['candidateNormalizedSeamMAE']=float(np.mean(np.abs(mapped-nn)))
  results[-1]['candidateRawSeamMAE']=float(np.mean(np.abs(values-nn)))
  results[-1]['currentRawSeamMAE']=float(np.mean(np.abs(cc-nn)))

report=dict(colorManagement=COLOR_MANAGEMENT,method='Local body endpoint mapping for diagnostic residuals only; no image registration.',limitations=['All fresh pairs have blue in front.','Settled pairs have no join; their zero mask is not evidence of settled-join correctness.','Absorption coefficients are diagnostic least-squares fits, not production changes.'],checkpoints=results)
print(json.dumps(report,indent=2));output.write_text(json.dumps(report,indent=2)+'\n')
