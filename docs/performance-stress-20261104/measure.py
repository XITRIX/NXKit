#!/usr/bin/env python3
"""Serial, alternating-order native Metal stress comparison. No frame sleeps."""
import argparse,csv,json,os,pathlib,statistics,subprocess
p=argparse.ArgumentParser();p.add_argument('--before',required=True,type=pathlib.Path);p.add_argument('--after',required=True,type=pathlib.Path);p.add_argument('--bundle',required=True,type=pathlib.Path);p.add_argument('--output',required=True,type=pathlib.Path);a=p.parse_args();a.output.mkdir(parents=True,exist_ok=True)
results={}
for scale,light in ((3,False),(2,True)):
 profile=f'{scale}-'+('light' if light else 'dark');results[profile]={}
 for repeat in range(3):
  for version in (('before','after') if repeat%2==0 else ('after','before')):
   name=f'{version}-{profile}-{repeat}';path=a.output/(name+'.csv')
   env=os.environ|{'NXKIT_METAL_BUNDLE_DIR':str(a.bundle.resolve()),'NXKIT_METAL_CACHE_DIR':str((a.output/('cache-'+name)).resolve()),'NXKIT_METAL_REQUIRE_ARCHIVE_HIT':'1','NXKIT_METAL_BUNDLED_ONLY':'1'}
   cmd=[str(getattr(a,version).resolve()),str(path),'--scale',str(scale),'--frames','160','--passes','3']+(['--light'] if light else [])
   with open(a.output/(name+'.log'),'w') as log:subprocess.run(cmd,env=env,stdout=log,stderr=subprocess.STDOUT,check=True)
   rows=list(csv.DictReader(open(path)));assert len(rows)==480
   assert all(int(x['library_compiles'])==int(x['pipeline_compiles'])==0 for x in rows)
   for x in rows:x['cpu_ms']=sum(float(x[k]) for k in ('layout_ms','render_ms','submit_ms'))
   for phase in ('idle','scroll'):
    selected=[x for x in rows if int(x['pass'])>0 and x['phase']==phase]
    result={}
    for key in ('layout_ms','render_ms','submit_ms','cpu_ms','sync_ms','total_ms','views','layers','glass'):
     values=sorted(float(x[key]) for x in selected)
     result[key]={'p50':statistics.median(values),'p95':values[int(.95*(len(values)-1))]}
    results[profile].setdefault(version,{}).setdefault(phase,[]).append(result)
   print(name,flush=True)
 for phase in ('idle','scroll'):
  for version in ('before','after'):
   values=results[profile][version][phase]
   results[profile][version][phase]={'repeats':values,'medianOfRepeatP50':{k:statistics.median(x[k]['p50'] for x in values) for k in values[0]}}
with open(a.output/'metrics.json','w') as f:json.dump(results,f,indent=2)
