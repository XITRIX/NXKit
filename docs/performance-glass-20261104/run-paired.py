#!/usr/bin/env python3
"""Run frozen/current release Metal workloads serially in alternating order."""
import argparse, csv, json, os, pathlib, statistics, subprocess
p=argparse.ArgumentParser();p.add_argument('--batch',type=pathlib.Path,required=True);p.add_argument('--build',type=pathlib.Path,required=True);p.add_argument('--bundle',type=pathlib.Path,required=True);a=p.parse_args()
out=a.batch/'paired';out.mkdir(exist_ok=True)
for workload in ('stress','navigation'):
 for scale,theme in ((3,'dark'),(2,'light')):
  for repeat in range(3):
   for version in (('before','after') if repeat%2==0 else ('after','before')):
    name=f'{workload}-{scale}-{theme}-{repeat}-{version}'
    executable=a.batch/'before'/f'{workload}-metal' if version=='before' else a.build/('MetalGlassStressPerformance' if workload=='stress' else 'MetalNavigationBenchmark')
    bundle=a.batch/'before-bundle' if version=='before' else a.bundle
    env=os.environ|{'NXKIT_METAL_BUNDLE_DIR':str(bundle),'NXKIT_METAL_CACHE_DIR':str(out/(name+'-cache')),'NXKIT_METAL_BUNDLED_ONLY':'1','NXKIT_METAL_REQUIRE_ARCHIVE_HIT':'1','MTL_DEBUG_LAYER':'0'}
    args=[str(executable),str(out/(name+'.csv'))]
    args+=['--scale',str(scale),'--frames','160','--passes','3'] if workload=='stress' else ['--full']
    if theme=='light':args+=['--light']+(['--2x'] if workload=='navigation' else [])
    with (out/(name+'.log')).open('w') as log:subprocess.run(args,env=env,stdout=log,stderr=subprocess.STDOUT,check=True)
    print(name+' passed',flush=True)
summary=[]
for scale,theme in ((3,'dark'),(2,'light')):
 for repeat in range(3):
  rows={v:list(csv.DictReader((out/f'stress-{scale}-{theme}-{repeat}-{v}.csv').open())) for v in ('before','after')}
  assert len(rows['before'])==len(rows['after'])==480
  for before,after in zip(rows['before'],rows['after']):
   assert all(before[k]==after[k] for k in ('pass','frame','phase','views','layers','glass'))
for workload in ('stress','navigation'):
 for scale,theme in ((3,'dark'),(2,'light')):
  for phase in (('idle','scroll') if workload=='stress' else ('rest','push','pop')):
   medians={}
   for version in ('before','after'):
    values={}
    for repeat in range(3):
     rows=list(csv.DictReader((out/f'{workload}-{scale}-{theme}-{repeat}-{version}.csv').open()))
     assert all(int(r.get('library_compiles',0))==0 and int(r.get('pipeline_compiles',0))==0 for r in rows)
     rows=[r for r in rows if int(r['pass'])>=1 and r['phase' if workload=='stress' else 'direction']==phase]
     metrics={'cpu_ms':[sum(float(r[k]) for k in ('layout_ms','render_ms','submit_ms')) for r in rows], 'total_ms':[float(r['total_ms']) for r in rows]} if workload=='stress' else {'cpu_ms':[float(r['cpu_ms']) for r in rows]}
     for key,v in metrics.items():values.setdefault(key,[]).append(statistics.median(v))
    medians[version]={k:statistics.median(v) for k,v in values.items()}
   summary.append({'workload':workload,'scale':scale,'theme':theme,'phase':phase,**medians,'changePercent':{k:100*(medians['after'][k]/v-1) for k,v in medians['before'].items()}})
(out/'summary.json').write_text(json.dumps(summary,indent=2));print(json.dumps(summary,indent=2))
