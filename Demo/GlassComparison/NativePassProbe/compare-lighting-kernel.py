"""Compare SDF shader only against independently native-lit/unlit registered pixels."""
from pathlib import Path
import json,re,subprocess,math,argparse
p=argparse.ArgumentParser();p.add_argument('renderer',type=Path);p.add_argument('report',type=Path);a=p.parse_args()
data=json.loads((a.report/'samples.json').read_text());records=[];lines=[]
for theme in ['dark','light']:
 if theme+'-no-highlight'not in data:continue
 for lit,unlit in zip(data[theme+'-baseline'],data[theme+'-no-highlight']):
  x,y,w,h=map(float,re.findall(r'[0-9.]+',lit['frame']));cx=x+w/2;cy=y+h/2
  for reference,backdrop in zip(lit['samples'],unlit['samples']):
   px,py=reference['pixel'];dx=(px+.5)/3-cx;dy=(py+.5)/3-cy;distance=math.hypot(dx,dy);sdf=distance-w/2
   if not -12<sdf<-.05:continue
   lines.append(f'{int(theme=="dark")} {w} {sdf} {dx/distance} {dy/distance} '+ ' '.join(map(str,backdrop['rgb'])))
   records.append({'theme':theme,'size':w,'gray':lit['gray'],'sdf':sdf,'angle':reference['angle'],'native':reference['rgb']})
output=subprocess.run([str(a.renderer.resolve())],input='\n'.join(lines)+'\n',text=True,capture_output=True,check=True)
rows=output.stdout.splitlines();assert len(rows)==len(records)
for record,row in zip(records,rows):
 record['portable']=list(map(int,row.split()));record['error']=sum(abs(x-y)for x,y in zip(record['native'],record['portable']))/3
report={'scope':'SDF highlight only; input backdrop is independent native no-highlight framebuffer','samples':records,'meanAbsoluteError':sum(x['error']for x in records)/len(records),'maximumChannelError':max(max(abs(a-b)for a,b in zip(x['native'],x['portable']))for x in records)}
interior=[x for x in records if x['sdf']<-.25]
report['fullyInside']={'minimumDepthPoints':.25,'sampleCount':len(interior),'meanAbsoluteError':sum(x['error']for x in interior)/len(interior),'maximumChannelError':max(max(abs(a-b)for a,b in zip(x['native'],x['portable']))for x in interior)}
(a.report/'kernel-comparison.json').write_text(json.dumps(report,indent=2)+'\n');print({k:v for k,v in report.items()if k!='samples'})
