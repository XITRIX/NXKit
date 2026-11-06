"""Export fixed-coordinate corner and glass comparisons.
Usage: compare-continuous.py RUN OUTPUT
RUN contains CornerTests after.ppm, native27/{ipad-framebuffer.png,ipad-corners.json},
and before/after OpticalRenderer outputs from the manifest documented in README.
The before solid mask and optical native images are retained repository evidence.
"""
from pathlib import Path
import hashlib,json,sys
import numpy as np
from PIL import Image,ImageCms,ImageDraw,ImageFont
run,out=(Path(s).resolve() for s in sys.argv[1:]);out.mkdir(parents=True,exist_ok=True)
repo=Path(__file__).resolve().parents[2]
profile=ImageCms.ImageCmsProfile(ImageCms.createProfile('sRGB')).tobytes()
def load(p):return np.asarray(Image.open(p).convert('RGB'))
def save(a,p):Image.fromarray(a).save(out/p,icc_profile=profile)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def products(key,n,b,a):
 for suffix,data in [('native',n),('before',b),('after',a)]:save(data,Path(key+'-'+suffix+'.png'))
 delta=np.abs(n.astype(float)-a.astype(float));save(np.clip(delta*4,0,255).astype('uint8'),Path(key+'-difference.png'))
 unmatched=np.dstack([a, np.where(np.max(delta,axis=2)>2,255,0).astype('uint8')]);save(unmatched,Path(key+'-unmatched.png'))
 save(np.stack([n[:,:,0],a[:,:,0],a[:,:,0]],axis=2),Path(key+'-registration.png'))
 return dict(key=key,label=key)
meta=json.loads((run/'native27/ipad-corners.json').read_text());scale=meta['scale'];assert scale==2
frame=load(run/'native27/ipad-framebuffer.png');x,y=round(meta['x']*scale),round(meta['y']*scale)
native=frame[y:y+1520,x:x+1440];before=load(repo/'docs/liquid-glass-corners/nxkit.png');after=load(run/'after.ppm')
assert native.shape==before.shape==after.shape
save(frame,Path('native-framebuffer.png'));save(native,Path('native-scene.png'));save(after,Path('after-scene.png'))
meta['retained26Identical']=bool(np.array_equal(native,load(repo/'docs/liquid-glass-corners/native.png')))
(out/'native-metadata.json').write_text(json.dumps(meta,indent=2)+'\n')
scenes=[];mask_metrics=[]
for row,(top,h,r) in enumerate(zip(meta['ys'],meta['heights'],meta['radii'])):
 for column,curve in enumerate(['circular','continuous']):
  left=20+360*column;sl=np.s_[top*2:(top+h)*2,left*2:(left+300)*2]
  n,b,a=(v[sl] for v in [native,before,after]);target=n[:,:,0]>127;actual=a[:,:,0]>127
  mask_metrics.append(dict(curve=curve,radius=r,height=h,beforeMAE=float(np.abs(n.astype(float)-b).mean()),afterMAE=float(np.abs(n.astype(float)-a).mean()),afterIoU=float(np.count_nonzero(target&actual)/np.count_nonzero(target|actual))))
  if column:
   sl=np.s_[(top-10)*2:(top+h+10)*2,(left-10)*2:(left+310)*2]
   scene=products(f'shape-{r}',*(v[sl] for v in [native,before,after]));scene['label']=f'Continuous 300 × {h}, radius {r}';scenes.append(scene)
frames=[(40,64,52,52),(150,50,80,80),(280,30,120,120),(480,62,132,56),(240,230,240,160)]
glass_metrics=[]
for bg in ['gray','white','blue','stripes']:
 for dark in [0,1]:
  name=f'{bg}-dark{dark}';n=load(repo/f'docs/liquid-glass-optics/images/{name}-none-native.png');b=load(run/f'before/{name}.ppm');a=load(run/f'after/{name}.ppm')
  assert n.shape==b.shape==a.shape
  for i,(x,y,w,h) in enumerate(frames):
   sl=np.s_[(y-8)*2:(y+h+8)*2,(x-8)*2:(x+w+8)*2]
   glass_metrics.append(dict(scene=name,width=w,height=h,beforeMAE=float(np.abs(n[sl].astype(float)-b[sl]).mean()),afterMAE=float(np.abs(n[sl].astype(float)-a[sl]).mean())))
  scene=products(name,n,b,a);scene['label']=f'Regular glass, {bg}, '+('dark' if dark else 'light');scenes.append(scene)
metrics=dict(solid=mask_metrics,glass=glass_metrics,units='Mean absolute sRGB channel error, 0–255. Glass ROIs: fixed frame plus 8pt exterior; equal weight per geometry.',glassBeforeMean=float(np.mean([m['beforeMAE'] for m in glass_metrics])),glassAfterMean=float(np.mean([m['afterMAE'] for m in glass_metrics])),glassImproved=sum(m['afterMAE']<m['beforeMAE'] for m in glass_metrics))
(out/'metrics.json').write_text(json.dumps(metrics,indent=2)+'\n')
source_paths=['UIKitCore/lib/LayerCornerGeometry.cpp','UIKitCore/lib/LayerCornerGeometry.hpp','UIKit/lib/GlassShader.cpp','UIKit/lib/CABackdropEffectLayer.cpp','UIKit/lib/CAGlassContainerLayer.cpp','UIKit/lib/LensMaterial.cpp','Demo/GlassComparison/OpticalRenderer.cpp','UIKit/tests/CornerRenderingTests.cpp']
provenance=dict(sources={p:sha(repo/p) for p in source_paths},nativeFramebufferSHA256=sha(run/'native27/ipad-framebuffer.png'),nativeExecutableSHA256=sha(run/'native27/NativeCornerReference.app/NativeCornerReference'),beforeCoreArchiveSHA256=sha(run/'before-core.a'),afterCoreArchiveSHA256=sha(run/'after/libOpticalCore.a'),beforeSolidSHA256=sha(repo/'docs/liquid-glass-corners/nxkit.png'),nativeOpticsMetadata='../liquid-glass-optics/native-metadata.json',alignment='Recorded stage origin only. No fitted translation, scaling, blur or color correction.',raster='Headless production CALayer/Skia raster with matching archive headers; iOS Demo built against current Graphite SDK. No GPU runtime parity claim.')
(out/'provenance.json').write_text(json.dumps(provenance,indent=2)+'\n')
# A compact preview; full original pixels remain available in the viewer.
key='shape-50';tiles=[Image.open(out/f'{key}-{v}.png').convert('RGB') for v in ['native','before','after']]
preview=Image.new('RGB',(tiles[0].width*3,tiles[0].height+36),(40,40,40));draw=ImageDraw.Draw(preview)
for i,(label,tile) in enumerate(zip(['UIKit','Previous NXKit','Corrected NXKit'],tiles)):
 preview.paste(tile,(i*tile.width,36));draw.text((i*tile.width+12,10),label,fill='white')
preview.save(out/'overview.png',icc_profile=profile)
html='''<!doctype html><meta charset="utf-8"><title>Continuous glass corners</title>
<style>body{font:16px system-ui;background:#202124;color:#eee;margin:24px}a{color:#9dcaff}button,select{font:inherit;padding:8px}nav{display:flex;gap:12px;align-items:center;flex-wrap:wrap}#stage{position:relative;display:inline-block;margin-top:20px;background:repeating-conic-gradient(#666 0 25%,#555 0 50%) 0/16px 16px;max-width:100%}#stage img{display:block;max-width:100%;height:auto}#over{position:absolute;inset:0;width:100%;height:100%}#caption{max-width:900px}input{width:180px}</style>
<h1>Continuous glass corners</h1><p>Fixed native coordinates. Full-resolution images; no fitted registration. <a href="README.md">Evidence and limitations</a> · <a href="metrics.json">Measurements</a></p>
<nav><select id="scene"></select><select id="mode"><option value="overlay">UIKit / corrected overlay</option><option value="native">UIKit</option><option value="before">Previous NXKit</option><option value="after">Corrected NXKit</option><option value="difference">Absolute difference ×4</option><option value="unmatched">Unmatched corrected pixels, &gt;2/255</option><option value="registration">Red UIKit / cyan corrected</option></select><label>NXKit opacity <input id="alpha" type="range" min="0" max="1" step=".01" value=".5"></label></nav>
<p id="caption"></p><div id="stage"><img id="base"><img id="over"></div>
<script>const scenes=SCENES;const scene=document.querySelector('#scene'),mode=document.querySelector('#mode'),alpha=document.querySelector('#alpha'),base=document.querySelector('#base'),over=document.querySelector('#over');scenes.forEach(s=>scene.add(new Option(s.label,s.key)));function update(){const key=scene.value,overlay=mode.value==='overlay';base.src=key+'-'+(overlay?'native':mode.value)+'.png';over.src=key+'-after.png';over.style.display=overlay?'block':'none';over.style.opacity=alpha.value;document.querySelector('#caption').textContent=key.startsWith('shape-')?'Independent solid masks isolate the silhouette and coverage. Native iPadOS 27 masks match the retained 26.4.1 capture byte for byte.':'Actual retained regular glass, both themes. Full-frame optics residuals include existing tint/refraction differences beyond this shape correction.'}scene.onchange=mode.onchange=alpha.oninput=update;update();</script>'''.replace('SCENES',json.dumps(scenes))
(out/'comparison.html').write_text(html)
print(json.dumps({k:v for k,v in metrics.items() if k not in ['solid','glass']},indent=2))
