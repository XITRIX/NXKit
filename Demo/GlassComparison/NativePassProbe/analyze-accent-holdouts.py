from pathlib import Path
import sys,gzip,json,base64,math
repo=Path(__file__).resolve().parents[3];sys.path.insert(0,str(repo/'Demo/GlassComparison/SettingsProbe'));from decode import unpack
root=repo/'docs/liquid-glass-native-settings/color-transfer';palette=json.loads((repo/'docs/liquid-glass-native-settings/pass-semantics/accent-palette.json').read_text())['samples']
def walk(x):
 if isinstance(x,dict):
  yield x
  for v in x.values():yield from walk(v)
 elif isinstance(x,list):
  for v in x:yield from walk(v)
def mats(d):
 return [unpack(base64.b64decode(x['archiveBase64']))['CAFilterInputs'] for x in walk(d['layerTree']) if x.get('name')=='vibrantColorMatrix' and'archiveBase64'in x and unpack(base64.b64decode(x['archiveBase64']))['CAFilterInputs'].get('inputBackdropAware')]
def vec(m):return[m['inputColorMatrix'][f'm{i}{j}']for i in range(1,5)for j in range(1,6)]
def hsv(c):
 h=max(c);l=min(c);delta=h-l;hh=0
 if delta:
  if h==c[0]:hh=((c[1]-c[2])/delta)%6
  elif h==c[1]:hh=(c[2]-c[0])/delta+2
  else:hh=(c[0]-c[1])/delta+4
 return[hh/6,delta/h if h else 0,h]
def distance(a,b):
 ah,bh=hsv(a),hsv(b);v=min(ah[2],bh[2]);h=2*min(abs(ah[0]-bh[0]),1-abs(ah[0]-bh[0]))*min(ah[1],bh[1])*v
 return sum((c-d)**2 for c,d in zip(a,b))+.25*h*h+.1*((ah[1]-bh[1])*v)**2+.1*(ah[2]-bh[2])**2
def interp(samples,c,constraint,anchor):
 sums=[0]*20;ws=0;isgray=max(c)-min(c)<1e-6
 if constraint and isgray:
  gray=sorted([s for s in samples if max(s['rgb'])-min(s['rgb'])<1e-6],key=lambda s:s['rgb'][0])
  lower=max((s for s in gray if s['rgb'][0]<=c[0]),key=lambda s:s['rgb'][0]);upper=min((s for s in gray if s['rgb'][0]>=c[0]),key=lambda s:s['rgb'][0]);t=(c[0]-lower['rgb'][0])/(upper['rgb'][0]-lower['rgb'][0])
  return[a*(1-t)+b*t for a,b in zip(vec(lower['vibrant']),vec(upper['vibrant']))]
 for s in samples:
  sg=max(s['rgb'])-min(s['rgb'])<1e-6
  if constraint and sg!=isgray:continue
  dd=distance(c,s['rgb']);w=1/(dd*dd);ws+=w
  for i,x in enumerate(vec(s['vibrant'])):sums[i]+=w*x
 out=[x/ws for x in sums]
 if constraint and not isgray:
  for i in range(3):out[i*5+4]=c[i]-anchor*sum(out[i*5+j]for j in range(3))
 return out
for appearance in['dark','light']:
 d=json.load(gzip.open(root/('holdout-'+appearance+'.json.gz')));ms=mats(d);assert len(ms)==len(d['surfaces'])
 for s,m in zip(d['surfaces'],ms):
  samples=[x for x in palette if x['appearance']==appearance and x['style']==s['style']];c=s['resolvedTint']['rgba'][:3];native=vec(m);anchor=.15 if appearance=='dark'and s['style']=='tinted'else 1
  errors=[]
  for constraint in[False,True]:
   predicted=interp(samples,c,constraint,anchor);ds=[]
   for y in[0,.125,.25,.5,.75,.875,1]:
    for row in range(3):
     a=sum(predicted[row*5+j]*y for j in range(3))+predicted[row*5+4];b=sum(native[row*5+j]*y for j in range(3))+native[row*5+4];ds.append(abs(min(max(a,0),1)-min(max(b,0),1))*255)
   errors.append((round(sum(ds)/len(ds),3),round(max(ds),3)))
  actualanchor=[sum(native[i*5+j]*anchor for j in range(3))+native[i*5+4]for i in range(3)]
  print(appearance,s['style'],s['name'],'old/new',errors,'anchorerror',round(max(abs(x-y)for x,y in zip(c,actualanchor)),8))
