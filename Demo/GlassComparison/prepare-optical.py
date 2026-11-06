"""Deterministic 720x440 sRGB PNG backgrounds shared by UIKit and NXKit."""
from pathlib import Path
import sys
from PIL import Image,ImageCms
import numpy as np
out=Path(sys.argv[1])/'backgrounds';out.mkdir(exist_ok=True,parents=True)
y,x=np.mgrid[:440,:720]
for name,color in {'black':(0,0,0),'white':(255,255,255),'gray':(128,128,128),'red':(255,0,0),'green':(0,255,0),'blue':(0,0,255),'yellow':(255,255,0),'mint':(128,200,160),'lavender':(180,120,200),'peach':(200,160,128)}.items():Image.new('RGB',(720,440),color).save(out/(name+'.png'))
# Wide-band smooth channels: local displacement can be inferred independently on each axis.
ramp=np.stack([np.round(x/719*255),np.round(y/439*255),np.full_like(x,128)],axis=2).astype('uint8');Image.fromarray(ramp).save(out/'ramp.png')
# Narrow lines keep the edge's optical fold visible against a constant neutral body.
lines=np.full((440,720,3),128,dtype='uint8');lines[(x%40)<3]=[255,255,255];lines[(y%40)<3]=[0,0,0];Image.fromarray(lines).save(out/'lines.png')
# Smooth colored wave stripes avoid a bitmap interpolation ambiguity.
a=2*np.pi*(x+y*.35)/120
stripes=np.stack([128+110*np.sin(a),128+110*np.sin(a+2.094),128+110*np.sin(a+4.189)],axis=2).clip(0,255).astype('uint8');Image.fromarray(stripes).save(out/'stripes.png')

# Independent frequency holdouts reuse the same continuous RGB wave family.
# They are not part of the default 60-scene capture batch.
for period in [60, 240]:
 phase=2*np.pi*(x+y*.35)/period
 wave=np.stack([128+110*np.sin(phase),128+110*np.sin(phase+2.094),128+110*np.sin(phase+4.189)],axis=2).clip(0,255).astype('uint8')
 Image.fromarray(wave).save(out/('waves'+str(period)+'.png'))

# Encode the shared input profile explicitly; raw channel bytes are unchanged.
profile=ImageCms.ImageCmsProfile(ImageCms.createProfile("sRGB")).tobytes()
for path in out.glob("*.png"):
 image=Image.open(path);image.load();image.save(path,icc_profile=profile)
