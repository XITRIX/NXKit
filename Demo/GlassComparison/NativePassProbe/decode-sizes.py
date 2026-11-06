"""Decode captured archives as data, preserving per-size applied layer trees."""
from pathlib import Path
import sys,json,gzip,base64,hashlib
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'SettingsProbe'))
from decode import unpack
root=Path(sys.argv[1]);expected=int(sys.argv[2])if len(sys.argv)>2 else 42;profiles=[]
def norm(x):
 if isinstance(x,dict):
  if 'archiveBase64'in x:return unpack(base64.b64decode(x['archiveBase64']))
  return {k:norm(v)for k,v in x.items()}
 if isinstance(x,list):return [norm(v)for v in x]
 return x
def find(x,out):
 if isinstance(x,dict):
  if x.get('class')=='CAFilter'and'archiveBase64'in x:
   raw=base64.b64decode(x['archiveBase64']);f=unpack(raw)
   if f.get('CAFilterType')=='glassBackground':out.append((f,hashlib.sha256(raw).hexdigest()))
  for v in x.values():find(v,out)
 elif isinstance(x,list):
  for v in x:find(v,out)
for p in sorted(root.glob('*.json.gz')):
 raw=p.read_bytes();d=json.loads(gzip.decompress(raw));assert len(d['surfaces'])==expected
 materials=[]
 def materials_walk(n):
  if n.get('value',{}).get('class')=='_UIMultiLayer':
   found=[];find(n,found)
   if len(found)==1:materials.append(n);return
  for c in n.get('sublayers',[]):materials_walk(c)
 materials_walk(d['layerTree']);assert len(materials)==len(d['surfaces']),(len(materials),len(d['surfaces']))
 for s,tree in zip(d['surfaces'],materials):
  assert tree['bounds']=="{{0, 0}, {%s, %s}}"%(s['width'],s['height']),(tree['bounds'],s)
  found=[];find(tree,found);assert len(found)==1,(p.name,s['width'],s['height'],s['style'],len(found))
  f,sha=found[0];assert len(f['CAFilterInputs'])==70
  profiles.append({'capture':p.name.removesuffix('.json.gz'),'appearance':'light'if p.name.startswith('light')else'dark','requestedTint':str(d['effectiveTint']),'effectiveTint':d['effectiveTint'],'style':s['style'],'resolvedSystemBlue':s.get('resolvedSystemBlue'),'geometry':{'width':s['width'],'height':s['height'],'cornerRadius':min(s['width'],s['height'])/2,'displayScale':d['scale']},'inputs':f['CAFilterInputs'],'sourceArchiveSHA256':sha,'sourceCaptureSHA256':hashlib.sha256(raw).hexdigest(),'layerTree':norm(tree)})
out={'runtime':d['os'],'context':'Overlapping attached public UIVisualEffectViews. This capture establishes applied configuration values, not isolated final pixel colors. Geometry uses view bounds; no transforms. Radius shortest side/2.','profiles':profiles}
(root.parent/(('size-profiles'if root.name=='sizes'else root.name+'-profiles')+'.json')).write_text(json.dumps(out,indent=2)+'\n');print(len(profiles),'profiles, all70 inputs and normalized applied layer trees')
