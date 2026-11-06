"""Export the retained lens-center regression. Usage: compare-lens-center.py RUN OUTPUT
RUN holds before/after-{theme}-{style}.ppm, center-before/after.log and frozen
before-core.a / after-core.a. Generate fixtures with LensRefractionTests --center-only.
"""
from pathlib import Path
from PIL import Image,ImageCms
import numpy as np,json,hashlib,sys
from PIL import ImageDraw
run,out=(Path(p).resolve() for p in sys.argv[1:]);out.mkdir(parents=True,exist_ok=True);profile=ImageCms.ImageCmsProfile(ImageCms.createProfile('sRGB')).tobytes()
y,x=np.mgrid[:400,:560];expected=np.stack([np.where(x%8<4,23,231),np.where(y%8<4,41,207),91+(x+y)%93],axis=2).astype('uint8')
results=[]
for theme in ['light','dark']:
 for style in ['regular','selection']:
  key=f'{theme}-{style}';n=[]
  for stage in ['before','after']:
   a=np.asarray(Image.open(run/f'{stage}-{key}.ppm').convert('RGB'));Image.fromarray(a).save(out/f'{key}-{stage}.png',icc_profile=profile)
   # Same central ROI used by the executable's 95x54 / x=68.5 fixture.
   region=np.s_[int((40.25+54*.4)*2):int((40.25+54*.6)*2),194:270]
   error=np.abs(a[region].astype(float)-expected[region]);n.append(dict(stage=stage,centerMAE=float(error.mean()),centerMaximum=float(error.max()),mismatchedPixels=int((np.max(error,axis=2)>1).sum())))
  results.append(dict(theme=theme,style=style,metrics=n))
  b=np.asarray(Image.open(out/f'{key}-before.png')).astype(float);a=np.asarray(Image.open(out/f'{key}-after.png')).astype(float);Image.fromarray(np.clip(np.abs(a-b)*4,0,255).astype('uint8')).save(out/f'{key}-difference.png',icc_profile=profile)
tiles=[Image.open(out/f'light-selection-{stage}.png').crop((130,74,336,202)).resize((824,512),Image.Resampling.NEAREST) for stage in ['before','after']]
preview=Image.new('RGB',(1648,540),(50,50,50));draw=ImageDraw.Draw(preview)
for i,(label,tile) in enumerate(zip(['Previous lens','Corrected lens'],tiles)):
 preview.paste(tile,(i*824,28));draw.text((i*824+10,8),label,fill='white')
preview.save(out/'overview.png',icc_profile=profile)
before_failures=(run/'center-before.log').read_text().count('FAIL:')
after_log=(run/'center-after.log').read_text()
assert before_failures==48 and 'Lens center texture checks passed' in after_log and 'FAIL:' not in after_log
assert all(m['centerMaximum']==0 for row in results for m in row['metrics'] if m['stage']=='after')
sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
(out/'metrics.json').write_text(json.dumps(dict(exportedFixture=results,regressionCases=48,beforeFailed=before_failures,afterFailed=after_log.count('FAIL:'),source='Production LensRefractionTests retained rendering, opaque 2x texture; no image registration or postprocessing in scored pixels.',sourceHashes={p:sha(p) for p in ['UIKit/lib/LensMaterial.cpp','UIKit/tests/LensRefractionTests.cpp']},beforeCoreSHA256=sha(run/'before-core.a'),afterCoreSHA256=sha(run/'after-core.a')),indent=2)+'\n')
(out/'comparison.html').write_text('''<!doctype html><meta charset="utf-8"><title>Lens center identity</title><style>body{background:#252525;color:white;font:16px system-ui;margin:24px}select{font:inherit;padding:8px}img{max-width:100%;image-rendering:pixelated}a{color:#add4ff}.row{display:flex;gap:20px;flex-wrap:wrap}.row figure{margin:16px 0}figure img{width:560px}</style><h1>Lens center distortion correction</h1><p>The center must preserve source pixels as the lens moves. Edge folds and chromatic transport remain active. <a href="README.md">Evidence and reproduction</a> · <a href="metrics.json">Measurements</a></p><select id="mode"><option>light-selection</option><option>dark-selection</option><option>light-regular</option><option>dark-regular</option></select><div class="row"><figure><figcaption>Previous</figcaption><img id="before"></figure><figure><figcaption>Corrected</figcaption><img id="after"></figure><figure><figcaption>Absolute change ×4</figcaption><img id="difference"></figure></div><script>let m=document.querySelector('#mode');function update(){for(let id of ['before','after','difference'])document.getElementById(id).src=m.value+'-'+id+'.png'}m.onchange=update;update();</script>''')
print(results)
