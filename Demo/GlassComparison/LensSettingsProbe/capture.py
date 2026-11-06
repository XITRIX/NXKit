"""Capture applied UIKit lens specs/filters, with UUID freshness checks.
Private calls stay in LensReference.m; this driver uses Simulator only.
"""
import argparse,base64,gzip,hashlib,json,os,subprocess,time,uuid
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--device',required=True);p.add_argument('--fixture',type=Path,required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--cases',type=Path);p.add_argument('--tints',default='0,0.5,1');a=p.parse_args()
out=a.output.resolve();out.mkdir(parents=True,exist_ok=True)
app=a.fixture.resolve()/'LensReference.app';bundle='org.nxkit.LensReference'
def sim(*args,env=None):return subprocess.check_output(['xcrun','simctl',*args],env=env,text=True,timeout=120).strip()
sim('install',a.device,str(app))
cases=json.loads(a.cases.read_text()) if a.cases else [dict(id=f'style{style}-dark{int(dark)}-lift{int(lift)}-{w}x{h}',style=style,contentMode=style,dark=dark,lifted=lift,width=w,height=h,liftInset=0,selected=1,background='gray',settings=True) for style in (0,1) for dark in (False,True) for lift in (False,True) for w,h in [(37,24),(52,52),(116,56),(398,62),(80,80),(160,120)]]
index=[]
for tint in a.tints.split(','):
 env=dict(os.environ);env.pop('SIMCTL_CHILD_LENS_TINT_AMOUNT',None)
 if tint!='system':env['SIMCTL_CHILD_LENS_TINT_AMOUNT']=tint
 sim('launch','--terminate-running-process',a.device,bundle,env=env)
 documents=Path(sim('get_app_container',a.device,bundle,'data'))/'Documents'
 for case in cases:
  request=dict(case,settings=True,captureNonce=str(uuid.uuid4()))
  temp=documents/'request.tmp';temp.write_text(json.dumps(request));temp.replace(documents/'request.json')
  deadline=time.monotonic()+35
  while True:
   try:d=json.loads((documents/'capture.json').read_text())
   except (ValueError,FileNotFoundError):d={}
   if d.get('request')==request:break
   if time.monotonic()>deadline:raise RuntimeError('Missing fresh native acknowledgement '+case['id'])
   time.sleep(.08)
  lenses=d.get('lensSettings',[]);assert lenses and any(x['identifier']=='manual-lens' for x in lenses)
  if tint!='system':assert abs(d['effectiveTint']-float(tint))<1e-6,d['effectiveTint']
  d['nativeExecutableSHA256']=hashlib.sha256((app/'LensReference').read_bytes()).hexdigest()
  d['fixtureSources']=json.loads((app/'fixture-source.json').read_text())
  target=out/(tint+'-'+case['id']+'.json.gz')
  with gzip.open(target,'wt')as f:json.dump(d,f)
  index.append(dict(file=target.name,sha256=hashlib.sha256(target.read_bytes()).hexdigest(),request=request,effectiveTint=d['effectiveTint'],lenses=len(lenses),scale=d['scale'],os=d['osBuild']))
  (out/'index.json').write_text(json.dumps(index,indent=2)+'\n')
  print(target.name,len(lenses),'lens settings',flush=True)
print('Fresh settings captures:',len(index),flush=True)
