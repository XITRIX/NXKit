"""Serial paired public-scene runs; requires distinct matching before/after bundles."""
import os,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[3];batch=pathlib.Path('/private/tmp/nxkit-pocket-easing');logs=batch/'performance';logs.mkdir(exist_ok=True)
for scale,theme in ((3,'dark'),(2,'light')):
 for repeat in range(3):
  for version in (('before','after') if repeat%2==0 else ('after','before')):
   name=f'ab-{version}-{scale}-{theme}-{repeat}'
   binary=batch/'before-metal' if version=='before' else batch/'raster/MetalScrollEdgeBenchmark'
   bundle=batch/'before-bundle' if version=='before' else root/'Demo/build/macos/ThorVGApp-metal-macosx'
   env=os.environ|{'NXKIT_METAL_BUNDLE_DIR':str(bundle),'NXKIT_METAL_CACHE_DIR':str(batch/('cache-'+name)),'NXKIT_METAL_REQUIRE_ARCHIVE_HIT':'1','NXKIT_METAL_BUNDLED_ONLY':'1'}
   with open(logs/(name+'.log'),'w') as log:subprocess.run([str(binary),str(logs/(name+'.csv')),'--scale',str(scale),'--'+theme],env=env,stdout=log,stderr=subprocess.STDOUT,check=True)
   print(name,flush=True)
subprocess.run([sys.executable,str(root/'Demo/ScrollEdgeComparison/analyze-pocket-performance.py'),str(logs),str(batch/'performance.json')],check=True)
