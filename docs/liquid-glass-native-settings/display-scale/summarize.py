from pathlib import Path
import json,gzip
exec(Path('/private/tmp/nxkit-rim-context/decode.py').read_text().split('A=filters(')[0])
profiles=json.load(open('docs/liquid-glass-native-settings/pass-semantics/all-styles-profiles.json'))['profiles'];summary=[]
for path in sorted((p/'all2x').glob('*.gz')):
 d=json.loads(gzip.decompress(path.read_bytes()));v=filters(d['layerTree']);assert len(v)==len(d['surfaces'])==56
 for src,settings in zip(d['surfaces'],v):
  theme=path.name.split('-')[0];tint=path.name.split('-')[1].split('.')[0];amount={'0':'0','half':'0.5','1':'1'}[tint]
  ref=next(x for x in profiles if x['appearance']==theme and x['requestedTint']==amount and x['style']==src['style'] and x['geometry']['width']==src['width'] and x['geometry']['height']==src['height'])
  diff={key:[ref['inputs'].get(key),value]for key,value in settings.items()if isinstance(value,(int,float))and ref['inputs'].get(key)!=value}
  summary.append(dict(appearance=theme,amount=amount,style=src['style'],width=src['width'],height=src['height'],differences=diff))
(p/'2x-scalar-differences.json').write_text(json.dumps(summary,indent=2));counts={}
for x in summary:
 for k,v in x['differences'].items():counts.setdefault(k,set()).add(str(v))
print(len(summary),'nativeprofiles',counts)
