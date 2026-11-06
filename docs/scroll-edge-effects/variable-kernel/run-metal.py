import subprocess,os,pathlib,shutil,sys
root=pathlib.Path('/Users/daniilvinogradov/Documents/Dev/NXKit');batch=pathlib.Path('/private/tmp/nxkit-variable-edge-metal');base=pathlib.Path('/private/tmp/nxkit-navigation-current-skia-raster')
mode=sys.argv[1];(batch/'logs').mkdir(parents=True,exist_ok=True)
if mode=='cold' and not (batch/'prior').exists():shutil.copytree(root/'shaders/apple/catalog',batch/'prior')
for scale in (1,2,3):
 for theme in ('light','dark'):
  name=f'{mode}-{scale}-{theme}';env=os.environ|{'NXKIT_METAL_CACHE_DIR':str(batch/name),'NXKIT_METAL_BUNDLE_DIR':str(batch/'no-bundle' if mode=='cold' else root/'Demo/build/macos/ThorVGApp-metal-macosx'),'NXKIT_METAL_BUNDLED_ONLY':'1','NXKIT_METAL_DIAGNOSTICS':'1'}
  if mode=='cold':env['NXKIT_METAL_HARVEST_DIR']=str(batch/'raw')
  else:env['NXKIT_METAL_REQUIRE_ARCHIVE_HIT']='1'
  with (batch/'logs'/f'{name}.log').open('w') as log:subprocess.run([str(base/'MetalScrollEdgeBenchmark'),str(batch/'logs'/f'{name}.csv'),'--scale',str(scale),'--'+theme],env=env,stdout=log,stderr=subprocess.STDOUT,check=True)
  print(name+' passed',flush=True)
for scale,theme in ((3,'dark'),(2,'light')):
 name=f'{mode}-navigation-{scale}-{theme}';env=os.environ|{'NXKIT_METAL_CACHE_DIR':str(batch/name),'NXKIT_METAL_BUNDLE_DIR':str(batch/'no-bundle' if mode=='cold' else root/'Demo/build/macos/ThorVGApp-metal-macosx'),'NXKIT_METAL_BUNDLED_ONLY':'1','NXKIT_METAL_DIAGNOSTICS':'1'}
 if mode=='cold':env['NXKIT_METAL_HARVEST_DIR']=str(batch/'raw')
 else:env['NXKIT_METAL_REQUIRE_ARCHIVE_HIT']='1'
 with (batch/'logs'/f'{name}.log').open('w') as log:subprocess.run([str(base/'MetalNavigationBenchmark'),str(batch/'logs'/f'{name}.csv'),'--full']+(['--light','--2x'] if theme=='light' else []),env=env,stdout=log,stderr=subprocess.STDOUT,check=True)
 print(name+' passed',flush=True)
