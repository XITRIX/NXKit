"""Compare the live Demo glass-platter/lens composition to standalone native UIKit.
Usage: compare-lens.py NATIVE_RUN RENDERER OUTPUT
Full native inputs remain untouched; display crops are fixed stage coordinates.
"""
from pathlib import Path
import hashlib,json,subprocess,sys,tempfile
import numpy as np
from PIL import Image,ImageCms
from comparison_images import load_srgb
run,renderer,out=map(lambda x:Path(x).resolve(),sys.argv[1:4]);out.mkdir(parents=True,exist_ok=True)
source=Path(__file__).resolve().parent;content=source.parent.parent/'docs/liquid-glass-lens/native/content'
content=next((Path(a.split('=',1)[1]).resolve() for a in sys.argv[4:] if a.startswith('--content=')),content)
cases=json.loads((source/'lens-cases.json').read_text());cases=[r for r in cases if r.get('action')!='motion' and not r.get('exportContent')]
raw_run=tempfile.TemporaryDirectory(prefix='nxkit-lens-pixels-');raw=Path(raw_run.name)
only=next((set(a.split('=',1)[1].split(',')) for a in sys.argv[4:] if a.startswith('--only=')),None)
if only: cases=[r for r in cases if r['id'] in only]
records=[];manifest=[]
for request in cases:
 name=request['id'];metadata=json.loads((run/'native'/(name+'.json')).read_text())
 rows=[r for r in metadata['hierarchy'] if r['class']=='_UILiquidLensView'];manual=next(r for r in rows if r['identifier']=='manual-lens');samples=[r for r in rows if r['frame'][1]>350]
 if len(samples)!=3:raise ValueError((name,len(samples)))
 dark=request['dark'];style=metadata['manualLens']['style'];lifted=metadata['manualLens']['lifted'];selected=request.get('selected',1)
 frames=[manual['frame']]+[r['frame'] for r in samples];record=dict(name=name,request=request,frames=frames,style=style,lifted=lifted,nativeExecutableSHA256=metadata['nativeExecutableSHA256'],nativeFixtureSourceSHA256=metadata['nativeFixtureSourceSHA256'])
 records.append(record)
 fields=[json.dumps(str(raw/(name+'.ppm'))),json.dumps(str(run/'LensReference.app'/(request['background']+'.png'))),json.dumps(str(content/f'content-dark{dark}-normal.png')),json.dumps(str(content/f'content-dark{dark}-selected.png')),str(dark),str(style),str(int(lifted)),str(selected)]+[str(v) for f in frames for v in f]
 manifest.append(' '.join(fields))
(raw/'manifest.txt').write_text('\n'.join(manifest)+'\n');subprocess.run([str(renderer),str(raw/'manifest.txt')],check=True,stdout=subprocess.DEVNULL)
y,x=np.mgrid[:1120,:1440];x=(x+.5)/2;y=(y+.5)/2
profile=ImageCms.ImageCmsProfile(ImageCms.createProfile('sRGB')).tobytes();results=[];assets=out/'images';assets.mkdir(exist_ok=True)
def capsule(f):
 fx,fy,w,h=f;r=min(w,h)/2;qx=abs(x-(fx+w/2))-(w/2-r);qy=abs(y-(fy+h/2))-(h/2-r)
 return np.hypot(np.maximum(qx,0),np.maximum(qy,0))+np.minimum(np.maximum(qx,qy),0)-r
crop=(0,460,1440,1020)
for record in records:
 name=record['name'];a=np.asarray(load_srgb(run/'native'/(name+'.png')),float);b=np.asarray(load_srgb(raw/(name+'.ppm')),float);diff=abs(a-b);maxdiff=diff.max(2)
 samples=[]
 for i,f in enumerate(record['frames']):
  d=capsule(f);inside=d<-.5;body=d<-min(12,min(f[2:])*.2);rim=(d<-.5)&(d>-2.5);band=(d<-2.5)&(d>-12)
  samples.append(dict(index=i,frame=f,insideMAE=float(diff[inside].mean()),bodyMAE=float(diff[body].mean()),rimMAE=float(diff[rim].mean()),bandMAE=float(diff[band].mean()),pixels=int(inside.sum())))
 record['elements']=samples;record['files']={}
 for label,array in [('native',a),('nxkit',b),('difference',diff),('redcyan',np.stack([a.mean(2),b.mean(2),b.mean(2)],2))]:
  file=assets/f'{name}-{label}.png';Image.fromarray(array.clip(0,255).astype('uint8')).crop(crop).save(file,icc_profile=profile);record['files'][label]='images/'+file.name
 for threshold,label in [(0,'exact'),(2,'within2')]:
  rgba=np.concatenate([b.astype('uint8'),np.where(maxdiff>threshold,255,0).astype('uint8')[...,None]],2);file=assets/f'{name}-{label}.png';Image.fromarray(rgba).crop(crop).save(file,icc_profile=profile);record['files'][label]='images/'+file.name
 results.append(record)
summary=dict(runtime='iPadOS 27.0 build24A434',alignment='Observed native frames; fixed display crop y230..510; no fitted transforms/colors',captureCount=len(results),standaloneCount=len(results)*3,states=results)
summary['aggregate']={field:float(np.mean([e[field] for r in results for e in r['elements'][1:]])) for field in ['insideMAE','bodyMAE','rimMAE','bandMAE']}
summary['manualComposition']={field:float(np.mean([r['elements'][0][field] for r in results])) for field in ['insideMAE','bodyMAE','rimMAE','bandMAE']}
(out/'metrics.json').write_text(json.dumps(summary,indent=2)+'\n');(out/'provenance.json').write_text(json.dumps(dict(rendererSHA256=hashlib.sha256(renderer.read_bytes()).hexdigest(),coreArchiveSHA256=hashlib.sha256((renderer.parent/'libLensCore.a').read_bytes()).hexdigest(),fixtureHashes=sorted(set(r['nativeFixtureSourceSHA256'] for r in results)),selectedContentAssets={f.name:hashlib.sha256(f.read_bytes()).hexdigest() for f in sorted(content.glob('*.png'))},method=summary['alignment']),indent=2)+'\n')
data=json.dumps(results).replace('</','<\\/')
html='''<!doctype html><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>UIKit / NXKit lenses</title><style>body{font:15px system-ui;background:#181818;color:#eee;margin:24px}h1{font-size:24px}p{max-width:900px}label{display:inline-block;margin:6px}a{color:#9cf}#stage{position:relative;background:repeating-conic-gradient(#555 0 25%,#444 0 50%) 50%/16px 16px}img{position:absolute;inset:0}select{font:inherit}</style><h1>UIKit / NXKit centered-content controls</h1><p>These fixtures mainly place selected ink in the flat center. Use the <a href="../refraction/comparison.html">edge-refraction tests</a> for perimeter sampling, dispersion and source crossing the lens edges.</p><p>The top row uses the existing regular glass and a separate LensView selection. The three lower shapes isolate lenses at different sizes. UIKit supplies the reference and identical typography assets for this comparison; the live Demo uses portable icons. Both themes, both explicit styles and multiple color backgrounds are retained. Geometry comes from native metadata; no pixel/color fitting is applied.</p><label>Scene <select id="scene" aria-label="Scene"></select></label><label>View <select id="mode" aria-label="View"><option value="overlay">Overlay</option><option value="native">UIKit</option><option value="nxkit">NXKit</option><option value="exact">Unmatched · exact</option><option value="within2">Unmatched · within 2/255</option><option value="difference">Absolute difference</option><option value="redcyan">Red/cyan registration</option></select></label><label>Opacity <input id="opacity" aria-label="Opacity" type="range" min="0" max="100" value="50"></label><label>Zoom <select id="zoom" aria-label="Zoom"><option value="fit">Fit</option><option value="pixels">Actual pixels</option></select></label><div id="stage"><img id="base"><img id="top"></div><p>Transparent checkerboard marks suppressed pixels. The source crop is fixed at stage y230–510 points; system controls above this row are outside the compared composition. Lifted geometry is caller-owned. Native acceleration/stretch and system-control interaction timing remain separate from these static optical tests.</p><p><a href="metrics.json">Metrics</a> · <a href="provenance.json">Provenance</a> · <a href="../research.md">Research and limits</a></p><script>const cases=CASES;const scene=document.getElementById('scene'),mode=document.getElementById('mode'),opacity=document.getElementById('opacity'),zoom=document.getElementById('zoom'),base=document.getElementById('base'),topImage=document.getElementById('top'),stage=document.getElementById('stage');cases.forEach((r,i)=>{let o=document.createElement('option');o.value=i;o.textContent=r.name;scene.appendChild(o)});function update(){const r=cases[scene.value],m=mode.value;base.src=r.files[m==='overlay'?'native':m];topImage.style.display=m==='overlay'?'block':'none';topImage.src=r.files.nxkit;topImage.style.opacity=opacity.value/100;const w=zoom.value==='pixels'?1440:Math.min(720,innerWidth-48),h=w*280/720;stage.style.width=w+'px';stage.style.height=h+'px';for(const im of[base,topImage]){im.style.width=w+'px';im.style.height=h+'px'}}scene.onchange=mode.onchange=opacity.oninput=zoom.onchange=update;addEventListener('resize',update);update();</script>'''.replace('CASES',data)
(out/'comparison.html').write_text(html);raw_run.cleanup();print(json.dumps(summary['aggregate']))
