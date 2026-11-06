"""Independent AIR-derived SDF equations; native pixels only, no fitting."""
import json,math,re,argparse
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument("report",type=Path);p.add_argument("profiles",type=Path);a=p.parse_args()
root=str(a.report)+'/'
data=json.load(open(root+'samples.json'));profiles=json.loads(a.profiles.read_text())['profiles']
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


# independent straight mathematical translation of inspected AIR; no fitting.
# regenerate points to retain normals/individual spreadcones and AA derivatives.
errors=[];records=[]
for theme in ['dark','light']:
 profile=next(x for x in profiles if x['appearance']==theme and x['requestedTint']=='0.5'and x['style']=='regular'and x['geometry']['width']==44 and x['geometry']['height']==44)
 h=find(profile['layerTree'],'CASDFKeyFillHighlightEffect');m=matrix(profile['layerTree'])
 for lit,unlit in zip(data[theme+'-baseline'],data[theme+'-no-highlight']):
  x,y,w,hh=map(float,re.findall(r'[0-9.]+',lit['frame']));cx=x+w/2;cy=y+hh/2
  for a,b in zip(lit['samples'],unlit['samples']):
   px,py=a['pixel'];dx=(px+.5)/3-cx;dy=(py+.5)/3-cy;r=math.hypot(dx,dy);d=w/2-r
   if not .25<d<12:continue
   nx=dx/r;ny=dy/r;aa=max((abs(nx)+abs(ny))/3,.0001)
   def sat(v):return max(0,min(1,v))
   def lobe(height,spread,angle,curv):
    t=sat(d/height);shape=(1-curv*t)if t<1 else 0
    gates=sat(d/aa+.5)*sat((height-d)/aa+.5)
    direction=nx*math.cos(angle-math.pi/2)+ny*math.sin(angle-math.pi/2)
    return shape*gates*sat((direction-math.cos(spread))/(1-math.cos(spread)))
   def bias(v,amount):return v/max(1+(1/max(amount,.000001)-2)*(1-v),.000001)
   R=0
   for prefix in ['key','fill']:
    H=h[prefix+'Height'];S=h[prefix+'Spread'];A=h[prefix+'Angle'];M=h[prefix+'Amount']
    R+=bias(lobe(H,S,A,h['curvature']),M)
    R+=bias(lobe(H*h['diffuseHeightScale'],S*h['diffuseSpreadScale'],A,1),M*h['diffuseAmountScale'])
   B=b['rgb'][0];C=a['rgb'][0];mapped=max(0,min(255,(m['m11']+m['m12']+m['m13'])*B+m['m14']*255+m['m15']*255));prediction=max(0,min(255,B+R*(mapped-B)));error=abs(prediction-C)
   records.append(dict(theme=theme,size=w,error=error,prediction=prediction,native=C,depth=d))
for theme in ['dark','light']:
 for size in [44,80,120]:
  rows=[x for x in records if x['theme']==theme and x['size']==size];print(theme,size,sum(x['error']for x in rows)/len(rows),max(x['error']for x in rows))
print('ALL',sum(x['error']for x in records)/len(records),max(x['error']for x in records))
json.dump({'method':'AIR-derived Schlick-bias kernel; no fitted constants','samples':len(records),'meanAbsoluteError':sum(x['error']for x in records)/len(records),'maximumChannelError':max(x['error']for x in records),'records':records},open(root+'air-sdf-verification.json','w'),indent=2)
