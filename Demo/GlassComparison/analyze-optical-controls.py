"""Extract native and optional NXKit center RGB table from a completed or partial matrix."""
from pathlib import Path
import json,sys
import numpy as np
from PIL import Image
from comparison_images import load_srgb
root=Path(sys.argv[1]);native=root/'native';rows=[]
for metadata in sorted(native.glob('*.json')):
 if metadata.name=='series.json':continue
 m=json.loads(metadata.read_text());r=m['request'];image=np.asarray(load_srgb(native/(r['id']+'.png')),dtype=float);values=[]
 for x,y,w,h,radius in m['frames']:
  cx,cy=round((x+w/2)*2),round((y+h/2)*2);values.append(image[cy-6:cy+6,cx-6:cx+6].mean(axis=(0,1)).tolist())
 rows.append(dict(r,RGB=values))
(root/'native-controls.json').write_text(json.dumps(rows,indent=2)+'\n')
for r in rows:print(r['id'],['/'.join(f'{v:.1f}' for v in rgb) for rgb in r['RGB']])
