"""One fresh process per perturbation; only process-local CAFilter values change."""
from pathlib import Path
import subprocess,os,time,uuid,json,argparse
p=argparse.ArgumentParser();p.add_argument('--device',required=True);p.add_argument('--output',required=True,type=Path);p.add_argument('--light',action='store_true');p.add_argument('--only');p.add_argument('--blur-matrix',action='store_true');a=p.parse_args();a.output.mkdir(parents=True,exist_ok=True)
bundle='org.nxkit.NativeGlassPassProbe';container=Path(subprocess.check_output(['xcrun','simctl','get_app_container',a.device,bundle,'data'],text=True).strip());source=container/'Documents/pass.json'
cases={'baseline':{},'face-black-0':{'inputFaceColorMatrixBlack':0},'face-white-1':{'inputFaceColorMatrixWhite':1},'saturation-0':{'inputFaceColorMatrixSaturation':0},'fill-red-half':{'inputFaceColorMatrixFillColor':[1,0,0,.5]},'max-luma-point2':{'inputFaceColorMatrixMaxLuma':.2,'inputFaceColorMatrixMaxLumaSDR':.2},'normal-0':{'inputBlurFillNormalOpacity':0},'normal-1':{'inputBlurFillNormalOpacity':1},'lighten-0':{'inputBlurFillLightenOpacity':0},'lighten-1':{'inputBlurFillLightenOpacity':1},'darken-0':{'inputBlurFillDarkenOpacity':0},'darken-1':{'inputBlurFillDarkenOpacity':1},'blur-0':{'inputBlurRadius':0},'blur-20':{'inputBlurRadius':20},'fill-blur-0':{'inputBlurFillBlurRadius':0},'fill-blur-20':{'inputBlurFillBlurRadius':20},'face-identity':{'inputFaceColorMatrixBlack':0,'inputFaceColorMatrixWhite':1,'inputFaceColorMatrixSaturation':1,'inputFaceColorMatrixFillColor':[0,0,0,0],'inputFaceColorMatrixMaxLuma':1,'inputFaceColorMatrixMaxLumaSDR':1},'fill-black-half':{'inputFaceColorMatrixFillColor':[0,0,0,.5]},'fill-white-half':{'inputFaceColorMatrixFillColor':[1,1,1,.5]},'face-opacity-0':{'inputFaceOpacity':0}}
cases.update({'max-luma-only':{'inputFaceColorMatrixMaxLuma':.2},'max-luma-sdr-only':{'inputFaceColorMatrixMaxLumaSDR':.2}})
if a.blur_matrix:cases={'tint-'+('half'if amount=='.5'else amount):{'__tint':amount}for amount in ['0','.5','1']}
for name,changes in cases.items():
 if a.only and name not in a.only.split(','):continue
 run=str(uuid.uuid4());env=os.environ.copy();
 if a.blur_matrix:env['SIMCTL_CHILD_PASS_BLUR_MATRIX']='1';env['SIMCTL_CHILD_PASS_TINT_AMOUNT']=changes.pop('__tint')
 env['SIMCTL_CHILD_PASS_RUN_ID']=run;env['SIMCTL_CHILD_PASS_CHANGES']=json.dumps(changes)
 if a.light:env['SIMCTL_CHILD_PASS_LIGHT']='1'
 subprocess.run(['xcrun','simctl','launch','--terminate-running-process',a.device,bundle],env=env,stdout=subprocess.DEVNULL,check=True,timeout=30)
 deadline=time.monotonic()+20
 while time.monotonic()<deadline:
  try:
   d=json.loads(source.read_text())
   if d['runID']==run:break
  except(FileNotFoundError,ValueError):pass
  time.sleep(.1)
 else:raise RuntimeError('No fresh ready signal')
 assert len(d['mutations'])==8 and not any('error'in v for m in d['mutations']for v in m.values()),d
 out=a.output/name;out.with_suffix('.json').write_text(json.dumps(d,indent=2)+'\n');subprocess.run(['xcrun','simctl','io',a.device,'screenshot',str(out.with_suffix('.png'))],check=True,capture_output=True,timeout=20);print(name,flush=True)
