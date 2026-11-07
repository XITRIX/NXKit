from pathlib import Path
import json,sys
source=Path(sys.argv[1]);rows=[]
for path in sorted(source.glob('*/trace.json')):
 d=json.loads(path.read_text());events=d['events'];touch=next((e for e in events if e['kind']=='window-before' and e['phase']==0),None)
 if not touch:continue
 t0=touch['t'];initial=touch['point'];frames=[f for f in d['frames'] if f['t']>=t0]
 timeline=[]
 for e in events:
  if e['t']<t0 or e['kind'] in ['window-after','control-touchesMoved','continueTracking']:continue
  if e['kind']=='window-before' and e['phase']==1:continue
  timeline.append({**e,'dt_ms':round((e['t']-t0)*1000,2)})
 for field in ['tracking','highlighted']:
  previous=False
  for f in frames:
   if f[field]!=previous:timeline.append({'kind':field,'value':f[field],'dt_ms':round((f['t']-t0)*1000,2)});previous=f[field]
 states={};lifted=None
 for f in frames:
  for g in f['gestures']:
   if states.get(g['id'],0)!=g['state']:
    timeline.append({'kind':'gr-state','class':g['class'],'state':g['state'],'dt_ms':round((f['t']-t0)*1000,2),'translation':g.get('translation')});states[g['id']]=g['state']
  for lens in f['lenses']:
   if lens.get('lifted')!=lifted:timeline.append({'kind':'lens-lifted','value':lens.get('lifted'),'dt_ms':round((f['t']-t0)*1000,2)});lifted=lens.get('lifted')
 timeline.sort(key=lambda e:e['dt_ms'])
 row={'id':d['config']['id'],'touchStart':initial,'initialValue':frames[0]['value'] if frames else None,'finalValue':frames[-1]['value'] if frames else None,'finalScroll':frames[-1]['scrollOffset'] if frames else None,'timeline':timeline};rows.append(row)
print(json.dumps(rows,indent=2))
