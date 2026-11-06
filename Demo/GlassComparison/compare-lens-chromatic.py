"""Compare background-only lens dispersion at observed native geometry.

Usage: compare-lens-chromatic.py RUN BEFORE AFTER OUTPUT [--additional-run=RUN]
No selected portal is installed. Exact source pixels and native rectangles are
reused; no image registration, tone fitting, or image-space correction occurs.
"""
from pathlib import Path
import hashlib,json,os,subprocess,sys,tempfile
import numpy as np
from PIL import Image,ImageCms
from comparison_images import load_srgb,COLOR_MANAGEMENT
run,before,after,out=(Path(s).resolve() for s in sys.argv[1:5]);out.mkdir(parents=True,exist_ok=True)
runs=[run]+[Path(s.split('=',1)[1]).resolve() for s in sys.argv if s.startswith('--additional-run=')]
only=next((set(s.split('=',1)[1].split(',')) for s in sys.argv if s.startswith('--only=')),None)
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
profile=ImageCms.ImageCmsProfile(ImageCms.createProfile('sRGB')).tobytes();assets=out/'images';assets.mkdir(exist_ok=True)
records=[]
for folder in runs:
 for file in sorted((folder/'native').glob('*.json')):
  m=json.loads(file.read_text())
  if not isinstance(m,dict):continue
  r=m.get('request',{});name=r.get('id',file.stem)
  if not name.startswith(('backdrop-','colors-')):continue
  if only and name not in only:continue
  if r.get('contentSetup')!='none' or r.get('hideLens') or not r.get('warps',True):continue
  def asset(filename):return next(p for p in [folder/'LensReference.app'/filename,folder/'native/source-assets'/filename,folder/'native'/filename] if p.is_file())
  frame=next(x['frame'] for x in m['hierarchy'] if x.get('identifier')=='manual-lens')
  records.append(dict(name=name,request=r,frame=frame,nativeBuild={k:m[k] for k in ['nativeExecutableSHA256','nativeFixtureSourceSHA256','nativeFixtureSourceSnapshot','captureColorSpace'] if k in m},sourceFrame=m['sourceFrameInBar'],background=asset(r['background']+'.png'),normal=asset(m['sourceAssets']['normal']),selected=asset(m['sourceAssets']['selected']),native=file.with_suffix('.png'),metadata=file))
if not records:raise ValueError('No background-only captures')
cache=next((Path(s.split('=',1)[1]).resolve() for s in sys.argv if s.startswith('--cache=')),run/'chromatic-raster-cache');cache.mkdir(parents=True,exist_ok=True);temp=tempfile.TemporaryDirectory();raw=Path(temp.name);images={}
for label,renderer in [('before',before),('after',after)]:
 folder=cache/sha(renderer);folder.mkdir(exist_ok=True);pending=[];rows=[];images[label]={}
 for q in records:
  r=q['request'];key=hashlib.sha256(json.dumps([r,q['frame'],q['sourceFrame'],[sha(q[k]) for k in ['background','normal','selected']]],sort_keys=True).encode()).hexdigest()[:16]
  # Experimental overrides are scoped to candidate report directories; include
  # their exact bytes in cache keys so changing a shader never reuses pixels.
  override=os.environ.get('NXKIT_LENS_SHADER_OVERRIDE') if label=='after' else None
  if override:key+='-'+sha(Path(override))[:12]
  cached=folder/(q['name']+'-'+key+'.png');images[label][q['name']]=cached
  if cached.exists():continue
  ppm=raw/(q['name']+'-'+label+'.ppm');row=' '.join(json.dumps(str(s)) for s in [ppm,q['background'],q['normal'],q['selected']])
  row+=' '+' '.join([str(int(bool(r.get('dark')))),str(r.get('style',1)),str(int(r.get('lifted',True))),'"none"','1','1',str({'none':0,'regular':1,'clear':2}[r.get('base','regular')]),'0']+[str(v) for v in q['frame']+q['sourceFrame']]+['1']);rows.append(row);pending.append((ppm,cached))
 if rows:
  manifest=raw/(label+'.txt');manifest.write_text('\n'.join(rows)+'\n');env=os.environ.copy()
  if label=='before':env.pop('NXKIT_LENS_SHADER_OVERRIDE',None)
  subprocess.run([str(renderer),str(manifest)],env=env,check=True,stdout=subprocess.DEVNULL)
  for ppm,cached in pending:load_srgb(ppm).save(cached,icc_profile=profile);ppm.unlink()
yy,xx=np.mgrid[:1120,:1440];xx=(xx+.5)/2;yy=(yy+.5)/2
native={q['name']:np.asarray(load_srgb(q['native']),float) for q in records};by_name={q['name']:q for q in records};results=[]
for q in records:
 name=q['name'];r=q['request'];x,y,w,h=q['frame'];rad=min(w,h)/2;qx=abs(xx-x-w/2)-(w/2-rad);qy=abs(yy-y-h/2)-(h/2-rad);d=np.hypot(np.maximum(qx,0),np.maximum(qy,0))+np.minimum(np.maximum(qx,qy),0)-rad
 band=(d<-.5)&(d>-10);vertical=abs((yy-y-h/2)/(h/2))>=abs((xx-x-w/2)/(w/2));masks=dict(perimeter=band,transition=(d<=-10)&(d>-18),center=d<=-18)
 masks.update({k:band&m for k,m in dict(top=vertical&(yy<y+h/2),bottom=vertical&(yy>=y+h/2),left=~vertical&(xx<x+w/2),right=~vertical&(xx>=x+w/2)).items()})
 a=native[name];row=dict(name=name,request=r,frame=q['frame'],metrics={},files={},markers=[])
 crop=(0,372,1440,780)
 def save(label,array):
  file=assets/(name+'-'+label+'.png');Image.fromarray(array.astype('uint8')).crop(crop).save(file,icc_profile=profile);row['files'][label]='images/'+file.name
 save('native',a)
 raster={k:np.asarray(load_srgb(images[k][name]),float) for k in ['before','after']}
 for label,b in raster.items():
  delta=b-a;chroma=delta-delta.mean(axis=2,keepdims=True)
  row['metrics'][label]={k:dict(pixels=int(mask.sum()),rgbMAE=float(abs(delta)[mask].mean()) if mask.any() else None,chromaMAE=float(abs(chroma)[mask].mean()) if mask.any() else None) for k,mask in masks.items()}
  save(label,b);save(label+'Difference',abs(delta))
  for tolerance,title in [(0,'Exact'),(2,'Within2')]:
   alpha=(abs(delta).max(axis=2)>tolerance).astype('uint8')*255;save(label+title,np.dstack([b,alpha]))
 control=by_name.get(r.get('controlID'));markers=r.get('backgroundMarkers',[])
 if control and control['name']!=name and markers:
  nearest=np.argmin(np.stack([(xx-m['center'][0])**2+(yy-m['center'][1])**2 for m in markers]),axis=0)
  for index,marker in enumerate(markers):
   mx,my=marker['center'];roi=(d<-.5)&(np.hypot(xx-mx,yy-my)<15)&(nearest==index);entry=dict(sourceCenter=marker['center'],RGB=marker['RGB'],channels={})
   for label,b in dict(native=a,**raster).items():
    c=native[control['name']] if label=='native' else np.asarray(load_srgb(images[label][control['name']]),float);den=np.array(marker['RGB'])-c;coverage=np.divide(b-c,den,out=np.zeros_like(b),where=abs(den)>16);weights=np.maximum(coverage-.1,0)*roi[:,:,None];mass=weights.sum(axis=(0,1));centroids=np.array([(weights*xx[:,:,None]).sum(axis=(0,1))/np.maximum(mass,1e-6),(weights*yy[:,:,None]).sum(axis=(0,1))/np.maximum(mass,1e-6)]).T
    variance=np.array([(weights*(xx[:,:,None]-centroids[:,0])**2).sum(axis=(0,1))/np.maximum(mass,1e-6),(weights*(yy[:,:,None]-centroids[:,1])**2).sum(axis=(0,1))/np.maximum(mass,1e-6)]).T
    entry['channels'][label]=dict(centroids=centroids.tolist(),mass=mass.tolist(),sigma=np.sqrt(variance).tolist(),RminusB=(centroids[0]-centroids[2]).tolist())
   valid=np.array(entry['channels']['native']['mass'])>1
   entry['nativeMeasuredChannels']=valid.tolist()
   entry['centroidError']={}
   for k in ['before','after']:
    recovered=valid&(np.array(entry['channels'][k]['mass'])>1)
    entry['centroidError'][k]=float(np.linalg.norm(np.array(entry['channels'][k]['centroids'])-entry['channels']['native']['centroids'],axis=1)[recovered].mean()) if recovered.any() else None
    entry[k+'MissingChannels']=int((valid&~recovered).sum())
   row['markers'].append(entry)
 results.append(row)
aggregate={}
# Validation excludes uniform controls and nonlifted semantic controls. Scores
# remain broken down by base/style/theme in each row rather than hiding a path.
scored=[r for r in results if r['request'].get('lifted',True) and r['request'].get('background')!='gray']
for label in ['before','after']:
 aggregate[label]={k:{metric:float(np.mean([r['metrics'][label][k][metric] for r in scored if r['metrics'][label][k][metric] is not None])) for metric in ['rgbMAE','chromaMAE']} for k in ['perimeter','top','bottom','left','right','transition','center']}
(out/'metrics.json').write_text(json.dumps(dict(sceneCount=len(records),scoredSceneCount=len(scored),aggregate=aggregate,scenes=results),indent=2)+'\n')
provenance=dict(colorManagement=COLOR_MANAGEMENT,geometry='Observed native2x rectangles; no image fitting or post-render correction.',before=json.loads((before.parent/'build-provenance.json').read_text()),after=json.loads((after.parent/'build-provenance.json').read_text()),captures={q['name']:dict(metadataSHA256=sha(q['metadata']),nativeSHA256=sha(q['native']),backgroundSHA256=sha(q['background']),**q['nativeBuild']) for q in records})
if os.environ.get('NXKIT_LENS_SHADER_OVERRIDE'):provenance['experimentalOverrideSHA256']=sha(Path(os.environ['NXKIT_LENS_SHADER_OVERRIDE']))
material=Path(__file__).resolve().parents[2]/'UIKit/lib/LensMaterial.cpp'
if material.is_file():provenance['materialSourceAtComparison']=dict(path=str(material),SHA256=sha(material))
(out/'provenance.json').write_text(json.dumps(provenance,indent=2)+'\n')
html='''<!doctype html><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Lens background chromatic refraction</title><style>body{font:15px system-ui;background:#181818;color:#eee;margin:24px}label{display:inline-block;margin:6px}select{font:inherit}#stage{position:relative;overflow:hidden;background:repeating-conic-gradient(#555 0 25%,#444 0 50%)50%/16px 16px}img{position:absolute}a{color:#9cf}p{max-width:900px}</style><h1>Lens background chromatic refraction</h1><p>Identical background pixels, no selected content. Native geometry determines the exact alignment. Transparent residuals remove matching pixels; all scores use original full-resolution images.</p><label>Scene <select id="scene" aria-label="Scene"></select></label><label>Revision <select id="revision" aria-label="Revision"><option>after</option><option>before</option></select></label><label>View <select id="mode" aria-label="View"><option value="overlay">Overlay</option><option value="native">UIKit</option><option value="nxkit">NXKit</option><option value="Exact">Unmatched exact</option><option value="Within2">Unmatched within2/255</option><option value="Difference">Absolute difference</option></select></label><label>Opacity <input id="opacity" aria-label="Overlay opacity" type="range" min="0" max="100" value="50"></label><label>Zoom <select id="zoom" aria-label="Zoom"><option value="closeup">Lens close-up</option><option value="fit">Fit stage</option><option value="pixels">Actual pixels</option></select></label><div id="stage"><img id="base"><img id="top"></div><pre id="scores"></pre><p><a href="metrics.json">Full metrics</a> · <a href="provenance.json">Provenance</a> · <a href="README.md">Evidence and limits</a></p><script>const cases=DATA;const e=id=>document.getElementById(id),scene=e('scene'),revision=e('revision'),mode=e('mode'),opacity=e('opacity'),zoom=e('zoom'),base=e('base'),topImage=e('top'),stage=e('stage');cases.forEach((r,i)=>{let o=document.createElement('option');o.value=i;o.textContent=r.name;scene.appendChild(o)});scene.value=Math.max(0,cases.findIndex(r=>r.name==='backdrop-grid-d0-none-w1'));function update(){const r=cases[scene.value],v=revision.value,m=mode.value;base.src=r.files[m==='overlay'?'native':m==='nxkit'?v:m==='native'?m:v+m];topImage.src=r.files[v];topImage.style.display=m==='overlay'?'block':'none';topImage.style.opacity=opacity.value/100;let w=zoom.value==='pixels'?1440:Math.min(720,innerWidth-48),scale=w/720,h=204*scale,left=0,top=0;if(zoom.value==='closeup'){const f=r.frame,x0=Math.max(0,Math.floor((f[0]-12)*2)/2),y0=Math.max(0,Math.floor((f[1]-186-12)*2)/2),x1=Math.min(720,Math.ceil((f[0]+f[2]+12)*2)/2),y1=Math.min(204,Math.ceil((f[1]-186+f[3]+12)*2)/2);scale=w/(x1-x0);h=(y1-y0)*scale;left=-x0*scale;top=-y0*scale}stage.style.width=w+'px';stage.style.height=h+'px';for(const im of[base,topImage]){im.style.left=left+'px';im.style.top=top+'px';im.style.width=720*scale+'px';im.style.height=204*scale+'px'}e('scores').textContent=JSON.stringify(r.metrics[v],null,2)}for(const el of[scene,revision,mode,zoom])el.onchange=update;opacity.oninput=update;addEventListener('resize',update);update();</script>'''.replace('DATA',json.dumps(results).replace('</','<\\/'))
(out/'comparison.html').write_text(html)
print(json.dumps(dict(sceneCount=len(records),scoredSceneCount=len(scored),aggregate=aggregate),indent=2))
