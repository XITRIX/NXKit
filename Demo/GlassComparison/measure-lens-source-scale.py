"""Measure deep-center native source markers, without fitting the lens image.
Usage: measure-lens-source-scale.py CAPTURE_NATIVE OUTPUT
Capture requests are retained as magnification/standalone-cases.json; use
build-lens.py / capture-lens.py to repeat on an available iOS simulator.
"""
from pathlib import Path
import gzip,hashlib,io,json,shutil,sys
import numpy as np
from PIL import Image,ImageCms
source,out=map(Path,sys.argv[1:3]);raw=out/'native-standalone';raw.mkdir(parents=True,exist_ok=True)
records=[];hashes={}
for metadata in sorted(source.glob('center-d*-s*-portal-only.json')):
 d=json.loads(metadata.read_text());name=metadata.stem;p=source/(name+'.png');im=Image.open(p);icc=im.info.get('icc_profile');a=np.array(ImageCms.profileToProfile(im,ImageCms.ImageCmsProfile(io.BytesIO(icc)),ImageCms.createProfile('sRGB'),outputMode='RGB') if icc else im.convert('RGB'),float)
 points=d['request']['markerPoints'];scale=d['scale'];centers=[]
 for px,py in points:
  x,y=px+40,py+250
  xx,yy=np.meshgrid(np.arange(round(x*scale)-7,round(x*scale)+8),np.arange(round(y*scale)-7,round(y*scale)+8));v=a[yy,xx]
  # Saturated markers on neutral glass: channel range removes the material.
  w=np.maximum(v.max(2)-v.min(2)-32,0)
  centers.append([float(((xx+.5)/scale*w).sum()/w.sum()),float(((yy+.5)/scale*w).sum()/w.sum())])
 r=dict(id=name,os=d['osBuild'],sourcePoints=points,stageCentroids=centers,horizontalScale=(centers[1][0]-centers[0][0])/(points[1][0]-points[0][0]),verticalScale=(centers[3][1]-centers[2][1])/(points[3][1]-points[2][1]))
 assert abs(r['horizontalScale']-1)<.001 and abs(r['verticalScale']-1)<.001
 records.append(r);shutil.copy2(p,raw/p.name)
 with gzip.open(raw/(metadata.name+'.gz'),'wb') as f:f.write(metadata.read_bytes())
 for file in [p,metadata]:hashes[file.name]=hashlib.sha256(file.read_bytes()).hexdigest()
assert len(records)==4
(out/'standalone-measurements.json').write_text(json.dumps(dict(method='Requested off-center marker spacing versus observed stage spacing; fixed bar origin40,250; channel-range centroid on neutral gray. No fitted translation or scaling.',records=records,inputSHA256=hashes),indent=2)+'\n')
