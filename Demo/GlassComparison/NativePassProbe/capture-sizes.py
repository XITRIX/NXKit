from pathlib import Path
import argparse,subprocess,os,uuid,json,time,gzip
p=argparse.ArgumentParser();p.add_argument('--device',required=True);p.add_argument('--output',required=True,type=Path);p.add_argument('--intermediate',action='store_true');a=p.parse_args();a.output.mkdir(parents=True,exist_ok=True);bundle='org.nxkit.NativeGlassPassProbe';container=Path(subprocess.check_output(['xcrun','simctl','get_app_container',a.device,bundle,'data'],text=True).strip());source=container/'Documents/sizes.json'
for light in [False,True]:
 for tint in (['.25','.75']if a.intermediate else ['0','.5','1']):
  run=str(uuid.uuid4());env=os.environ.copy();env['SIMCTL_CHILD_PROBE_RUN_ID']=run;env['SIMCTL_CHILD_PROBE_TINT_AMOUNT']=tint
  if light:env['SIMCTL_CHILD_PROBE_LIGHT']='1'
  subprocess.run(['xcrun','simctl','launch','--terminate-running-process',a.device,bundle],env=env,stdout=subprocess.DEVNULL,check=True,timeout=30)
  deadline=time.monotonic()+25
  while time.monotonic()<deadline:
   try:
    d=json.loads(source.read_text())
    if d['runID']==run:break
   except(ValueError,FileNotFoundError):pass
   time.sleep(.2)
  else:raise RuntimeError('No fresh size capture')
  assert len(d['surfaces'])==42
  def count_filters(x):
   if isinstance(x,dict):return int(x.get('name')=='glassBackground'and'archiveBase64'in x)+sum(count_filters(v)for v in x.values())
   if isinstance(x,list):return sum(count_filters(v)for v in x)
   return 0
  assert count_filters(d['layerTree'])==42,count_filters(d['layerTree'])
  name=('light'if light else'dark')+'-'+({'.25':'quarter','.5':'half','.75':'threequarter'}.get(tint,tint))
  (a.output/(name+'.json.gz')).write_bytes(gzip.compress(json.dumps(d).encode()));print(name,flush=True)
