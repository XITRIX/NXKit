"""Exercise both portable foreground APIs against native intermediate pixels.
This isolates color transfer from the independently approximated glass optics.
Usage: compare-glass-color-transfer.py RENDERER REPORT_DIRECTORY
"""
from pathlib import Path
import json,subprocess,sys,hashlib
import numpy as np
renderer,out=Path(sys.argv[1]).resolve(),Path(sys.argv[2]);data=json.loads((out/'transfer.json').read_text());samples=data['samples'];lines=[];requests=[]
for i,r in enumerate(samples):
 for scale in ([1,2,3] if i%12==0 else [2]):
  alpha=1.;fields=[f's{i}-{scale}',int(r['dark']),*map(lambda x:round(x*255),r['source']),*map(lambda x:round(x*255),r['background']),alpha,scale];lines.append(' '.join(map(str,fields)));requests.append(dict(id=fields[0],sample=i,scale=scale,alpha=alpha))
manifest=out/'transfer-input.txt';manifest.write_text('\n'.join(lines)+'\n')
result=subprocess.run([str(renderer),str(manifest)],capture_output=True,text=True,check=True);(out/'transfer-rendered.txt').write_text(result.stdout)
observations={}
for line in result.stdout.splitlines():
 p=line.split();observations[p[0]]=[list(map(int,p[1:4])),list(map(int,p[4:7]))]
for r in requests:
 v=observations[r['id']];r.update(glass=v[0],lens=v[1]);target=np.array(samples[r['sample']]['observed'])*255;r['native']=target.tolist();r['glassMAE255']=float(abs(target-v[0]).mean());r['lensMAE255']=float(abs(target-v[1]).mean());r['validation']=samples[r['sample']]['validation']
summary={k:float(np.mean([r[k] for r in requests if r['validation']])) for k in ['glassMAE255','lensMAE255']};print('held-out API pixel errors',summary)
(out/'api-transfer.json').write_text(json.dumps(dict(method=__doc__,rendererSHA256=hashlib.sha256(renderer.read_bytes()).hexdigest(),heldOut=summary,records=requests),indent=2)+'\n')
assert len(observations)==len(requests)
assert max(abs(np.array(r['glass'])-r['lens']).max() for r in requests)<=1
assert max(summary.values())<2
