"""Exercise explicit zero-frost locality through the production retained renderer.
Usage: check-optical-locality.py OPTICAL_RENDERER OUTPUT
Each input has the same center pixels and different surrounding colors. Zero
frost must preserve equal center output in both themes and all tint modes;
positive frost is the control proving those neighbors enter the actual filter.
"""
from pathlib import Path
import json,subprocess,sys
from PIL import Image,ImageCms

renderer=Path(sys.argv[1]).resolve();out=Path(sys.argv[2]).resolve();out.mkdir(parents=True,exist_ok=True)
frames=[[40,64,52,52,26],[150,50,80,80,40],[280,30,120,120,60],[480,62,132,56,28],[240,230,240,160,36]]
profile=ImageCms.ImageCmsProfile(ImageCms.createProfile('sRGB')).tobytes()
for color,rgb in [('red',(255,0,0)),('blue',(0,0,255))]:
 image=Image.new('RGB',(720,440),rgb)
 for x,y,w,h,_ in frames:
  cx,cy=int(x+w/2),int(y+h/2)
  for yy in range(cy-2,cy+3):
   for xx in range(cx-2,cx+3):image.putpixel((xx,yy),(128,128,128))
 image.save(out/(color+'.png'),icc_profile=profile)

for frost in [0,6]:
 lines=[]
 for color in ['red','blue']:
  for dark in [0,1]:
   for tint in ['none','blue','coral']:
    name=f'{color}-dark{dark}-{tint}-frost{frost}'
    lines.append(' '.join([json.dumps(str(out/(name+'.ppm'))),json.dumps(str(out/(color+'.png'))),str(dark),tint]+[str(v) for f in frames for v in f]))
 manifest=out/f'manifest{frost}.txt';manifest.write_text('\n'.join(lines)+'\n')
 subprocess.run([str(renderer),str(manifest),f'--frost={frost},{frost}'],check=True,stdout=subprocess.DEVNULL)

checks=0;positive=0
for dark in [0,1]:
 for tint in ['none','blue','coral']:
  images={}
  for frost in [0,6]:
   images[frost]=[Image.open(out/f'{color}-dark{dark}-{tint}-frost{frost}.ppm').convert('RGB') for color in ['red','blue']]
  for x,y,w,h,_ in frames:
   cx,cy=int((x+w/2)*2),int((y+h/2)*2)
   samples=lambda frost:[[im.getpixel((xx,yy)) for yy in range(cy-2,cy+2) for xx in range(cx-2,cx+2)] for im in images[frost]]
   zero=samples(0)
   if zero[0]!=zero[1]:raise AssertionError(f'Zero frost sampled neighbors: dark={dark},tint={tint},size={w}x{h}')
   checks+=1
   control=samples(6)
   positive+=control[0]!=control[1]
if positive!=checks:raise AssertionError('Positive-frost control did not distinguish every tested context')
print(f'Passed {checks} zero-frost locality checks and {positive} positive-frost controls')
