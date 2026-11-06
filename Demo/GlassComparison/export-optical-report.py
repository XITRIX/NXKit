"""Retain selected aligned pixel comparisons and an offline overlay viewer.
Usage: export-optical-report.py MATRIX BEFORE_SCALE1 BEFORE_SCALE2 AFTER OUTPUT [--all-scenes] [--compact-before] [--html-only] [--single-baseline] [--baseline-revision=REV]
"""
from pathlib import Path
import json,sys,shutil,hashlib
from PIL import Image,ImageCms
from comparison_images import load_srgb,COLOR_MANAGEMENT
matrix,before1,before2,after,out=map(lambda p:Path(p).resolve(),sys.argv[1:6]);out.mkdir(exist_ok=True,parents=True);assets=out/'images';assets.mkdir(exist_ok=True)
metrics=json.loads((after/'metrics.json').read_text());records=json.loads((matrix/'native/series.json').read_text());chosen=[]
if len(metrics['states'])!=len(records):raise RuntimeError('Capture and rendered metric series differ; finish the matrix before export')
profile=ImageCms.ImageCmsProfile(ImageCms.createProfile('sRGB')).tobytes()
for meta in records:
 r=meta['request'];name=r['id']
 detailed=(r['background'] in ['gray','lines','stripes'] and (r['tint']!='coral' or r['background']=='gray') and not (r['tint']=='blue' and r['background']=='lines'))
 if not detailed and '--all-scenes' not in sys.argv:continue
 if not (after/(name+'.ppm')).exists():continue
 files={}
 for label,source in [('native',matrix/'native'/(name+'.png')),('before1',before1/(name+'.ppm')),('before2',before2/(name+'.ppm')),('after',after/(name+'.ppm'))]:
  if label=='before2' and '--single-baseline' in sys.argv:continue
  if label.startswith('before') and not detailed and '--compact-before' in sys.argv:continue
  if source.exists():
   path=assets/(name+'-'+label+'.png')
   if '--html-only' not in sys.argv:load_srgb(source).save(path,icc_profile=profile)
   files[label]='images/'+path.name
 for label,suffix in [('exact','-unmatched-exact.png'),('within2','-unmatched-within2.png'),('difference','-absolute.png'),('redcyan','-red-cyan.png')]:
  source=after/(name+suffix)
  if source.exists():
   if '--html-only' not in sys.argv:shutil.copy2(source,assets/source.name)
   files[label]='images/'+source.name
 chosen.append(dict(name=name,request=r,frames=meta['frames'],files=files))
if (after/'matrix.png').exists():shutil.copy2(after/'matrix.png',out/'overview.png')
(out/'metrics.json').write_text(json.dumps(metrics,indent=2)+'\n')
(out/'native-metadata.json').write_text(json.dumps(records,indent=2)+'\n')
controls=matrix/'native-controls.json'
if controls.exists():shutil.copy2(controls,out/controls.name)
# Self-contained markup uses precomputed image residuals: no canvas readback or local-file permissions.
data=json.dumps(chosen).replace('</','<\\/')
html=r'''<!doctype html><html lang="en"><meta charset="utf-8"><title>UIKit / NXKit aligned glass</title><style>
body{font:15px system-ui,sans-serif;background:#161616;color:#eee;margin:24px}h1{font-size:24px}label{display:inline-block;margin:8px}select,input{font:inherit}p{max-width:1000px;color:#bbb}#viewport{position:relative;overflow:hidden;width:720px;height:440px;background:repeating-conic-gradient(#555 0 25%,#444 0 50%) 50%/16px 16px;border:1px solid #777}#viewport img{position:absolute;width:720px;height:440px;top:0;left:0;image-rendering:auto}#overlay{opacity:.5}#values{white-space:pre-wrap;font:13px ui-monospace,monospace}a{color:#8cf}button{padding:5px 10px;margin:6px}
</style><h1>UIKit and NXKit: aligned glass pixels</h1><p>Native iPadOS 27.0 framebuffer and the production NXKit retained tree on Skia’s raster backend share the observed frame geometry and sRGB color space. Alignment comes from metadata. No fitted shifts, color fitting, or exposure changes are applied. Every scene includes 52, 80, and 120 point circles, a 132 × 56 pill, and a 240 × 160 continuous-corner card.</p>
<label>Scene <select id="scene" aria-label="Scene"></select></label><label>View <select id="mode" aria-label="View"><option value="overlay">Overlay UIKit + NXKit</option><option value="exact">Only unmatched pixels: exact</option><option value="within2">Only unmatched pixels: within 2 / 255</option><option value="difference">Absolute RGB difference</option><option value="redcyan">Red = UIKit, cyan = NXKit</option><option value="native">UIKit</option><option value="after">NXKit after</option><option value="before1">NXKit before: actual scale 1</option><option value="before2">NXKit before: calibrated scale 2</option></select></label><label>Overlay opacity <input id="opacity" aria-label="Overlay opacity" type="range" min="0" max="100" value="50"></label><label>Zoom <select id="zoom" aria-label="Zoom"><option value="1">1×</option><option value="2">2×</option></select></label>
<div id="viewport"><img id="base"><img id="overlay"></div><p>The unmatched modes make pixels transparent when every channel agrees exactly, or differs by at most 2 / 255. The checkerboard is the transparent area; it is not another glass background. Difference is unamplified. Red/cyan indicates spatial disagreement. BASELINE_NOTE</p><div id="values"></div><p><a href="metrics.json">Full matrix metrics</a> · <a href="native-metadata.json">Native geometry and capture metadata</a></p><script>
const cases=CASES;const scene=document.getElementById('scene'),mode=document.getElementById('mode'),base=document.getElementById('base'),overlay=document.getElementById('overlay'),opacity=document.getElementById('opacity');
cases.forEach((c,i)=>{const o=document.createElement('option');o.value=i;o.textContent=c.name;scene.appendChild(o)});
function update(){const c=cases[scene.value];for(const option of mode.options)option.disabled=option.value!=='overlay'&&!c.files[option.value];if(mode.selectedOptions[0].disabled)mode.value='overlay';const m=mode.value;base.style.display='block';overlay.style.display='none';base.style.opacity=1;if(m==='overlay'){base.src=c.files.native;overlay.src=c.files.after;overlay.style.display='block';overlay.style.opacity=opacity.value/100}else{base.src=c.files[m]||c.files.after}const z=Number(document.getElementById('zoom').value),width=z===1?Math.min(720,Math.max(1,window.innerWidth-48)):1440,height=width*440/720;document.getElementById('viewport').style.width=width+'px';document.getElementById('viewport').style.height=height+'px';for(const image of [base,overlay]){image.style.width=width+'px';image.style.height=height+'px'}document.getElementById('values').textContent=c.name+'\nFrames: '+c.frames.map(f=>f[2]+'×'+f[3]+' at '+f[0]+','+f[1]).join(' | ')}
scene.onchange=mode.onchange=opacity.oninput=document.getElementById('zoom').onchange=update;window.addEventListener('resize',update);update();</script></html>'''.replace('CASES',data)
baselineNote='The baseline scale 1 view includes the previous backing-scale bug; scale 2 isolates material changes.' if any('before1' in c['files'] for c in chosen) else 'These pastel holdout captures have no old-renderer baseline; the baseline view controls are disabled.'
if '--single-baseline' in sys.argv:
 revision=next((a.split('=',1)[1] for a in sys.argv if a.startswith('--baseline-revision=')), 'the preserved starting revision')
 baselineNote='Before is the preserved renderer at '+revision+', using the same 2× scale and frame geometry as After.'
 html=html.replace('NXKit before: actual scale 1','NXKit before').replace('<option value="before2">NXKit before: calibrated scale 2</option>','')
html=html.replace('BASELINE_NOTE',baselineNote)
(out/'comparison.html').write_text(html)
provenance=dict(colorManagement=COLOR_MANAGEMENT,nativeOS=sorted(set(m['osBuild'] for m in records)),retainedScenes=len(chosen),fullMatrixScenes=len(records),nativeCapture='simctl io screenshot PNG, explicit sRGB chunk; originals retained in the run directory',geometry='Observed presentation-layer rectangles',fixtureSources={name:hashlib.sha256((Path(__file__).parent/name).read_bytes()).hexdigest() for name in ['OpticalReference.m','OpticalRenderer.cpp']})
if (before1/'baseline-provenance.json').exists():provenance['iterationBaseline']=json.loads((before1/'baseline-provenance.json').read_text())
if (after/'production-build.json').exists():provenance['productionBuild']=json.loads((after/'production-build.json').read_text())
if (matrix/'baseline-provenance.json').exists():provenance['preservedBuildHashes']=json.loads((matrix/'baseline-provenance.json').read_text())
if (matrix/'source-image-validation.json').exists():
 provenance['inputImageValidation']=json.loads((matrix/'source-image-validation.json').read_text())
 shutil.copy2(matrix/'source-image-validation.json',out/'source-image-validation.json')
(out/'provenance.json').write_text(json.dumps(provenance,indent=2)+'\n')
print('Exported',len(chosen),'selected scenes,',len(records)*5,'native material cases')
