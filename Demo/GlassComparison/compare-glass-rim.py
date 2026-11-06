"""Resting regular-glass contour/highlight comparison, including the exterior.

Usage: compare-glass-rim.py BEFORE_RENDERER AFTER_RENDERER OUTPUT
Reuses retained native framebuffer crops; never fits translations or colors.
"""
from pathlib import Path
import hashlib,json,shutil,subprocess,sys,tempfile
import numpy as np
from PIL import Image,ImageCms
from comparison_images import load_srgb

before,after,out=[Path(p).resolve() for p in sys.argv[1:4]]
cachedBefore=next((Path(a.split('=',1)[1]).resolve() for a in sys.argv[4:] if a.startswith('--before-images=')),None)
repo=Path(__file__).resolve().parents[2]
reference=repo/'docs/liquid-glass-optics'
records=json.loads((reference/'native-metadata.json').read_text())
out.mkdir(parents=True,exist_ok=True);images=out/'images';images.mkdir(exist_ok=True)
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
profile=ImageCms.ImageCmsProfile(ImageCms.createProfile('sRGB')).tobytes()
results=[];captureHashes={};backgroundHashes={}
with tempfile.TemporaryDirectory(prefix='nxkit-rim-report-') as directory:
    temp=Path(directory)
    subprocess.run([sys.executable,str(repo/'Demo/GlassComparison/prepare-optical.py'),str(temp)],check=True)
    for version,renderer in [('before',before),('after',after)]:
        if version=='before' and cachedBefore:
            for m in records:
                name=m['request']['id']+'-before.ppm'
                shutil.copy2(cachedBefore/name,temp/name)
            continue
        lines=[]
        for m in records:
            r=m['request'];name=r['id'];background=temp/'backgrounds'/(r['background']+'.png')
            # Native untouched regions independently verify source bytes; ICC
            # container bytes can differ across Pillow versions.
            native=np.asarray(load_srgb(reference/'images'/(name+'-native.png')))
            expected=np.asarray(Image.open(background).convert('RGB')).repeat(2,0).repeat(2,1)
            assert np.array_equal(native[800:860,20:220],expected[800:860,20:220]),name
            backgroundHashes[r['background']]=hashlib.sha256(expected.tobytes()).hexdigest()
            lines.append(' '.join([json.dumps(str(temp/(name+'-'+version+'.ppm'))),json.dumps(str(background)),str(r['dark']),r['tint']]+[str(v) for frame in m['frames'] for v in frame]))
        manifest=temp/(version+'.txt');manifest.write_text('\n'.join(lines)+'\n')
        subprocess.run([str(renderer),str(manifest)],check=True,stdout=subprocess.DEVNULL)
    y,x=np.mgrid[:880,:1440];x=(x+.5)/2;y=(y+.5)/2
    for m in records:
        r=m['request'];name=r['id'];nativeFile=reference/'images'/(name+'-native.png')
        captureHashes[str(nativeFile.relative_to(repo))]=sha(nativeFile)
        native=np.asarray(load_srgb(nativeFile),dtype=np.int16)
        samples={v:np.asarray(Image.open(temp/(name+'-'+v+'.ppm')).convert('RGB'),dtype=np.int16) for v in ['before','after']}
        row=dict(r,elements=[])
        row['beforeAfterMaxChannelError']=int(abs(samples['before']-samples['after']).max())
        if r['background']=='black':
            assert row['beforeAfterMaxChannelError']<=1,(name,'contour must preserve black-backdrop material AA',row['beforeAfterMaxChannelError'])
        for i,(fx,fy,w,h,radius) in enumerate(m['frames']):
            cx=fx+w/2;cy=fy+h/2;qx=abs(x-cx)-(w/2-radius);qy=abs(y-cy)-(h/2-radius)
            d=np.hypot(np.maximum(qx,0),np.maximum(qy,0))+np.minimum(np.maximum(qx,qy),0)-radius
            valid=np.ones(d.shape,dtype=bool)
            if i==4:valid=(abs(x-cx)<w/2-2*radius-2)|(abs(y-cy)<h/2-2*radius-2)
            masks={'exterior':(d>0)&(d<.75)&valid,'highlight':(d<=0)&(d>-3)&valid,'body':d<-18}
            element=dict(index=i,size=[w,h])
            for key,mask in masks.items():
                assert mask.any()
                element[key]={v:float(abs(im-native)[mask].mean()) for v,im in samples.items()}
            row['elements'].append(element)
        results.append(row)
        # First four geometries: native and NXKit remain at identical stage
        # coordinates. Keep the 240x160 card in the numerical score as well.
        crop=(60,36,1250,314)
        a=Image.fromarray(native.astype('uint8')).crop(crop)
        b=Image.fromarray(samples['after'].astype('uint8')).crop(crop)
        c=Image.fromarray(samples['before'].astype('uint8')).crop(crop)
        error=abs(np.asarray(a,dtype=np.int16)-np.asarray(b,dtype=np.int16))
        modes={'native':a,'before':c,'after':b,'overlay':Image.blend(a,b,.5),'difference':Image.fromarray(error.astype('uint8')),
               'residual':Image.fromarray(np.dstack((np.asarray(b),np.where(error.max(2)<=2,0,255).astype('uint8'))))}
        for label,im in modes.items():im.save(images/(name+'-'+label+'.png'),icc_profile=profile)

def aggregate(rows):
    return {region:{v:float(np.mean([e[region][v] for row in rows for e in row['elements']])) for v in ['before','after']} for region in ['exterior','highlight','body']}
summary=dict(all=aggregate(results),neutral=aggregate([r for r in results if r['tint']=='none']),byTheme={str(d):aggregate([r for r in results if r['dark']==d]) for d in [0,1]},byTint={t:aggregate([r for r in results if r['tint']==t]) for t in ['none','blue','coral']})
sources=['UIKit/lib/GlassShader.cpp','UIKit/lib/CABackdropEffectLayer.cpp','UIKit/lib/CAGlassContainerLayer.cpp','UIKit/lib/GlassRim.hpp','UIKit/lib/UIGlassEffect.cpp','Demo/GlassComparison/OpticalRenderer.cpp','Demo/GlassComparison/compare-glass-rim.py','UIKit/tests/GlassRimTests.cpp']
metrics=dict(method='Retained UIKit27.0/24A434 sRGB2x resting frames; observed geometry, no fitted registration/color. Five sizes per scene. Exterior0..0.75pt, inner highlight0..3pt, body>18pt; continuous card corner neighborhoods excluded from edge metrics. Equal scene/geometry weighting. Exterior includes the existing elevation shadow. The four first geometries are shown in the viewer; all five are scored.',summary=summary,records=results,captureSHA256=captureHashes,backgroundPixelSHA256=backgroundHashes,sourceSHA256={f:sha(repo/f) for f in sources},rendererSHA256={v:sha(p) for v,p in [('before',before),('after',after)]},cachedBaselineSHA256={p.name:sha(p) for p in cachedBefore.glob('*.ppm')} if cachedBefore else {})
(out/'metrics.json').write_text(json.dumps(metrics,indent=2)+'\n')
html='''<!doctype html><html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Glass contour and highlights · UIKit / NXKit</title><style>body{font:16px/1.5 system-ui;background:#23242a;color:#eee;margin:24px;max-width:1250px}label{display:inline-block;margin:8px 18px 8px 0}select{font:inherit}img{display:block;width:1190px;max-width:100%}#stage{margin-top:20px;background:repeating-conic-gradient(#555 0 25%,#444 0 50%) 50%/16px 16px}pre{white-space:pre-wrap;font-size:13px}a{color:#9cf}</style><h1>Glass contour and highlights · UIKit / NXKit</h1><p>Both themes, neutral/blue/coral tint and ten backgrounds. The exterior contour is measured separately from the inner white reflection and body. Same fixed stage coordinates, no fitted alignment. Retained UIKit27.0 captures; residual differences remain.</p><label>Scene <select id="scene"></select></label><label>View <select id="mode"><option value="overlay">Overlay50%</option><option value="native">UIKit</option><option value="before">NXKit before</option><option value="after">NXKit after</option><option value="difference">Absolute difference</option><option value="residual">Remove matches within2/255</option></select></label><div id="stage"><img id="image" alt="Aligned glass contour comparison"></div><pre id="score"></pre><p><a href="README.md">Method and limits</a> · <a href="metrics.json">All scores and provenance</a></p><script>const data=__DATA__,scene=document.getElementById('scene'),mode=document.getElementById('mode');data.forEach(r=>scene.add(new Option(r.id,r.id)));scene.value='white-dark0-none';function update(){const r=data.find(x=>x.id===scene.value);document.getElementById('image').src='images/'+r.id+'-'+mode.value+'.png';document.getElementById('score').textContent=JSON.stringify(r,null,2)}scene.onchange=mode.onchange=update;update()</script></html>'''
(out/'comparison.html').write_text(html.replace('__DATA__',json.dumps(results)))
print(json.dumps(summary,indent=2))
