"""Copy a completed iOS26 oracle run and create compact native contact sheets.
Usage: python3 collect-native.py DEVICE_UUID OUTPUT_DIRECTORY
Requires Pillow only for the optional contact sheets.
"""
import gzip,hashlib,json,shutil,sys
from pathlib import Path
udid=sys.argv[1];out=Path(sys.argv[2]).resolve();out.mkdir(parents=True,exist_ok=True)
base=Path.home()/'Library/Developer/CoreSimulator/Devices'/udid/'data/Containers/Data/Application'
candidates=list(base.glob('*/Documents/scroll-edge-reference/done.json'))
if len(candidates)!=1:raise RuntimeError(f'Expected one completed isolated oracle run; found {candidates}')
src=candidates[0].parent
meta=json.loads(candidates[0].read_text())
if not str(meta['version']).startswith('26.'):raise RuntimeError('Only iOS26 captures are accepted')
if len(json.loads((src/'states.json').read_text()))!=meta['cases']:raise RuntimeError('Oracle run is incomplete')
for p in src.glob('*.png'):shutil.copyfile(p,out/p.name)
for p in src.glob('*exception.json'):shutil.copyfile(p,out/p.name)
for name in ['runtime-api.json','done.json']:shutil.copyfile(src/name,out/name)
with gzip.open(out/'states.json.gz','wb') as f:f.write((src/'states.json').read_bytes())
try:
 from PIL import Image,ImageDraw
 for theme in ['light','dark']:
  names=[f'{theme}-{style}-{offset}' for offset in ['start','middle','end'] for style in ['automatic','soft','hard']]
  thumbs=[]
  for name in names:
   im=Image.open(out/f'{name}.png').convert('RGB');im.thumbnail((268,582));thumbs.append((name,im))
  w,h=268,607;sheet=Image.new('RGB',(w*3,h*3),'#dddddd');draw=ImageDraw.Draw(sheet)
  for i,(name,im) in enumerate(thumbs):x=(i%3)*w;y=(i//3)*h;sheet.paste(im,(x,y+25));draw.text((x+4,y+5),name,fill='black')
  sheet.save(out/f'{theme}-contact.png')
 if (out/'mask-ramp.png').exists():
  sheet=Image.new('RGB',(354*3,300),'#dddddd');draw=ImageDraw.Draw(sheet)
  for i,mode in enumerate(['zero','one','ramp']):
   im=Image.open(out/f'mask-{mode}.png').convert('RGB').crop((48,440,756,952)).resize((354,256));sheet.paste(im,(i*354,28));draw.text((i*354+4,6),mode,fill='black')
  sheet.save(out/'variable-blur-mask-contact.png')
except ImportError:pass
manifest={'device':udid,'runtime':meta,'files':{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(out.iterdir()) if p.is_file() and p.name!='manifest.json'}}
(out/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
print(json.dumps({'output':str(out),'runtime':meta,'pictures':len(list(out.glob('*.png')))},indent=2))
