"""Decode plist object references as data; never unarchive/instantiate classes."""
from pathlib import Path
import json,plistlib,base64,hashlib,argparse
def unpack(raw):
 p=plistlib.loads(raw);objects=p['$objects']
 def dec(v):
  if isinstance(v,plistlib.UID):return dec(objects[v.data])if v.data else None
  if isinstance(v,list):return [dec(x)for x in v]
  if not isinstance(v,dict):return v
  if 'NS.keys'in v:return dict(zip(dec(v['NS.keys']),dec(v['NS.objects'])))
  cls=objects[v['$class'].data]['$classname']if isinstance(v.get('$class'),plistlib.UID)else None
  if cls=='LKNSDictionaryCodingProxy':return dec(v['dict'])
  return ({'class':cls}if cls else{})|{k:dec(x)for k,x in v.items()if k!='$class'}
 return dec(p['$top']['root'])

def extract(source):
 d=json.loads(source.read_text());found=[]
 def walk(o,path=()):
  if isinstance(o,dict):
   if o.get('type')=='glassBackground'and'archiveBase64'in o:
    raw=base64.b64decode(o['archiveBase64']);found.append({'path':list(path),'archiveSHA256':hashlib.sha256(raw).hexdigest(),'filter':unpack(raw)})
   for k,v in o.items():walk(v,path+(k,))
  elif isinstance(o,list):
   for i,v in enumerate(o):walk(v,path+(i,))
 walk(d['layerTree'])
 if len(found)!=3:raise ValueError(f'Expected three glass backgrounds, got {len(found)}')
 for item,name in zip(found,['regular','clear','tinted']):item['style']=name
 return {'os':d['os'],'environment':d['environment'],'profiles':found}

if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('source',type=Path);a=p.parse_args();a.source.with_suffix('.profiles.json').write_text(json.dumps(extract(a.source),indent=2)+'\n')
