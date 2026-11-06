"""Difference only native framebuffer perturbations at identical registered samples."""
from pathlib import Path
import json,sys
root=Path(sys.argv[1]);data=json.loads((root/'samples.json').read_text());output={}
for theme in ['dark','light']:
 if theme+'-baseline'not in data:continue
 baseline=data[theme+'-baseline'];regions={}
 for case,shapes in data.items():
  if not case.startswith(theme+'-') or case.endswith('-baseline') or '-uniform-'in case or '-sdf-'in case:continue
  measurements=[]
  for original,changed in zip(baseline,shapes):
   assert original['frame']==changed['frame'] and original['gray']==changed['gray']
   bands={}
   for name,lower,upper in [('interior',-12,-3),('rim',-3,0),('microcontour',0,1),('exterior',1,20.1)]:
    deltas=[];signed=[]
    for a,b in zip(original['samples'],changed['samples']):
     assert a['pixel']==b['pixel']
     if lower<=a['distance']<upper:
      signed.extend(x-y for x,y in zip(a['rgb'],b['rgb']));deltas.extend(abs(x-y)for x,y in zip(a['rgb'],b['rgb']))
    bands[name]={'meanAbsoluteDelta':sum(deltas)/len(deltas),'maximumDelta':max(deltas),'meanSignedDelta':sum(signed)/len(signed)}
   measurements.append({'frame':original['frame'],'gray':original['gray'],'bands':bands})
  regions[case]=measurements
 output[theme]=regions
(root/'differences.json').write_text(json.dumps(output,indent=2)+'\n')
for theme,rows in output.items():
 print(theme)
 for case,shapes in rows.items():print(case,[(x['gray'],x['frame'],round(x['bands']['rim']['meanAbsoluteDelta'],3),x['bands']['rim']['maximumDelta'])for x in shapes])
