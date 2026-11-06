"""Retain fixed ScrollPocket before/after images and matching geometry evidence.
Usage: python3 compare-pockets.py BEFORE_FIXTURE AFTER_FIXTURE OUTPUT
Optional trailing labels override the default adaptive/fixed captions.
Both fixtures are portable implementations, not native UIKit.
"""
from pathlib import Path
import gzip,hashlib,json,shutil,sys
from PIL import Image,ImageDraw
before,after,out=map(lambda v:Path(v).resolve(),sys.argv[1:4]);out.mkdir(parents=True,exist_ok=True)
labels=sys.argv[4:6] if len(sys.argv)==6 else ['Before: adaptive','After: fixed ScrollPocket']
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def states(directory):
    for name in ('states.json','states.json.gz'):
        p=directory/name
        if p.exists():return json.loads(gzip.decompress(p.read_bytes()) if p.suffix=='.gz' else p.read_bytes())
    raise FileNotFoundError(directory)
old={s['name']:s for s in states(before)};new=states(after);geometry=[]
for row in new:
    baseline=old[row['name']]
    for index,(a,b) in enumerate(zip(baseline['edges'],row['edges'])):
        if a['visible'] and b['visible']:
            delta=[y-x for x,y in zip(a['bandRect'],b['bandRect'])]
            geometry.append({'case':row['name'],'edge':index,'deltaPoints':delta})
assert all(all(abs(v)<.001 for v in r['deltaPoints']) for r in geometry)
images=[];row_height=370;sheet=Image.new('RGB',(804,row_height*4),'#ddd');draw=ImageDraw.Draw(sheet)
for index,(theme,kind) in enumerate((('light','dense'),('dark','dense'),('light','sparse'),('dark','sparse'))):
    relative=Path('profile' if kind=='dense' else 'sparse-profile')/(theme+'-soft.png')
    for column,(label,directory) in enumerate(zip(labels,(before,after))):
        source=directory/relative;name=('before' if column==0 else 'after')+'-'+theme+'-'+kind+'.png';shutil.copyfile(source,out/name)
        image=Image.open(source).convert('RGB');assert image.size==(804,1748)
        top=image.crop((0,0,804,380)).resize((402,190),Image.Resampling.LANCZOS)
        bottom=image.crop((0,1440,804,1748)).resize((402,154),Image.Resampling.LANCZOS)
        y=index*row_height;draw.text((column*402+6,y+5),theme+' '+kind+' - '+label,fill='black');sheet.paste(top,(column*402,y+23));sheet.paste(bottom,(column*402,y+213))
        images.append({'file':name,'source':str(source),'sha256':sha(source)})
sheet.save(out/'before-after.png')
(out/'portable-states.json.gz').write_bytes(gzip.compress(json.dumps(new,indent=2).encode(),mtime=0))
result={'scope':'Portable appearance comparison. Not native optical parity.','labels':labels,'images':images,'matchingActiveBands':len(geometry),'maximumBandCoordinateChangePoints':max((abs(v) for r in geometry for v in r['deltaPoints']),default=0),'geometry':geometry}
(out/'visuals.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({k:result[k] for k in ('matchingActiveBands','maximumBandCoordinateChangePoints')}))
