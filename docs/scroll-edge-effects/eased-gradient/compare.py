"""Compare retained iOS26 captures to identical before/after portable fixtures."""
from pathlib import Path
import json,hashlib,sys
from PIL import Image,ImageDraw
native,before,after,out=map(Path,sys.argv[1:5]);out.mkdir(parents=True,exist_ok=True)
meta={'nativeRuntime':'iOS 26.0 (23A343), retained reference','limits':'Native optics adapt to content; portable optics intentionally remain constant. Carrier geometries differ; no extension was added. Images show composition, not recovered native alpha.','images':[]}
for theme in ('dark','light'):
 sheet=Image.new('RGB',(1206,748),'#ddd');draw=ImageDraw.Draw(sheet)
 for row,kind in enumerate(('profile','sparse-profile')):
  for col,(label,root) in enumerate((('iOS 26',native),('Before',before),('Eased',after))):
   p=root/kind/(theme+'-soft.png');im=Image.open(p).convert('RGB');assert im.size==(804,1748)
   x,y=col*402,row*374
   draw.text((x+5,y+5),label+' / '+('dense' if row==0 else 'sparse'),fill='black')
   sheet.paste(im.crop((0,0,804,380)).resize((402,190),Image.Resampling.LANCZOS),(x,y+24))
   sheet.paste(im.crop((0,1440,804,1748)).resize((402,154),Image.Resampling.LANCZOS),(x,y+216))
   name=f'{theme}-{row}-{col}.png';im.save(out/name)
   meta['images'].append({'file':name,'source':str(p),'sha256':hashlib.sha256(p.read_bytes()).hexdigest()})
 sheet.save(out/(theme+'-comparison.png'))
for kind in ('profile','sparse-profile'):
 old=json.load(open(before/kind/'states.json'));new=json.load(open(after/kind/'states.json'))
 assert old==new,'Appearance changes must leave all fixture state/geometry identical'
meta['identicalPortableStateCases']=8
(out/'comparison.json').write_text(json.dumps(meta,indent=2)+'\n')
