"""Selected ink dispersion, independently of ordinary backdrop fringes.

Usage: compare-selected-lens-radial.py BEFORE_RENDERER AFTER_RENDERER OUTPUT
Uses retained UIKit marker framebuffers/source assets and rich-source validation
frames. Source-only markers never contain ordinary-row ink behind the lens.
"""
from pathlib import Path
import hashlib,json,subprocess,sys,tempfile
import numpy as np
from PIL import Image,ImageCms

before,after,out=map(lambda s:Path(s).resolve(),sys.argv[1:4]);out.mkdir(parents=True,exist_ok=True)
repo=Path(__file__).resolve().parents[2];raw=repo/'docs/liquid-glass-lens/native/edge-probes'
reference=repo/'docs/liquid-glass-lens/refraction';states={r['name']:r for r in json.loads((reference/'metrics.json').read_text())['states']}
profile=ImageCms.ImageCmsProfile(ImageCms.createProfile('sRGB')).tobytes()
images=out/'images';images.mkdir(exist_ok=True)
work=tempfile.TemporaryDirectory(prefix='nxkit-selected-ca-');temp=Path(work.name)
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
cases=[];captures={};inputs={}
def add(name,dark,style,setup,control,background='gray',markers=False,validation=False):
    if name in [r['id'] for r in cases]:return
    if name in states:assert np.allclose(states[name]['frame'],[302.25,260,116,56]),name
    nativeFile=reference/'images'/(name+'-native.png')
    if not nativeFile.exists():nativeFile=raw/(name+'.png')
    native=Image.open(nativeFile).convert('RGB')
    if native.height==1120:native=native.crop((0,372,1440,780))
    assert native.size==(1440,408)
    captures[str(nativeFile.relative_to(repo))]=sha(nativeFile)
    color={'gray':(128,128,128),'blue':(0,0,255),'red':(255,0,0),'green':(0,255,0)}[background]
    bg=temp/(background+'.png');Image.new('RGB',(720,560),color).save(bg,icc_profile=profile)
    source=raw/('probe-markers-visible-d0-s1-selected.png' if markers else 'probe-pilot-portal-selected.png')
    normal=raw/('probe-markers-visible-d0-s1-normal.png' if markers else 'probe-pilot-portal-normal.png')
    for p in [source,normal]:inputs[str(p.relative_to(repo))]=sha(p)
    cases.append(dict(id=name,dark=dark,style=style,setup=setup,control=control,background=background,markers=markers,validation=validation,native=native,source=source,normal=normal,bg=bg))
for dark in (0,1):
    for style in (0,1):
        control=f'edges-dark{dark}-style{style}-none';add(control,dark,style,'none',None,markers=True)
        name=f'markers-visible-d{dark}-s{style}';m=json.loads((raw/(name+'.json')).read_text())
        assert m['request']['markerPoints']==[[320.25,18],[320.25,58],[270.25,38],[370.25,38],[282.25,22],[358.25,22]]
        add(name,dark,style,'portal-only',control,markers=True)
add('pilot-none',0,1,'none',None,validation=True)
for name,setup in [('pilot-portal-only','portal-only'),('pilot-portal','portal'),('pilot-underlay','underlay')]:add(name,0,1,setup,'pilot-none',validation=True)
for color in ('blue','red','green'):
    control=f'colors-d0-{color}-none';add(control,0,1,'none',None,color,validation=True)
    add(f'colors-d0-{color}',0,1,'portal',control,color,validation=True)
rendered={}
for revision,renderer in [('before',before),('after',after)]:
    rows=[]
    for c in cases:
        output=temp/(c['id']+'-'+revision+'.ppm')
        fields=[str(output),str(c['bg']),str(c['normal']),str(c['source'])]
        rows.append(' '.join(json.dumps(v) for v in fields)+f" {c['dark']} {c['style']} 1 "+json.dumps(c['setup'])+' 1 1 1 0 302.25 260 116 56 -64 -64 768 204 1')
    manifest=temp/(revision+'.txt');manifest.write_text('\n'.join(rows)+'\n')
    subprocess.run([str(renderer),str(manifest)],check=True,stdout=subprocess.DEVNULL)
    for c in cases:
        p=temp/(c['id']+'-'+revision+'.ppm')
        rendered[c['id'],revision]=np.asarray(Image.open(p).convert('RGB').crop((0,372,1440,780)),dtype=np.int16);p.unlink()
yy,xx=np.mgrid[:408,:1440];X=(xx+.5)/2;Y=(yy+372+.5)/2
qX=np.maximum(abs(X-360.25)-30,0);qY=abs(Y-288);depth=28-np.hypot(qX,qY)
edge=(depth>.5)&(depth<18);center=depth>=18
rois=[[700,520,744,550],[700,602,744,633],[604,554,640,596],[804,554,837,596],[620,514,668,574],[772,514,820,574]]
palette=np.array([[255,0,0],[0,255,0],[0,0,255],[255,0,255],[0,255,255],[255,255,0]])
names=['top-red','bottom-green','left-blue','right-magenta','corner-cyan','corner-yellow']
def coverage(image,control,roi,color):
    x,y,x1,y1=roi;a=image[y-372:y1-372,x:x1].astype(float);b=control[y-372:y1-372,x:x1].astype(float)
    denom=color-b;return np.clip(np.divide(a-b,denom,out=np.zeros_like(a),where=abs(denom)>=16),0,1)
def centroid(weights,roi):
    x,y,x1,y1=roi;w=np.maximum(weights-.1,0);cx,cy=np.meshgrid((np.arange(x,x1)+.5)/2,(np.arange(y,y1)+.5)/2);mass=w.sum((0,1))
    return np.stack([(cx[...,None]*w).sum((0,1))/mass,(cy[...,None]*w).sum((0,1))/mass],axis=1)
records=[];lookup={c['id']:c for c in cases}
for c in cases:
    name=c['id'];native=np.asarray(c['native'],dtype=np.int16);a=rendered[name,'before'];b=rendered[name,'after']
    record={k:c[k] for k in ['id','dark','style','setup','control','background','markers','validation']}
    record['channelMAE']={v:float(abs(rendered[name,v]-native)[edge].mean()) for v in ['before','after']}
    if c['setup']=='none' or c['style']==0 or c['setup']=='underlay':assert np.array_equal(a,b),name
    assert np.array_equal(a[center],b[center]),(name,'center')
    if c['control']:
        control=c['control'];nativeControl=np.asarray(lookup[control]['native'],dtype=np.int16)
        record['inkSignalMAE']={v:float(abs((rendered[name,v]-rendered[control,v])-(native-nativeControl))[edge].mean()) for v in ['before','after']}
        if c['markers']:
            points=[]
            for i,(roi,rgb) in enumerate(zip(rois,palette)):
                cov={v:coverage(im,ctl,roi,rgb) for v,im,ctl in [('native',native,nativeControl),('before',a,rendered[control,'before']),('after',b,rendered[control,'after'])]}
                centers={v:centroid(w,roi) for v,w in cov.items()}
                point=dict(id=names[i],sourceRGB=rgb.tolist(),roi=roi,centroids={v:p.tolist() for v,p in centers.items()},coverageMAE={v:float(abs(cov[v]-cov['native']).mean()) for v in ['before','after']},centroidError={v:float(np.linalg.norm(centers[v]-centers['native'],axis=1).mean()) for v in ['before','after']})
                if i<4:
                    axis=0 if i<2 else 1;sign=-1 if i in (1,3) else 1
                    point['GBSeparation']={v:float(sign*(p[1,axis]-p[2,axis])) for v,p in centers.items()}
                    if i in (2,3):
                        point['radialGBSeparation']={v:float((p[2,0]-p[1,0])*(1 if i==2 else -1)) for v,p in centers.items()}
                        if c['style']==1:assert abs(point['radialGBSeparation']['after']-point['radialGBSeparation']['native'])<abs(point['radialGBSeparation']['before']-point['radialGBSeparation']['native'])
                points.append(point)
            record['points']=points
    # Absolute stage crop with a fixed 12pt surround; no registration or warps.
    crop=(580,124,860,284)
    nativeImage=c['native'].crop(crop);beforeImage=Image.fromarray(a.astype('uint8')).crop(crop);afterImage=Image.fromarray(b.astype('uint8')).crop(crop)
    files={'native':nativeImage,'before':beforeImage,'after':afterImage,'overlay':Image.blend(nativeImage,afterImage,.5)}
    error=abs(np.asarray(nativeImage,dtype=np.int16)-np.asarray(afterImage,dtype=np.int16));files['difference']=Image.fromarray(error.astype('uint8'))
    files['residual']=Image.fromarray(np.dstack((np.asarray(afterImage),np.where(error.max(2)<=2,0,255).astype('uint8'))))
    for label,im in files.items():im.save(images/(name+'-'+label+'.png'),icc_profile=profile)
    records.append(record)
selected=[r for r in records if r['markers'] and r['style']==1 and r['setup']!='none']
summary={v:dict(radialGBSeparationError=float(np.mean([abs(p['radialGBSeparation'][v]-p['radialGBSeparation']['native']) for r in selected for p in r['points'][2:4]])),axialGBSeparationError=float(np.mean([abs(p['GBSeparation'][v]-p['GBSeparation']['native']) for r in selected for p in r['points'][:4]])),allMarkerCentroidError=float(np.mean([p['centroidError'][v] for r in selected for p in r['points']])),allMarkerCoverageMAE=float(np.mean([p['coverageMAE'][v] for r in selected for p in r['points']]))) for v in ['before','after']}
sources=['UIKit/lib/LensMaterial.cpp','UIKit/tests/LensRefractionTests.cpp','Demo/GlassComparison/LensRefractionRenderer.cpp','Demo/GlassComparison/compare-selected-lens-radial.py']
result=dict(method='Retained UIKit 27.0 2x framebuffers and native-exported RGBA inputs; independent shader-only controls remove material/rim color. Per-channel coverage=(image-control)/(sourceRGB-control), clipped0..1; centroid weights=max(coverage-.1,0). Six fixed marker ROIs; Side-normal G/B transport is the calibration target; existing tangential transport is also scored. Corner ROI tradeoffs remain explicit. Rich-grid scenes are validation. Regular, no-source, ordinary-underlay and deep-center outputs must remain byte-identical. No fitted pixel registration or color fitting.',summary=summary,records=records,captureSHA256=captures,inputSHA256=inputs,sourceSHA256={p:sha(repo/p) for p in sources},rendererSHA256={'before':sha(before),'after':sha(after)})
(out/'metrics.json').write_text(json.dumps(result,indent=2)+'\n')
page='''<!doctype html><html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Tinted lens radial dispersion · UIKit / NXKit</title><style>body{font:16px/1.5 system-ui;background:#191a1e;color:#eee;margin:24px;max-width:1100px}label{display:inline-block;margin:8px 20px 8px 0}select{font:inherit}img{display:block;width:560px;max-width:100%;margin:16px 0}pre{white-space:pre-wrap;font-size:13px}a{color:#9cf}</style><h1>Tinted lens radial dispersion · UIKit / NXKit</h1><p>Selected-source-only colored markers: ordinary white icons are absent. Both themes compare the native side-normal and tangential channel separation against before/after NXKit. Rich-source and colored-background cases validate the adjustment. Fixed stage coordinates; no image fitting. Corner tradeoffs and residual optical errors remain visible.</p><label>Scene <select id="scene"></select></label><label>View <select id="mode"><option value="overlay">Overlay50%</option><option value="native">UIKit</option><option value="before">NXKit before</option><option value="after">NXKit after</option><option value="difference">Absolute difference</option><option value="residual">Remove matches within2/255</option></select></label><img id="image" alt="Tinted source radial separation comparison"><pre id="score"></pre><p><a href="README.md">Method and limits</a> · <a href="metrics.json">All measurements/provenance</a></p><script>const data=__DATA__,scene=document.getElementById('scene'),mode=document.getElementById('mode');data.forEach(r=>scene.add(new Option(r.id,r.id)));scene.value='pilot-portal';function update(){const r=data.find(x=>x.id===scene.value);document.getElementById('image').src='images/'+r.id+'-'+mode.value+'.png';document.getElementById('score').textContent=JSON.stringify(r,null,2)}scene.onchange=mode.onchange=update;update()</script></html>'''
(out/'comparison.html').write_text(page.replace('__DATA__',json.dumps(records)))
print(json.dumps(summary,indent=2));work.cleanup()
