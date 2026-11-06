"""Compare native UITabBar source zoom with before/after Demo holds.
Usage: compare-lens-magnification.py RENDERER BEFORE_RENDERER OUTPUT
Retained native inputs: docs/liquid-glass-lens/tab-press (independent UIKit).
"""
from pathlib import Path
import gzip,hashlib,json,subprocess,sys,io,shutil
import numpy as np
from PIL import Image,ImageCms,ImageDraw
repo=Path(__file__).resolve().parents[2]
renderer,before,out=Path(sys.argv[1]).resolve(),Path(sys.argv[2]).resolve(),Path(sys.argv[3])
images=out/'images';images.mkdir(parents=True,exist_ok=True)
retained=repo/'docs/liquid-glass-lens/tab-press';records=[]
srgb=ImageCms.createProfile('sRGB');profile=ImageCms.ImageCmsProfile(srgb).tobytes()
def rgb(path):
 im=Image.open(path);icc=im.info.get('icc_profile');im=im.convert('RGB')
 return ImageCms.profileToProfile(im,ImageCms.ImageCmsProfile(io.BytesIO(icc)),srgb,outputMode='RGB') if icc else im
for case in json.loads((retained/'metrics.json').read_text())['records']:
 name=case['id'];trace=json.load(gzip.open(retained/'native'/(name+'-trace.json.gz')));cols={k:i for i,k in enumerate(trace['columns'])}
 buttons=[e['id'] for e in trace['schema'] if e['viewClass']=='_UITabButton' and e['path'].startswith('stage.1.0.0.0.') and e['path'].count('.')==5]
 assert len(buttons)==4
 snapshot=next(s for s in trace['snapshots'] if s['name']=='contact-1')
 held=min(trace['frames'],key=lambda f:abs(f['t']-snapshot['startedT']))
 def row(frame,identifier):return next(r for r in frame['layers'] if r[0]==identifier)
 scales=[[row(frame,b)[cols['presentationM11']] for b in buttons] for frame in [trace['frames'][0],held,trace['frames'][-1]]]
 assert max(abs(s-1.16) for s in scales[1])<1e-5
 request=case['request'];results={}
 for label,exe in [('before',before),('nx',renderer)]:
  prefix=out/(name+'-'+label)
  subprocess.run([str(exe),str(prefix),str(int(request['dark'])),*map(str,request['backgroundRGB']),'2','2','--fixed-optics'],check=True,stdout=subprocess.DEVNULL)
  results[label]={}
  for phase in ['rest','held']:
   raw=Path(str(prefix)+'-'+phase+'.ppm');im=rgb(raw).crop((0,200,804,400));raw.unlink();im.save(images/(name+'-'+phase+'-'+label+'.png'),icc_profile=profile)
   meta=Path(str(prefix)+'-'+phase+'.json');results[label][phase]=json.loads(meta.read_text());meta.unlink()
 phases={}
 for phase in ['rest','held']:
  native=rgb(retained/'images'/(name+'-'+phase+'-native.png'));native.save(images/(name+'-'+phase+'-native.png'),icc_profile=profile)
  nx=rgb(images/(name+'-'+phase+'-nx.png'));old=rgb(images/(name+'-'+phase+'-before.png'));a,b,c=[np.array(im,dtype=float) for im in [native,nx,old]]
  diff=np.abs(a-b);Image.blend(native,nx,.5).save(images/(name+'-'+phase+'-overlay.png'))
  Image.fromarray(np.clip(diff*4,0,255).astype('uint8')).save(images/(name+'-'+phase+'-difference.png'))
  residual=b.astype('uint8');residual[np.max(diff,axis=2)<=2]=[64,64,64];Image.fromarray(residual).save(images/(name+'-'+phase+'-residual.png'))
  # Fixed coordinates around the selected glyph/caption, no fitted alignment.
  roi=(slice(54,154),slice(444,533))
  phases[phase]=dict(fullBefore=float(abs(a-c).mean()),fullNow=float(diff.mean()),selectedBefore=float(abs(a-c)[roi].mean()),selectedNow=float(diff[roi].mean()),beforeNXDifference=float(abs(b-c).mean()),geometry=results['nx'][phase])
 symbolIDs=[next(e['id'] for e in trace['schema'] if e['path']==next(b['path'] for b in trace['schema'] if b['id']==button)+'.0' and e['viewClass']=='UIImageView') for button in buttons]
 nativeSymbolFrames=[[row(held,identifier)[cols[k]] for k in ['presentationStageX','presentationStageY','presentationStageWidth','presentationStageHeight']] for identifier in symbolIDs]
 sizeError=float(np.max(np.abs(np.array(nativeSymbolFrames)[:,2:]-np.array(results['nx']['held']['selectedSymbols'])[:,2:])))
 records.append(dict(id=name,nativeItemScales=dict(rest=scales[0],held=scales[1],release=scales[2]),nativeSelectedSymbolFrames=nativeSymbolFrames,selectedSymbolSizeMaxError=sizeError,phases=phases))
 print(name,phases['held']['selectedBefore'],phases['held']['selectedNow'],flush=True)
(out/'metrics.json').write_text(json.dumps(records,indent=2)+'\n')
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
files=['UIKit/include/LensView.h','UIKit/lib/LensView.cpp','UIKit/lib/LensMaterial.cpp','Demo/app/Screens/GlassDemoViewController/LensTabBarView.cpp','Demo/GlassComparison/TabPressRenderer.cpp','Demo/GlassComparison/compare-lens-magnification.py']
(out/'provenance.json').write_text(json.dumps(dict(renderer=sha(renderer),beforeRenderer=sha(before),sourceSHA256={p:sha(repo/p) for p in files},nativeTraceSHA256={r['id']:sha(retained/'native'/(r['id']+'-trace.json.gz')) for r in records}),indent=2)+'\n')
page='''<!doctype html><meta charset="utf-8"><title>Lifted lens magnification</title><style>body{font:16px/1.5 system-ui;background:#191a1e;color:#eee;margin:24px;max-width:1100px}select{font:inherit;margin:8px}img{display:block;width:804px;max-width:100%}pre{white-space:pre-wrap}a{color:#9cf}</style><h1>Lifted lens magnification</h1><p>Native selected tab items scale 1.16 around their own centers, independently of platter inflation and lens growth. Fixed-pose Demo holds isolate source zoom from motion. The optical field, selected vibrancy and perimeter remain approximate. <a href="README.md">Evidence and limits</a></p><select id="scene"></select><select id="phase"><option>held</option><option>rest</option></select><select id="mode"><option>overlay</option><option>native</option><option>nx</option><option>before</option><option>difference</option><option>residual</option></select><img id="image"><pre id="score"></pre><script>const data=__DATA__;const scene=document.getElementById('scene'),phase=document.getElementById('phase'),mode=document.getElementById('mode');data.forEach(r=>scene.add(new Option(r.id,r.id)));function update(){const r=data.find(r=>r.id===scene.value);document.getElementById('image').src='images/'+r.id+'-'+phase.value+'-'+mode.value+'.png';document.getElementById('score').textContent=JSON.stringify({nativeItemScales:r.nativeItemScales,measurements:r.phases[phase.value]},null,2)}scene.onchange=phase.onchange=mode.onchange=update;update()</script>'''
(out/'comparison.html').write_text(page.replace('__DATA__',json.dumps(records).replace('<','\\u003c')))
preview=Image.new('RGB',(1206,252),(25,25,28));draw=ImageDraw.Draw(preview)
for ri,name in enumerate(['phone-light-gray','phone-dark-black']):
 for ci,(label,title) in enumerate([('native','UIKit'),('before','Before'),('nx','Now')]):
  draw.text((ci*402+8,ri*126+2),name+' / '+title,fill='white');preview.paste(rgb(images/(name+'-held-'+label+'.png')).resize((402,100)),(ci*402,ri*126+23))
preview.save(out/'overview.png')
assert all(max(abs(s-1.16) for s in r['phases']['held']['geometry']['sourceScales'])<.0001 for r in records)
assert all(r['phases']['rest']['beforeNXDifference']<.001 for r in records)
