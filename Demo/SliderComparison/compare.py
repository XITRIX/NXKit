#!/usr/bin/env python3
"""Compare native P3 screenshots with --render output after conversion to sRGB."""
import argparse, hashlib, io, json
from pathlib import Path
from PIL import Image, ImageCms, ImageDraw
import numpy as np

root=Path(__file__).resolve().parents[2]
parser=argparse.ArgumentParser();parser.add_argument('render',type=Path)
parser.add_argument('--output',type=Path,default=root/'docs/slider/comparison');args=parser.parse_args()
args.output.mkdir(parents=True,exist_ok=True)
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()
results=[];panels=[]
for theme in ['light','dark']:
    for name in ['default','disabled','thumbless','tinted','min-image','max-image','both-images','thumb-image','highlighted','selected','ticks','explicit-zero','explicit-one']:
        path=root/'docs/slider/native/final-geometry'/theme/(name+'.png')
        native=Image.open(path)
        if native.info.get('icc_profile'):
            native=ImageCms.profileToProfile(native,ImageCms.ImageCmsProfile(io.BytesIO(native.info['icc_profile'])),ImageCms.createProfile('sRGB'),outputMode='RGB')
        else:native=native.convert('RGB')
        rendered=args.render/(theme+'-'+name+'.ppm');ours=Image.open(rendered).convert('RGB')
        # Fixed fixture ROI, not production geometry: entire 300pt slider plus
        # 10pt sampling margin, at 3x. Includes contour, images and shadow.
        roi=(30,60,990,270);a=np.asarray(native.crop(roi),dtype=float);b=np.asarray(ours.crop(roi),dtype=float)
        error=np.abs(a-b)
        results.append({'theme':theme,'case':name,'MAE':float(error.mean()),'maximum':int(error.max()),'p99':float(np.percentile(error,99)),
                        'nativeSHA256':digest(path),'renderSHA256':digest(rendered),'colorSpace':'sRGB','roi':roi})
        if name in ['default','disabled','thumbless','tinted']:
            # Equal-size native/port pair in separate rows.
            panel=Image.new('RGB',(960,445),'#ddd');panel.paste(native.crop(roi),(0,25));panel.paste(ours.crop(roi),(0,235))
            ImageDraw.Draw(panel).text((5,5),theme+' / '+name+' / native above, NXKit below',fill='black');panels.append(panel)
montage=Image.new('RGB',(1920,445*4),'#ddd')
for i,panel in enumerate(panels):montage.paste(panel,(960*(i//4),445*(i%4)))
montage.save(args.output/'resting.png')
report={'coverage':'26 resting 3x fixture comparisons. Does not establish dynamic or physical-device parity. Native P3 converted to sRGB before comparison.',
        'cases':results}
(args.output/'metrics.json').write_text(json.dumps(report,indent=2)+'\n')
for row in results:print(row['theme'],row['case'],'MAE',round(row['MAE'],4),'p99',row['p99'],'max',row['maximum'])
