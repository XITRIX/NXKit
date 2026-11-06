"""Estimate relative Gaussian sigma from captured square-wave fundamental response."""
from pathlib import Path
import json,math,cmath,sys
root=Path(sys.argv[1]);d=json.loads((root/'scanlines.json').read_text());out={}
for preference in ['half','one']:
 rows=[]
 for radius in [0,1,2,5]:
  samples=d[f'{preference}-radius-{radius}'];ref=d[f'{preference}-radius-0'];estimates=[]
  for i,row in enumerate(samples):
   period=[4,8,16,32][i%4];omega=2*math.pi/(period*3)
   def amplitude(x):return 2*abs(sum(v*cmath.exp(-1j*omega*k)for k,v in enumerate(x)))/len(x)
   a=amplitude(row['samples']);a0=amplitude(ref[i]['samples']);ratio=a/a0 if a0>0 else None
   sigma=math.sqrt(-2*math.log(ratio))/(omega*3)if ratio and 0<ratio<=1 else None
   estimates.append({'periodPoints':period,'amplitude':a,'referenceAmplitude':a0,'ratio':ratio,'equivalentAddedSigmaPoints':sigma,'mean':row['mean'],'minimum':row['minimum'],'maximum':row['maximum']})
  rows.append({'nativeRadius':radius,'responses':estimates})
 out[preference]=rows
(root/'frequency-response.json').write_text(json.dumps(out,indent=2)+'\n')
for pref,rows in out.items():
 print(pref)
 for r in rows:print(r['nativeRadius'],[(x['periodPoints'],round(x['amplitude'],3),None if x['equivalentAddedSigmaPoints']is None else round(x['equivalentAddedSigmaPoints'],3))for x in r['responses'][:4]])
