"""Replay retained UIKit touches: NATIVE_DIR BEFORE_RENDERER AFTER_RENDERER OUTPUT.
No fitted offset/translation shifts; native timestamps drive the same input.
"""
from pathlib import Path
import json,sys,subprocess,hashlib,math,html
import numpy as np
native,before,after,out=map(Path,sys.argv[1:5]);out.mkdir(parents=True,exist_ok=True)
records=[]
for path in sorted(native.glob('*.json')):
 d=json.loads(path.read_text())
 if not isinstance(d,dict) or 'configuration' not in d:continue
 c=d['configuration'];name=c['id'];events=d['events'];frames=d['frames'];first=events[0]['sample'];stage=d['stage'];reset=c.get('setTranslation',[0,0]);target=c['target']=='scroll'
 lines=[f"{int(target)} {c.get('offset',0)} {d['minimum']} {d['maximum']} {int('setTranslation' in c)} {reset[0]} {reset[1]} {first[9]} {first[10]} {stage[0]} {stage[1]}"]
 timeline=[(e.get('deliveredAt',e['t']),0,i,'E',e) for i,e in enumerate(events)]+[(f[0],1,i,'F',f) for i,f in enumerate(frames)];timeline.sort(key=lambda x:x[:3])
 references=[]
 for t,_,i,kind,v in timeline:
  if kind=='E':lines.append('E '+str(t)+' '+str(len(v['touches']))+' '+' '.join(str(x) for row in v['touches'] for x in row));references.append([t]+v['sample'][1:])
  else:lines.append(f'F {t} {v[9]} {v[10]}');references.append(v)
 input_path=out/(name+'-input.txt');input_path.write_text('\n'.join(lines)+'\n');versions={}
 for label,exe in [('before',before),('after',after)]:
  result=out/(name+'-'+label+'.txt');subprocess.run([str(exe.resolve()),str(input_path.resolve()),str(result.resolve())],check=True,stdout=subprocess.DEVNULL)
  rows=[line.split() for line in result.read_text().splitlines()];assert len(rows)==len(references),(name,len(rows),len(references));versions[label]=np.array([[float(x) for x in r[1:]] for r in rows])
 a=np.array(references,float);scores={}
 for label,b in versions.items():
  frame=np.array([row[3]=='F' for row in timeline]);e=b[:,1:3]-a[:,1:3];tr=b[:,3:5]-a[:,3:5];vel=b[:,5:7]-a[:,5:7]
  scores[label]={'offsetRMSE':float(np.mean(np.sum(e[frame]**2,axis=1))**.5),'maxOffsetError':float(np.linalg.norm(e[frame],axis=1).max()),'translationRMSE':float(np.mean(np.sum(tr**2,axis=1))**.5),'velocityRMSE':float(np.mean(np.sum(vel**2,axis=1))**.5),'terminalOffset':b[-1,1:3].tolist()}
 # The runner can compact pointer slots during long FIFO overlap sequences.
 # Keep those captures but flag any unexpected all-up gap in an intended relay.
 expected_overlap=all(c['paths'][i]['lift']>=c['paths'][i+1]['points'][0][0] for i in range(len(c['paths'])-1)) if len(c['paths'])>1 else False
 all_up=[e['t'] for e in events[:-1] if not any(t[1] in (0,1,2) for t in e['touches'])]
 record={'id':name,'target':c['target'],'nativeEnd':a[-1,1:3].tolist(),'scores':scores,'unexpectedAllUpInRelay':all_up if expected_overlap else [],'nativeDelegates':d.get('delegates',[])};records.append(record)
 plotcols=[(2,'Content offset Y'),(4,'Pan translation Y'),(6,'Pan velocity Y')];parts=[]
 for col,title in plotcols:
  series=[('UIKit',a[:,0],a[:,col],'#f8f8f8'),('Before',versions['before'][:,0],versions['before'][:,col],'#ff8d69'),('After',versions['after'][:,0],versions['after'][:,col],'#50cff5')]
  low=min(float(v.min()) for _,_,v,_ in series);high=max(float(v.max()) for _,_,v,_ in series);span=max(1,high-low);duration=max(.1,float(a[-1,0]));paths=[]
  for label,t,v,color in series:
   points=' '.join(f'{30+float(x)/duration*900:.2f},{200-(float(y)-low)/span*170:.2f}' for x,y in zip(t,v));paths.append(f'<polyline points="{points}" fill="none" stroke="{color}" stroke-width="1.5"/>')
  parts.append(f'<h3>{title}</h3><svg viewBox="0 0 960 230" role="img" aria-label="{title}"><text x="30" y="18" fill="#aaa">{high:.1f}</text><text x="30" y="225" fill="#aaa">{low:.1f} · {duration:.2f}s</text>'+''.join(paths)+'</svg>')
 (out/(name+'.html')).write_text('<!doctype html><meta charset="utf-8"><style>body{font:15px system-ui;background:#16181b;color:#ddd;margin:24px}svg{width:100%;max-width:1100px;background:#22262b}a{color:#7cf}</style><a href="comparison.html">All cases</a><h1>'+html.escape(name)+'</h1><p>UIKit white · Before orange · After cyan. Native input/timestamps; no fitted registration.</p>'+''.join(parts)+'<pre>'+html.escape(json.dumps(record,indent=2))+'</pre>')
 print(name,'offset',round(scores['before']['offsetRMSE'],3),'->',round(scores['after']['offsetRMSE'],3),'translation',round(scores['before']['translationRMSE'],3),'->',round(scores['after']['translationRMSE'],3),flush=True)
(out/'metrics.json').write_text(json.dumps(records,indent=2)+'\n')
page='<!doctype html><meta charset="utf-8"><title>UIKit scroll comparison</title><style>body{font:16px system-ui;background:#17191d;color:#eee;margin:32px}a{color:#7cf}td,th{padding:9px 14px;border-bottom:1px solid #555;text-align:left}table{border-collapse:collapse}</style><h1>UIKit / NXKit pan and scrolling</h1><p>Real native OS touch input, replayed at its recorded timestamps. White: UIKit; orange: before; cyan: after. Read <a href="README.md">method and limitations</a>.</p><table><tr><th>Case</th><th>Offset RMSE before / after (pt)</th><th>Translation RMSE before / after (pt)</th></tr>'
for r in records:
 b,a=r['scores']['before'],r['scores']['after'];warning=' · input relay gap' if r['unexpectedAllUpInRelay'] else ''
 page+=f'<tr><td><a href="{r["id"]}.html">{r["id"]}</a>{warning}</td><td>{b["offsetRMSE"]:.3f} / {a["offsetRMSE"]:.3f}</td><td>{b["translationRMSE"]:.3f} / {a["translationRMSE"]:.3f}</td></tr>'
if (out/'reversal/comparison.html').exists():page=page.replace('<table>','<p><a href="reversal/comparison.html">Pull beyond an edge, then reverse — additional UIKit comparisons</a></p><table>',1)
(out/'comparison.html').write_text(page+'</table>')
(out/'provenance.json').write_text(json.dumps({'nativeSHA256':{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in native.glob('*.json')},'beforeRendererSHA256':hashlib.sha256(before.read_bytes()).hexdigest(),'afterRendererSHA256':hashlib.sha256(after.read_bytes()).hexdigest()},indent=2)+'\n')

if '--check' in sys.argv:
 limits={'scroll-flick':1,'scroll-catch-inertia':.5,'scroll-pull':2,'scroll-bounce-catch':6,'scroll-bounce-regrab':6,'scroll-resize-drag':1,'scroll-resize-bounce':3,'scroll-rotate-bounce':2,'scroll-slow-release':.5,'scroll-release-20':.5,'scroll-release-40':.5,'scroll-linear-249':5,'scroll-linear-250':5,'scroll-linear-251':5}
 indexed={r['id']:r for r in records}
 for name,limit in limits.items():
  value=indexed[name]['scores']['after']['offsetRMSE']
  if value>limit:raise AssertionError(f'{name}: {value:.3f} pt exceeds retained native replay limit {limit}')
 print(f'{len(limits)} native replay regression limits passed')
