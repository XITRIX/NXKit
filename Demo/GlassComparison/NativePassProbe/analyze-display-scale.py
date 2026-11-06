"""Validate archived UIKit inputs and public trait controls without fitting."""
import argparse,base64,gzip,json,plistlib
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('report',type=Path);a=p.parse_args()
def archive(text):
 d=plistlib.loads(base64.b64decode(text));objects=d['$objects']
 def resolve(o):
  if isinstance(o,plistlib.UID):return resolve(objects[o.data])
  if isinstance(o,list):return [resolve(v)for v in o]
  if isinstance(o,dict):
   if 'NS.keys'in o:return dict(zip(resolve(o['NS.keys']),resolve(o['NS.objects'])))
   return {k:resolve(v)for k,v in o.items()if k!='$class'}
  return o
 return resolve(d['$top']['root'])
def filters(o):
 result=[]
 if isinstance(o,dict):
  if o.get('name')=='glassBackground'and'archiveBase64'in o:
   result.append(archive(o['archiveBase64'])['CAFilterInputs']['dict'])
  for v in o.values():result+=filters(v)
 elif isinstance(o,list):
  for v in o:result+=filters(v)
 return result
profiles=0
for path in (a.report/'all2x').glob('*.gz'):
 data=json.loads(gzip.decompress(path.read_bytes()));values=filters(data['layerTree'])
 assert len(values)==len(data['surfaces'])==56
 profiles+=len(values)
print('Validated native 2x material profiles:',profiles)
assert profiles==336
for path in sorted((a.report/'trait-cases').glob('*.gz')):
 data=json.loads(gzip.decompress(path.read_bytes()));values=filters(data['layerTree']);assert len(values)==4
 keys=['inputKeyFillHighlightHeight','inputKeyFillHighlightEffectOffset','inputKeyFillHighlightColorBias']
 settings=[tuple(v[k]for k in keys)for v in values];assert len(set(settings))==1
 print(path.name,'screen',data['scale'],'trait',data['traitScale'],'idiom',data['traitIdiom'],'settings',settings[0])
context=a.report.parent/'lighting-spatial/interactive-context'
if context.exists():
 normal=filters(json.loads((context/'normal.json').read_text())['layerTree'])
 interactive=filters(json.loads((context/'interactive.json').read_text())['layerTree'])
 assert len(normal)==len(interactive)==42
 assert normal==interactive
 print('Interactive controls: 42 valid native filters per process; inputs identical')
