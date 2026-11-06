from pathlib import Path
import json,plistlib,base64
p=Path('/private/tmp/nxkit-rim-context')
def decode(text):
 d=plistlib.loads(base64.b64decode(text));objects=d['$objects']
 def get(o):
  if isinstance(o,plistlib.UID):return get(objects[o.data])
  if isinstance(o,list):return [get(v)for v in o]
  if isinstance(o,dict):
   if 'NS.keys'in o:return dict(zip(get(o['NS.keys']),get(o['NS.objects'])))
   return {k:get(v)for k,v in o.items()if k!='$class'}
  return o
 return get(d['$top']['root'])
def filters(o):
 out=[]
 if isinstance(o,dict):
  if o.get('name')=='glassBackground'and'archiveBase64'in o:out.append(decode(o['archiveBase64'])['CAFilterInputs']['dict'])
  for v in o.values():out+=filters(v)
 elif isinstance(o,list):
  for v in o:out+=filters(v)
 return out
A=filters(json.load(open(p/'normal.json'))['layerTree']);B=filters(json.load(open(p/'interactive.json'))['layerTree']);assert len(A)==len(B)==42,(len(A),len(B))
changes=[{k:[a.get(k),b.get(k)]for k in a.keys()|b.keys()if a.get(k)!=b.get(k)}for a,b in zip(A,B)];print('3x',len(A),'changed',sum(bool(c)for c in changes));(p/'differences.json').write_text(json.dumps({'filterCountPerProcess':42,'changedProfiles':sum(bool(c)for c in changes),'differences':changes},indent=2))
D=filters(json.load(open(p/'2x-interactive.json'))['layerTree']);assert len(D)==3,len(D)
(p/'2x-filter-inputs.json').write_text(json.dumps(D,indent=2))
for d in D:print({k:v for k,v in d.items()if'KeyFillHighlight'in k})
