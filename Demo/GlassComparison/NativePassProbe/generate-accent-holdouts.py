"""Export independent accent holdouts without adding them to the fitted palette."""
from pathlib import Path
import argparse,base64,gzip,hashlib,json,math,sys
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'SettingsProbe'))
from decode import unpack
root=Path(__file__).resolve().parents[3]
source=root/'docs/liquid-glass-native-settings/color-transfer'
target=root/'UIKit/tests/fixtures/NativeGlassAccentHoldouts.inc'
p=argparse.ArgumentParser();p.add_argument('--check',action='store_true');a=p.parse_args()
def walk(x):
 if isinstance(x,dict):
  yield x
  for v in x.values():yield from walk(v)
 elif isinstance(x,list):
  for v in x:yield from walk(v)
def number(x):
 assert math.isfinite(x)
 s=format(x,'.17g');return s+('.0'if'.'not in s and'e'not in s else'')+'f'
lines=['// Independent iOS27.0/24A434 accent matrices. Generated from native archives.',
       '// These colors are excluded from NativeGlassAccentData.hpp.']
records=[]
for theme in ['dark','light']:
 path=source/('holdout-'+theme+'.json.gz');raw=path.read_bytes();d=json.loads(gzip.decompress(raw))
 assert d['os']=='Version 27.0 (Build 24A434)' and d['effectiveTint']==.5
 matrices=[]
 for x in walk(d['layerTree']):
  if x.get('name')=='vibrantColorMatrix'and'archiveBase64'in x:
   v=unpack(base64.b64decode(x['archiveBase64']))['CAFilterInputs']
   if v.get('inputBackdropAware'):matrices.append(v)
 assert len(matrices)==len(d['surfaces'])==16
 sha=hashlib.sha256(raw).hexdigest();lines.append('// '+path.name+' SHA256 '+sha)
 for s,v in zip(d['surfaces'],matrices):
  assert s['width']==280 and s['height']==80
  rgb=s['resolvedTint']['rgba'][:3];m=v['inputColorMatrix'];values=[m[f'm{i}{j}']for i in range(1,5)for j in range(1,6)]
  lines.append('{'+str(theme=='dark').lower()+','+str(s['style']=='clearTinted').lower()+', {'+','.join(number(x)for x in rgb)+',1.0f}, {{'+','.join(number(x)for x in values)+'}}},')
  records.append({'appearance':theme,'style':s['style'],'name':s['name'],'color':rgb,'matrix':values,'sourceSHA256':sha})
result='\n'.join(lines)+'\n';normalized=json.dumps(records,indent=2)+'\n';meta=source/'accent-holdouts.json'
if a.check:
 assert target.read_text()==result,'Accent holdout fixture is stale'
 assert meta.read_text()==normalized,'Normalized holdouts are stale'
 print('32 independent native accent matrices and archive hashes verified')
else:
 target.write_text(result);meta.write_text(normalized)
 print('Exported 32 independent native accent matrices')
