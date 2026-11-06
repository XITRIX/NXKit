"""Measure transparent-gap preservation in native/portable sparse edge profiles.
Usage: python3 analyze-sparse.py NATIVE_SPARSE_DIR OUTPUT_DIR [PORTABLE_SPARSE_DIR]
Needs Pillow. Fixture:48pt grid, skip fill/text when(x+y)%2==0, offset120,180.
"""
import json,sys
from pathlib import Path
from PIL import Image,ImageDraw
native=Path(sys.argv[1]).resolve();out=Path(sys.argv[2]).resolve();portable=Path(sys.argv[3]).resolve() if len(sys.argv)>3 else None
out.mkdir(parents=True,exist_ok=True)
rows=[20,40,100,116,126,136,146,156,740,760,780,791,810,840,860]
result={'scope':'Transparent cell-center comparison of this fixture. This is not a general screenshot parity score.','grid':{'tile':48,'offset':[120,180],'transparentRule':'(column+row)%2==0','gridOpaque':False,'gridBackground':'clear','screenBackground':'systemBackground'},'native':{},'portable':{}}

def excluded(x,y):return (62<=y<=116 and (145<=x<=255 or 325<=x<=395)) or (770<=y<=860 and 95<=x<=307)
def inspect(directory,theme):
 a=Image.open(directory/f'{theme}-soft.png').convert('RGB');b=Image.open(directory/f'{theme}-hidden.png').convert('RGB');values=[]
 for y in rows:
  # Avoid the horizontal tile boundary and isolate clear interiors from blur spill.
  if not 8<=(y+180)%48<=40:continue
  samples=[]
  for x in range(48,390,48):
   if excluded(x,y):continue
   column=int((x+120)//48);row=int((y+180)//48)
   if (column+row)%2:continue
   soft=a.getpixel((x*2,y*2));hidden=b.getpixel((x*2,y*2));samples.append({'x':x,'softRGB':list(soft),'hiddenRGB':list(hidden),'absoluteRGBDifference':sum(abs(p-q) for p,q in zip(soft,hidden))/3})
  if samples:values.append({'y':y,'samples':samples,'meanAbsoluteRGBDifference':sum(d['absoluteRGBDifference'] for d in samples)/len(samples)})
 return values

def colored_profile(directory,theme):
 a=Image.open(directory/f'{theme}-soft.png').convert('RGB');b=Image.open(directory/f'{theme}-hidden.png').convert('RGB');values=[]
 for y in rows:
  if not 8<=(y+180)%48<=40:continue
  xs=[x for x in range(48,390,48) if not excluded(x,y) and (int((x+120)//48)+int((y+180)//48))%2]
  if not xs:continue
  av=[channel for x in xs for channel in a.getpixel((x*2,y*2))];bv=[channel for x in xs for channel in b.getpixel((x*2,y*2))];am=sum(av)/len(av);bm=sum(bv)/len(bv);den=sum((x-bm)**2 for x in bv);slope=sum((x-bm)*(z-am) for x,z in zip(bv,av))/den if den else 0
  values.append({'y':y,'sampleX':xs,'allChannelSlope':slope,'allChannelIntercept':am-slope*bm})
 return values

for theme in ['light','dark']:
 result['native'][theme]={'transparentGaps':inspect(native,theme),'coloredCells':colored_profile(native,theme)}
 if portable:result['portable'][theme]={'transparentGaps':inspect(portable,theme),'coloredCells':colored_profile(portable,theme)}
columns=4 if portable else 2;width,height=201,459;sheet=Image.new('RGB',(width*columns,height*2),'#ddd');draw=ImageDraw.Draw(sheet)
for row,theme in enumerate(['light','dark']):
 sources=[(native,theme+'-soft','Native soft'),(native,theme+'-hidden','Native hidden')]
 if portable:sources.extend([(portable,theme+'-soft','Portable soft'),(portable,theme+'-hidden','Portable hidden')])
 for col,(directory,name,label) in enumerate(sources):
  im=Image.open(directory/(name+'.png')).convert('RGB').resize((201,437),Image.Resampling.LANCZOS);sheet.paste(im,(col*width,row*height+22));draw.text((col*width+4,row*height+4),theme+' '+label,fill='black')
sheet.save(out/'sparse-comparison.png');(out/'sparse-measurements.json').write_text(json.dumps(result,indent=2)+'\n')
for label in ['native','portable']:
 for theme,values in result[label].items():
  print(label,theme,[(d['y'],round(d['meanAbsoluteRGBDifference'],3)) for d in values['transparentGaps']])
