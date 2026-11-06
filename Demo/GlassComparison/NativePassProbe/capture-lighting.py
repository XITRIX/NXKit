"""Fresh-process, framebuffer-only lighting isolation. Requires LightingProbe.app installed."""
from pathlib import Path
import subprocess,os,time,uuid,json,argparse,hashlib
p=argparse.ArgumentParser();p.add_argument('--device',required=True);p.add_argument('--output',required=True,type=Path);p.add_argument('--only');p.add_argument('--uniform');p.add_argument('--sdf',action='store_true');p.add_argument('--light-only',action='store_true');p.add_argument('--dark-only',action='store_true');a=p.parse_args();a.output.mkdir(parents=True,exist_ok=True)
bundle='org.nxkit.NativeGlassPassProbe';container=Path(subprocess.check_output(['xcrun','simctl','get_app_container',a.device,bundle,'data'],text=True).strip());signal=container/'Documents/pass.json'
cases={
 'baseline':({},'normal'), 'no-highlight':({},'off'), 'no-highlight-matrix':({},'noMatrix'),
 'no-ring':({'inputRingShadowOpacity':0},'normal'),
 'no-bleed':({'inputBleedOpacity':0},'normal'),
 'bleed-radius0':({'inputBleedBlurRadius':0},'normal'),
 'no-elevation':({'inputShadowOpacity':0,'inputSDRShadowOpacity':0},'normal'),
 'no-filter-highlight':({'inputKeyFillHighlightAmount':0},'normal'),
 'no-internal-shadow':({'inputShadowAmount':0},'normal'),
 'ring-only':({'inputBleedOpacity':0,'inputShadowOpacity':0,'inputSDRShadowOpacity':0,'inputKeyFillHighlightAmount':0,'inputShadowAmount':0},'off'),
 'no-lighting':({'inputRingShadowOpacity':0,'inputBleedOpacity':0,'inputShadowOpacity':0,'inputSDRShadowOpacity':0,'inputKeyFillHighlightAmount':0,'inputShadowAmount':0},'off'),
}
if a.sdf:
 cases={
  'sdf-no-diffuse':({},'normal',{'diffuseAmountScale':0}),
  'sdf-zero':({},'normal',{'keyAmount':0,'fillAmount':0,'diffuseAmountScale':0}),
  'sdf-key-only':({},'normal',{'fillAmount':0,'diffuseAmountScale':0}),
  'sdf-key-height2':({},'normal',{'fillAmount':0,'diffuseAmountScale':0,'keyHeight':2}),
  'sdf-key-curvature0':({},'normal',{'fillAmount':0,'diffuseAmountScale':0,'curvature':0}),
  'sdf-key-curvature1':({},'normal',{'fillAmount':0,'diffuseAmountScale':0,'curvature':1}),
 }
for light in ([True] if a.light_only else [False] if a.dark_only else [False,True]):
 for name,settings in cases.items():
  changes,highlight=settings[:2];sdf=settings[2] if len(settings)>2 else {}
  if a.only and name not in a.only.split(','):continue
  run=str(uuid.uuid4());env=os.environ.copy();env['SIMCTL_CHILD_PASS_RUN_ID']=run;env['SIMCTL_CHILD_PASS_TINT_AMOUNT']='.5';env['SIMCTL_CHILD_PASS_CHANGES']=json.dumps(changes);env['SIMCTL_CHILD_PASS_HIGHLIGHT']=highlight
  if light:env['SIMCTL_CHILD_PASS_LIGHT']='1'
  if sdf:env['SIMCTL_CHILD_PASS_SDF_CHANGES']=json.dumps(sdf)
  if a.uniform:env['SIMCTL_CHILD_PASS_UNIFORM_GRAY']=a.uniform
  subprocess.run(['xcrun','simctl','launch','--terminate-running-process',a.device,bundle],env=env,check=True,stdout=subprocess.DEVNULL,timeout=30)
  deadline=time.monotonic()+25
  while time.monotonic()<deadline:
   try:
    data=json.loads(signal.read_text())
    if data['runID']==run:break
   except(FileNotFoundError,ValueError):pass
   time.sleep(.1)
  else:raise RuntimeError('No fresh lighting signal')
  assert len(data['frames'])==9 and sum('highlight'in x for x in data['mutations'])==9 and sum('changes'in x for x in data['mutations'])==9,data
  assert not any('error'in x for x in data['mutations']),data
  target=a.output/(('light'if light else'dark')+'-'+('uniform-'+a.uniform.replace('.','p')+'-'if a.uniform else '')+name)
  target.with_suffix('.json').write_text(json.dumps(data,indent=2)+'\n')
  subprocess.run(['xcrun','simctl','io',a.device,'screenshot',str(target.with_suffix('.png'))],check=True,capture_output=True,timeout=20)
  print(target.name,flush=True)
