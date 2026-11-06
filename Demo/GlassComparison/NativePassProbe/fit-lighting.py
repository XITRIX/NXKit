import json,math,re,random
root='docs/liquid-glass-native-settings/lighting-spatial/'
data=json.load(open(root+'samples.json'));profiles=json.load(open('docs/liquid-glass-native-settings/pass-semantics/all-styles-profiles.json'))['profiles']
def find(o,kind):
 if isinstance(o,dict):
  if o.get('class')==kind:return o
  for v in o.values():
   r=find(v,kind)
   if r:return r
 elif isinstance(o,list):
  for v in o:
   r=find(v,kind)
   if r:return r

def matrix(o):
 if isinstance(o,dict):
  if o.get('CAFilterType')=='vibrantColorMatrix':return o['CAFilterInputs']['inputColorMatrix']
  for v in o.values():
   r=matrix(v)
   if r:return r
 elif isinstance(o,list):
  for v in o:
   r=matrix(v)
   if r:return r

def angular(nx,ny,angle,spread):
 cosine=max(-1,min(1,nx*math.cos(angle-math.pi/2)+ny*math.sin(angle-math.pi/2)))
 t=max(0,min(1,1-math.acos(cosine)/spread));return t*t*(3-2*t)
points=[]
for theme in ['dark','light']:
 profile=next(x for x in profiles if x['appearance']==theme and x['requestedTint']=='0.5'and x['style']=='regular'and x['geometry']['width']==44 and x['geometry']['height']==44)
 h=find(profile['layerTree'],'CASDFKeyFillHighlightEffect');m=matrix(profile['layerTree'])
 for lit,unlit in zip(data[theme+'-baseline'],data[theme+'-no-highlight']):
  x,y,w,hh=map(float,re.findall(r'[0-9.]+',lit['frame']));cx=x+w/2;cy=y+hh/2
  for a,b in zip(lit['samples'],unlit['samples']):
   px,py=a['pixel'];dx=(px+.5)/3-cx;dy=(py+.5)/3-cy;r=math.hypot(dx,dy);d=w/2-r
   if not .25<d<12:continue
   nx=dx/r;ny=dy/r
   key=angular(nx,ny,h['keyAngle'],h['keySpread'])*h['keyAmount']+angular(nx,ny,h['fillAngle'],h['fillSpread'])*h['fillAmount']
   diff=h['diffuseAmountScale']*(angular(nx,ny,h['keyAngle'],h['keySpread']*h['diffuseSpreadScale'])*h['keyAmount']+angular(nx,ny,h['fillAngle'],h['fillSpread']*h['diffuseSpreadScale'])*h['fillAmount'])
   B=b['rgb'][0];C=a['rgb'][0];mapped=max(0,min(255,(m['m11']+m['m12']+m['m13'])*B+m['m14']*255+m['m15']*255));D=mapped-B
   points.append((theme,w,d,key,diff,B,C,D))
train=[p for p in points if p[0]=='dark' and p[1]==44]
def pred(p,v):
 _,_,d,key,diff,B,C,D=p
 kg,kh,ex,dg,dh,de=v
 R=kg*key*max(0,1-d/kh)**ex+dg*diff*math.exp(-(d/dh)**de)
 return max(0,min(255,B+R*D))
def loss(v,pts=train):return sum((pred(p,v)-p[6])**2 for p in pts)/len(pts)
bounds=[(.1,8),(.6,2.2),(.1,4),(.1,12),(.2,10),(.2,4)]
v=[2.8,1.25,1,6,2.1,1];best=loss(v)
random.seed(0)
for loop in range(5000):
 i=random.randrange(6);scale=.2*(.15+.85*(1-loop/5000));cand=v.copy();cand[i]*=math.exp(random.gauss(0,scale));cand[i]=max(bounds[i][0],min(bounds[i][1],cand[i]));l=loss(cand)
 if l<best:best=l;v=cand
print('params',v,'rmse',best**.5,'trainmae',sum(abs(pred(p,v)-p[6])for p in train)/len(train))
for theme in ['dark','light']:
 for size in [44,80,120]:
  pts=[p for p in points if p[0]==theme and p[1]==size];errors=[abs(pred(p,v)-p[6])for p in pts];print(theme,size,sum(errors)/len(errors),max(errors))
json.dump({'parameters':v,'trainScope':'dark44pt, threebackdrops','trainRMSE':best**.5,'samples':len(points)},open(root+'sdf-fit.json','w'),indent=2)
