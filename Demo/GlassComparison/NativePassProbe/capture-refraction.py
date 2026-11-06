"""Coordinate-coded native refraction capture, fresh filter copies and processes."""
from pathlib import Path
import subprocess,os,time,uuid,json,argparse
p=argparse.ArgumentParser();p.add_argument('--holding-tone',action='store_true');p.add_argument('--phase-two',action='store_true');p.add_argument('--device',required=True);p.add_argument('--output',required=True,type=Path);a=p.parse_args();a.output.mkdir(parents=True,exist_ok=True);bundle='org.nxkit.NativeGlassPassProbe';container=Path(subprocess.check_output(['xcrun','simctl','get_app_container',a.device,bundle,'data'],text=True).strip());source=container/'Documents/pass.json'
cases={'native':{},'identity':{'inputInnerRefractionAmount':0,'inputOuterRefractionAmount':0},'inner':{'inputOuterRefractionAmount':0},'outer':{'inputInnerRefractionAmount':0},'inner-small':{'inputOuterRefractionAmount':0,'inputInnerRefractionAmount':-8},'inner-height':{'inputOuterRefractionAmount':0,'inputInnerRefractionHeight':8},'outer-height':{'inputInnerRefractionAmount':0,'inputOuterRefractionHeight':8}}
if a.phase_two:
 cases={'opacity-one':{'inputRefractionOpacity':1},'outer-opacity-one':{'inputInnerRefractionAmount':0,'inputRefractionOpacity':1},'inner-opacity-one':{'inputOuterRefractionAmount':0,'inputRefractionOpacity':1},'outer-wide-mask':{'inputInnerRefractionAmount':0,'inputRefractionOpacity':1,'inputRefractionDistance0':-16,'inputRefractionDistance1':0},'native-blurred':{'inputRefractionOpacity':.6,'inputBlurRadius':5,'inputBlurOpacity1':1,'inputBlurOpacity2':1,'inputBlurOpacity3':1},'native-blurred-no-outer':{'inputOuterRefractionAmount':0,'inputRefractionOpacity':.6,'inputBlurRadius':5,'inputBlurOpacity1':1,'inputBlurOpacity2':1,'inputBlurOpacity3':1}}
if a.holding_tone:
 cases={'holding-half':{'inputInnerRefractionAmount':0,'inputOuterRefractionAmount':0,'inputSDRHoldingToneEnabled':True,'inputSDRHoldingToneWhite':.5,'inputSDRGradientDistance0':100,'inputSDRGradientDistance1':101},'holding-gradient':{'inputInnerRefractionAmount':0,'inputOuterRefractionAmount':0,'inputSDRHoldingToneEnabled':True,'inputSDRHoldingToneWhite':.5,'inputSDRGradientDistance0':-32,'inputSDRGradientDistance1':0}}
for axis in (['x'] if a.phase_two or a.holding_tone else ['x','y']):
 for name,changes in cases.items():
  run=str(uuid.uuid4());env=os.environ.copy();env.update(SIMCTL_CHILD_PASS_RUN_ID=run,SIMCTL_CHILD_PASS_TINT_AMOUNT='.5',SIMCTL_CHILD_PASS_CHANGES=json.dumps(changes),SIMCTL_CHILD_PASS_AXIS=axis)
  subprocess.run(['xcrun','simctl','launch','--terminate-running-process',a.device,bundle],env=env,stdout=subprocess.DEVNULL,check=True,timeout=30);deadline=time.monotonic()+25
  while time.monotonic()<deadline:
   try:
    d=json.loads(source.read_text())
    if d['runID']==run:break
   except(ValueError,FileNotFoundError):pass
   time.sleep(.1)
  else:raise RuntimeError('No fresh refraction capture')
  assert len(d['mutations'])==5 and not any('error'in v for m in d['mutations']for v in m.values()),d
  target=a.output/f'{axis}-{name}';target.with_suffix('.json').write_text(json.dumps(d,indent=2)+'\n');subprocess.run(['xcrun','simctl','io',a.device,'screenshot',str(target.with_suffix('.png'))],check=True,capture_output=True,timeout=20);print(target.name,flush=True)
