from pathlib import Path
import subprocess,os,json,time,csv,gzip,hashlib,statistics
root=Path('/Users/daniilvinogradov/Documents/Dev/NXKit');out=root/'docs/scroll-edge-effects/variable-kernel';batch=Path('/private/tmp/nxkit-variable-optimized-performance');logs=batch/'logs';logs.mkdir(parents=True,exist_ok=True)
for _ in range(600):
 if '** BUILD SUCCEEDED **' in Path('/private/tmp/nxkit-variable-app-ios.log').read_text() and 'strict-navigation-2-light passed' in Path('/private/tmp/nxkit-variable-strict.log').read_text():break
 time.sleep(1)
else:raise RuntimeError('iOS build did not finish before timing')
for scale,theme in ((3,'dark'),(2,'light')):
 for repeat in range(3):
  for version in (('before','after') if repeat%2==0 else ('after','before')):
   before=version=='before';name=f'{version}-{scale}-{theme}-{repeat}'
   binary=Path('/private/tmp/nxkit-variable-edge-before/benchmark') if before else Path('/private/tmp/nxkit-navigation-current-skia-raster/MetalScrollEdgeBenchmark')
   cache=Path('/private/tmp/nxkit-variable-edge-before')/f'cache-{scale}-{theme}' if before else Path('/private/tmp/nxkit-variable-edge-metal')/f'strict-{scale}-{theme}'
   bundle=Path('/private/tmp/nxkit-variable-edge-before/no-bundle') if before else root/'Demo/build/macos/ThorVGApp-metal-macosx'
   env=os.environ|{'NXKIT_METAL_CACHE_DIR':str(cache),'NXKIT_METAL_BUNDLE_DIR':str(bundle),'NXKIT_METAL_BUNDLED_ONLY':'1'}
   if not before:env['NXKIT_METAL_REQUIRE_ARCHIVE_HIT']='1'
   with (logs/(name+'.log')).open('w') as log:subprocess.run([str(binary),str(logs/(name+'.csv')),'--scale',str(scale),'--'+theme],env=env,stdout=log,stderr=subprocess.STDOUT,check=True)
   r=json.loads((logs/(name+'.json')).read_text());assert r['failures']==0 and all(p['libraryCompiles']==p['pipelineCompiles']==0 for p in r['passes'][1:])
   if not before:assert r['libraryCompiles']==r['pipelineCompiles']==0
   print(name+' passed',flush=True)
profiles=[]
for scale,theme in ((3,'dark'),(2,'light')):
 versions={}
 for version in ('before','after'):
  selected=[]
  for repeat in range(3):
   rows=list(csv.DictReader((logs/f'{version}-{scale}-{theme}-{repeat}.csv').open()));assert len(rows)==975
   selected.extend(r for r in rows if int(r['pass'])>=1 and r['case'] in ('soft-discovered','automatic-discovered') and r['phase']=='vertical')
  versions[version]={'frames':len(selected),'cpuMedianMS':statistics.median(float(r['cpu_ms']) for r in selected),'synchronizedTotalMedianMS':statistics.median(float(r['total_ms']) for r in selected)}
 a,b=versions['before'],versions['after'];profiles.append({'scale':scale,'theme':theme,'versions':versions,'cpuMedianChangePercent':100*(b['cpuMedianMS']/a['cpuMedianMS']-1),'synchronizedTotalMedianChangePercent':100*(b['synchronizedTotalMedianMS']/a['synchronizedTotalMedianMS']-1)})
artifacts={}
for p in logs.iterdir():
 dest=out/'performance'/ (p.name+'.gz');dest.parent.mkdir(exist_ok=True);dest.write_bytes(gzip.compress(p.read_bytes(),mtime=0));artifacts[str(dest.relative_to(out))]=hashlib.sha256(dest.read_bytes()).hexdigest()
report={'scope':'Three alternating before/after repetitions on local macOS Graphite/Metal, after both app builds finish; this is not a phone power/thermal or sustained frame pacing test.','fixture':'402x874pt four-edge chrome scene; automatic/soft discovered vertical scrolling; last two passes, 84 selected frames per version/profile.','before':'Current 4pt masked-uniform blur at one pixel per point; retained exact-source binary.','after':'Current 4pt variable kernel; full resolution at 1x/2x, antialiased coarse-kernel input and fused 17-tap finishing at 3x, fine kernels retained at display resolution.','repeatOrder':['before-after','after-before','before-after'],'profiles':profiles,'compilationPolicy':'Both versions require zero compilation during measured passes 1/2. Before uses its original warm cache and can compile during excluded pass 0; after uses the newly baked offline archive and requires zero compilation over every pass.','artifactsSHA256':artifacts}
(out/'performance.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(profiles,indent=2))
