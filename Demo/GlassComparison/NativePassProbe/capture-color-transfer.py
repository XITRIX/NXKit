"""Reproduce native color-transfer controls in fresh process-local tint contexts.

Run only on a simulator dedicated to this research: the installed probe bundle
is replaced between the neutral and accent apps. No system defaults are saved.
"""
from pathlib import Path
import argparse,gzip,json,os,subprocess,time,uuid
p=argparse.ArgumentParser();p.add_argument('--device',required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args();a.output.mkdir(parents=True,exist_ok=True)
bundle='org.nxkit.NativeGlassPassProbe';here=Path(__file__).resolve().parent
cases=[('dark-baseline',{}),('dark-no-bleed',{'PROBE_NO_BLEED':'1'}),
       ('light-baseline',{'PROBE_LIGHT':'1'}),('light-no-bleed',{'PROBE_LIGHT':'1','PROBE_NO_BLEED':'1'}),
       ('dark-saturation0',{'PROBE_NO_BLEED':'1','PROBE_FACE_SATURATION':'0'}),
       ('dark-saturation0-white05',{'PROBE_NO_BLEED':'1','PROBE_FACE_SATURATION':'0','PROBE_FACE_WHITE':'.5'}),
       ('dark-black0',{'PROBE_NO_BLEED':'1','PROBE_FACE_BLACK':'0'}),
       ('dark-black025',{'PROBE_NO_BLEED':'1','PROBE_FACE_BLACK':'.25'}),
       ('dark-saturation1',{'PROBE_NO_BLEED':'1','PROBE_FACE_SATURATION':'1'}),
       ('dark-saturation2',{'PROBE_NO_BLEED':'1','PROBE_FACE_SATURATION':'2'})]
for source,filename,variants in [('NeutralColorProbe.m','neutral.json',cases),('AccentHoldoutProbe.m','holdout.json',[('holdout-dark',{}),('holdout-light',{'PROBE_LIGHT':'1'})])]:
 build=a.output/('build-'+Path(source).stem)
 subprocess.run(['python3',str(here/'build.py'),str(build),'--source',str(here/source)],check=True)
 subprocess.run(['xcrun','simctl','install',a.device,str(build/'Probe.app')],check=True,timeout=30)
 container=Path(subprocess.check_output(['xcrun','simctl','get_app_container',a.device,bundle,'data'],text=True).strip());data=container/'Documents'/filename
 for name,changes in variants:
  run=str(uuid.uuid4());env=os.environ.copy()
  for key in list(env):
   if key.startswith('SIMCTL_CHILD_PROBE_'):del env[key]
  env['SIMCTL_CHILD_PROBE_RUN_ID']=run;env['SIMCTL_CHILD_PROBE_TINT_AMOUNT']='.5'
  for key,value in changes.items():env['SIMCTL_CHILD_'+key]=value
  subprocess.run(['xcrun','simctl','launch','--terminate-running-process',a.device,bundle],env=env,check=True,timeout=30,stdout=subprocess.DEVNULL)
  deadline=time.monotonic()+20
  while time.monotonic()<deadline:
   try:
    result=json.loads(data.read_text())
    if result['runID']==run:break
   except(FileNotFoundError,ValueError):pass
   time.sleep(.1)
  else:raise RuntimeError('No fresh probe result for '+name)
  assert result['effectiveTint']==.5
  (a.output/(name+'.json.gz')).write_bytes(gzip.compress(json.dumps(result).encode()))
  if source=='NeutralColorProbe.m':subprocess.run(['xcrun','simctl','io',a.device,'screenshot',str(a.output/(name+'.png'))],check=True,timeout=30,capture_output=True)
  print(name,flush=True)
 subprocess.run(['xcrun','simctl','terminate',a.device,bundle],check=True,timeout=30)
