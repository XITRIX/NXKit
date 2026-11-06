import subprocess,os,json,csv,shutil,argparse
from pathlib import Path
parser=argparse.ArgumentParser(description='Verify current ScrollPocket archives and compare rendering costs with a retained baseline.')
parser.add_argument('--batch',type=Path,required=True)
parser.add_argument('--build-dir',type=Path,required=True)
parser.add_argument('--before-binary',type=Path,required=True)
parser.add_argument('--before-bundle',type=Path,required=True)
parser.add_argument('--bundle',type=Path,required=True)
args=parser.parse_args()
batch=args.batch.resolve();base=args.build_dir.resolve();newbundle=args.bundle.resolve()
(batch/'logs').mkdir(parents=True,exist_ok=True)
def run_edge(name,scale,theme,before=False):
 binary=args.before_binary.resolve() if before else base/'MetalScrollEdgeBenchmark'
 bundle=args.before_bundle.resolve() if before else newbundle
 env=os.environ|{'NXKIT_METAL_CACHE_DIR':str(batch/name),'NXKIT_METAL_BUNDLE_DIR':str(bundle),'NXKIT_METAL_BUNDLED_ONLY':'1','NXKIT_METAL_REQUIRE_ARCHIVE_HIT':'1','NXKIT_METAL_DIAGNOSTICS':'1'}
 with (batch/'logs'/f'{name}.log').open('w') as log:subprocess.run([str(binary),str(batch/'logs'/f'{name}.csv'),'--scale',str(scale),'--'+theme],env=env,stdout=log,stderr=subprocess.STDOUT,check=True)
 report=json.loads((batch/'logs'/f'{name}.json').read_text());assert report['libraryCompiles']==report['pipelineCompiles']==report['failures']==0
 assert sum(p['frames'] for p in report['passes'])==975
for scale,theme in ((3,'dark'),(2,'light')):
 for repeat in range(3):
  for version in (('before','after') if repeat%2==0 else ('after','before')):
   name=f'ab-{version}-{scale}-{theme}-{repeat}';run_edge(name,scale,theme,version=='before');print(name+' passed',flush=True)
   if version=='after' and repeat==2:
    for suffix in ('.csv','.json','.log'):shutil.copyfile(batch/'logs'/(name+suffix),batch/'logs'/(f'strict-{scale}-{theme}'+suffix))
for scale in (1,2,3):
 for theme in ('light','dark'):
  name=f'strict-{scale}-{theme}'
  if (batch/'logs'/f'{name}.json').exists():continue
  run_edge(name,scale,theme);print(name+' passed',flush=True)
for scale,theme in ((3,'dark'),(2,'light')):
 name=f'strict-navigation-{scale}-{theme}'
 env=os.environ|{'NXKIT_METAL_CACHE_DIR':str(batch/name),'NXKIT_METAL_BUNDLE_DIR':str(newbundle),'NXKIT_METAL_BUNDLED_ONLY':'1','NXKIT_METAL_REQUIRE_ARCHIVE_HIT':'1','NXKIT_METAL_DIAGNOSTICS':'1'}
 args=[str(base/'MetalNavigationBenchmark'),str(batch/'logs'/f'{name}.csv'),'--catalog','--full','--interactive']+(['--light','--2x'] if theme=='light' else [])
 with (batch/'logs'/f'{name}.log').open('w') as log:subprocess.run(args,env=env,stdout=log,stderr=subprocess.STDOUT,check=True)
 with (batch/'logs'/f'{name}.csv').open() as source:rows=list(csv.DictReader(source))
 assert len(rows)==17497 and all(int(r['library_compiles'])==int(r['pipeline_compiles'])==0 for r in rows)
 print(name+' passed',flush=True)
print('Strict eight profiles and alternating paired performance runs passed',flush=True)
