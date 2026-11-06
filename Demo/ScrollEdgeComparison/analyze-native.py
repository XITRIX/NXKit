"""Measure native carriers and optical profiles. Needs Pillow; no native execution."""
import gzip,json,sys
from pathlib import Path
from PIL import Image,ImageChops,ImageStat
out=Path(sys.argv[1]).resolve()
states=json.loads(gzip.decompress((out/'states.json.gz').read_bytes()))
byname={d['name']:d for d in states}
ROWS=[0,20,40,60,80,100,106,116,126,136,146,156,166,700,720,740,760,780,791,810,840,860]

def exclude_chrome(x,y):
 return (62<=y<=116 and (145<=x<=255 or 325<=x<=395)) or (770<=y<=860 and 95<=x<=307)

def profile(a,b,centers):
 result=[]
 # Fixture tile centers are (24+48k)-contentOffset.x; offset.x=120.
 # 25+48k is instead about one point from a tile boundary. Keep both families
 # explicit so the edge's contrast mask is not confused with blur mixing.
 candidates=range(48,390,48) if centers else range(25,390,48)
 for y in ROWS:
  xs=[x for x in candidates if not exclude_chrome(x,y)]
  av=[a.getpixel((x*2,y*2)) for x in xs];bv=[b.getpixel((x*2,y*2)) for x in xs];rgb=[]
  for c in range(3):
   observed=[v[c] for v in av];reference=[v[c] for v in bv]
   am=sum(observed)/len(observed);bm=sum(reference)/len(reference)
   den=sum((x-bm)**2 for x in reference)
   slope=sum((x-bm)*(z-am) for x,z in zip(reference,observed))/den if den else 0
   rgb.append({'slope':slope,'intercept':am-slope*bm,'nativeMean':am,'hiddenMean':bm})
  difference=sum(abs(v-u) for aa,bb in zip(av,bv) for v,u in zip(aa,bb))/(len(xs)*3)
  result.append({'y':y,'sampleX':xs,'meanAbsoluteRGBDifference':difference,'rgb':rgb})
 return result

result={'provenance':'Derived from native iOS26 PNGs and read-only layer/view traces. Contrast fits are not recovered mask constants or Gaussian sigma.','cases':len(states),'geometry':{}}
for name in ['light-soft-start','light-soft-middle','light-hard-start','light-hard-middle','dark-soft-middle','light-soft-sides','light-hard-sides']:
 if name not in byname:continue
 d=byname[name];result['geometry'][name]={'safe':d['safe'],'offset':d['offset'],'carriers':[{'class':v['class'],'frame':v['windowFrame'],'alpha':v['alpha']} for v in d['views'] if 'BackdropView' in v['class']]}
a=Image.open(out/'light-soft-middle.png').convert('RGB');b=Image.open(out/'hidden.png').convert('RGB')
result['lightSoftVersusHidden']={'cellCenters':profile(a,b,True),'nearTileBoundaries':profile(a,b,False)}
near_rest={}
for theme in ['light','dark']:
 source=out/'profile'/f'{theme}-hidden.png'
 if not source.exists():continue
 hidden=Image.open(source).convert('RGB');palette={}
 for k in range(3,8):palette[(k+8)%5]=hidden.getpixel(((24+48*k-120)*2,(48*8+36-180)*2))
 records=[]
 for style in ['automatic','soft','hard']:
  for position in ['start','one','eight','overscroll']:
   name=f'{theme}-{style}-{position}'
   if name not in byname:continue
   state=byname[name];image=Image.open(out/(name+'.png')).convert('RGB');ox,oy=state['offset']
   for world_y in [4,12,24,36]:
    y=-oy+world_y;av=[];bv=[]
    for k in range(8):
     x=24+48*k-ox
     if x>=400:continue
     bv.extend(palette[(k+int(world_y//48))%5]);av.extend(image.getpixel((round(x*2),round(y*2))))
    am=sum(av)/len(av);bm=sum(bv)/len(bv);den=sum((x-bm)**2 for x in bv);slope=sum((x-bm)*(z-am) for x,z in zip(bv,av))/den
    records.append({'case':name,'worldY':world_y,'windowY':y,'slope':slope,'intercept':am-slope*bm})
 near_rest[theme]={'provenance':'Reference colors sampled at unclipped flat-cell centers from same-trait hidden fixture; text and tile boundaries avoided.','palette':palette,'rows':records}
result['nearRestContentContrast']=near_rest
(out/'measurements.json').write_text(json.dumps(result,indent=2)+'\n')
paired={}
for theme in ['light','dark']:
 p=out/'profile';soft=p/f'{theme}-soft.png';hidden=p/f'{theme}-hidden.png'
 if soft.exists() and hidden.exists():
  a=Image.open(soft).convert('RGB');b=Image.open(hidden).convert('RGB');paired[theme]={'cellCenters':profile(a,b,True),'nearTileBoundaries':profile(a,b,False)}
if paired:(out/'profile/measurements.json').write_text(json.dumps(paired,indent=2)+'\n')
print(json.dumps({'cases':len(states),'output':str(out/'measurements.json'),'pairedTraits':list(paired)},indent=2))
