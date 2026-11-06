"""Native UIKit stationary tab press versus the retained Demo composition.

Usage: compare-tab-press.py NATIVE_CAPTURE RENDERER OUTPUT [BEFORE_PIXELS]
       compare-tab-press.py NATIVE_CAPTURE RENDERER OUTPUT --append
Native capture uses tab-press-cases.json with capture-lens-motion.py. Append
accepts an independent capture session; it never fits or changes the renderer.
"""
from pathlib import Path
import gzip, hashlib, io, json, shutil, subprocess, sys
import numpy as np
from PIL import Image, ImageCms, ImageDraw

session=Path(sys.argv[1]);renderer=Path(sys.argv[2]).resolve();out=Path(sys.argv[3])
append='--append' in sys.argv[4:]
before=Path(sys.argv[4]) if len(sys.argv)>4 and not sys.argv[4].startswith('--') else None
contentReport='--content' in sys.argv[4:]
images=out/'images';raw=out/'native';images.mkdir(parents=True,exist_ok=True);raw.mkdir(exist_ok=True)
records=json.loads((out/'metrics.json').read_text())['records'] if append else []
manifest=json.loads((session/'manifest.json').read_text())
assert manifest['testExitCode']==0
profile=ImageCms.ImageCmsProfile(ImageCms.createProfile('sRGB')).tobytes()
sha=lambda path:hashlib.sha256(path.read_bytes()).hexdigest()
def rgb(path):
    image=Image.open(path);icc=image.info.get('icc_profile');image=image.convert('RGB')
    return ImageCms.profileToProfile(image,ImageCms.ImageCmsProfile(io.BytesIO(icc)),ImageCms.createProfile('sRGB'),outputMode='RGB') if icc else image
for case in manifest['cases']:
    name=case['id'];source=session/('motion-'+name);trace=json.loads((source/'trace.json').read_text())
    columns={key:i for i,key in enumerate(trace['columns'])}
    platter=next(e['id'] for e in trace['schema'] if e['viewClass']=='UIKit._UITabBarItemPlatterView')
    lens=next(e['id'] for e in trace['schema'] if e['viewClass']=='_UILiquidLensView')
    snapshot=next(s for s in trace['snapshots'] if s['name']=='contact-1')
    assert snapshot['touchActive'] and snapshot['success'] and snapshot['startedT']<1
    held=min(trace['frames'],key=lambda f:abs(f['t']-snapshot['startedT']))
    def row(frame,identifier):return next(r for r in frame['layers'] if r[0]==identifier)
    def model(frame,identifier):
        r=row(frame,identifier);return [r[columns['modelBoundsWidth']],r[columns['modelBoundsHeight']]]
    heldPlatter=row(held,platter)
    nativeScale=[heldPlatter[columns['presentationM11']],heldPlatter[columns['presentationM22']]]
    stageLeft=-trace['stageInWindow'][0]
    prefix=out/name
    subprocess.run([str(renderer),str(prefix),str(int(case['dark'])),*map(str,case['backgroundRGB'])],check=True)
    record=dict(id=name,validation=append,request=case,nativeLiftedBounds=model(held,lens),nativeRestingBounds=model(trace['frames'][0],lens),nativePlatterScale=nativeScale,os=trace['os'],phases={})
    record['beforeAvailable']=bool(before)
    for phase,nativeName in [('rest','rest'),('held','contact-1')]:
        nativeImage=rgb(source/(nativeName+'.png')).crop((round(stageLeft*2),200,round((stageLeft+402)*2),400))
        pixels=Path(str(prefix)+'-'+phase+'.ppm');nx=rgb(pixels).crop((0,200,804,400))
        metadata=Path(str(prefix)+'-'+phase+'.json');nxGeometry=json.loads(metadata.read_text())
        pixels.unlink();metadata.unlink()
        nativeImage.save(images/(name+'-'+phase+'-native.png'),icc_profile=profile);nx.save(images/(name+'-'+phase+'-nx.png'),icc_profile=profile)
        Image.blend(nativeImage,nx,.5).save(images/(name+'-'+phase+'-overlay.png'),icc_profile=profile)
        a=np.array(nativeImage,dtype=np.int16);b=np.array(nx,dtype=np.int16);error=np.abs(a-b)
        Image.fromarray(error.astype('uint8')).save(images/(name+'-'+phase+'-difference.png'))
        mask=error.max(axis=2)<=2
        Image.fromarray(np.concatenate([b.astype('uint8'),np.where(mask,0,255).astype('uint8')[...,None]],axis=2)).save(images/(name+'-'+phase+'-residual.png'))
        # A fixed blank patch between the first two item centers, away from
        # symbols, captions, the selection and the refractive perimeter.
        patch=(slice(98,103),slice(220,225))
        probeNative=np.median(a[patch],axis=(0,1));probeNX=np.median(b[patch],axis=(0,1))
        scores=dict(channelMAE=float(error.mean()),probeNative=probeNative.tolist(),probeNX=probeNX.tolist(),probeMaxChannelError=float(np.abs(probeNative-probeNX).max()),nxGeometry=nxGeometry)
        if before:
            oldPath=before/(name+'-'+phase+'.ppm')
            if not oldPath.exists():oldPath=before/(name+'-'+phase+'-nx.png')
            old=rgb(oldPath)
            if old.size!=(804,200):old=old.crop((0,200,804,400))
            old.save(images/(name+'-'+phase+'-before.png'),icc_profile=profile)
            oldArray=np.array(old,dtype=np.int16);oldProbe=np.median(oldArray[patch],axis=(0,1))
            scores.update(beforeMAE=float(np.abs(a-oldArray).mean()),probeBefore=oldProbe.tolist(),probeBeforeMaxChannelError=float(np.abs(probeNative-oldProbe).max()))
        record['phases'][phase]=scores
        shutil.copy2(source/(nativeName+'.png'),raw/(name+'-'+phase+'.png'))
    nativeDelta=np.array(record['phases']['held']['probeNative'])-record['phases']['rest']['probeNative']
    nxDelta=np.array(record['phases']['held']['probeNX'])-record['phases']['rest']['probeNX']
    record['pressDeltaMaxChannelError']=float(np.abs(nativeDelta-nxDelta).max())
    record['lensBoundsMaxError']=float(np.abs(np.array(record['nativeLiftedBounds'])-record['phases']['held']['nxGeometry']['lens'][2:]).max())
    record['platterScaleMaxError']=float(np.abs(np.array(nativeScale)-record['phases']['held']['nxGeometry']['platterScale']).max())
    nativeSymbols=[v['frame'] for v in trace['hierarchy'] if v['class']=='UIImageView' and 10<v['frame'][2]<50 and 10<v['frame'][3]<50 and 120<=v['frame'][1]<160][:4]
    if len(nativeSymbols)==4 and len(record['phases']['rest']['nxGeometry'].get('symbols',[]))==4:
        record['nativeSymbolFrames']=[[r[0]-stageLeft,*r[1:]] for r in nativeSymbols]
        record['symbolFrameMaxCoordinateError']=float(np.abs(np.array(record['nativeSymbolFrames'])-record['phases']['rest']['nxGeometry']['symbols']).max())
    nativeCaptions=[v['frame'] for v in trace['hierarchy'] if 'Label' in v['class'] and 150<=v['frame'][1]<180][:8]
    if len(nativeCaptions)==8 and len(record['phases']['rest']['nxGeometry'].get('captions',[]))==4:
        record['nativeCaptionFrames']=[[r[0]-stageLeft,*r[1:]] for r in nativeCaptions[:4]]
        record['nativeSelectedCaptionFrames']=[[r[0]-stageLeft,*r[1:]] for r in nativeCaptions[4:]]
        record['captionFrameMaxCoordinateError']=float(np.abs(np.array(record['nativeCaptionFrames'])-[r['frame'] for r in record['phases']['rest']['nxGeometry']['captions']]).max())
    with gzip.open(raw/(name+'-trace.json.gz'),'wb') as f:f.write((source/'trace.json').read_bytes())
    records=[r for r in records if r['id']!=name]+[record]
    print(name,'held MAE',round(record['phases']['held']['channelMAE'],3),'probe error',record['phases']['held']['probeMaxChannelError'],'press delta error',record['pressDeltaMaxChannelError'],'lens size error',record['lensBoundsMaxError'],flush=True)

method='Actual XCTest touch-down/hold captures on native UITabBar versus a stationary routed touch on the production Demo composition. Stage crop is derived from native stage/window geometry; no fitted image translations, scaling, colors or masks. Both sides render at2x and are color-managed to sRGB. Full-crop scores include symbol/font, material, selection tint, shadow and optics residuals. The blank fixed probe isolates platter press color; delta errors remove resting material bias. Validation colors are captured independently after choosing the response.'
(out/'metrics.json').write_text(json.dumps(dict(method=method,records=records),indent=2)+'\n')
repo=Path(__file__).resolve().parents[2]
sources=['Demo/GlassComparison/LensReference.m','Demo/GlassComparison/LensMotionUITests.m','Demo/GlassComparison/TabPressRenderer.cpp','Demo/GlassComparison/compare-tab-press.py','Demo/app/Screens/GlassDemoViewController/LensTabBarView.cpp','Demo/app/Screens/GlassDemoViewController/DemoSystemSymbol.hpp','UIKit/lib/LensMaterial.cpp','UIKit/lib/GlassShader.cpp','UIKit/lib/CABackdropEffectLayer.cpp','UIKit/lib/CAGlassContainerLayer.cpp','UIKit/lib/GlassRim.hpp','UIKit/lib/GlassInteraction.hpp','UIKitCore/lib/platforms/apple/AppleSystemImage.mm','UIKitCore/lib/CALayer.cpp','UIKitCore/lib/NXAffineTransform.cpp','UIKit/lib/BackdropEffectView.cpp','UIKitCore/lib/UILabel.cpp','Demo/app/Screens/GlassDemoViewController/GlassScreenTypography.hpp','Demo/GlassComparison/TabCaptionReference.m']
provenance=json.loads((out/'provenance.json').read_text()) if append else dict(sessions=[])
provenance.update(rendererSHA256=sha(renderer),sourceSHA256={p:sha(repo/p) for p in sources})
provenance['sessions'].append(manifest)
provenance['captureSHA256']={p.name:sha(p) for p in raw.iterdir()}
(out/'provenance.json').write_text(json.dumps(provenance,indent=2)+'\n')
preview=Image.new('RGB',(1206,226*2),(25,25,28));draw=ImageDraw.Draw(preview)
for rowIndex,name in enumerate(['phone-dark-black','phone-dark-brown']):
    for column,(suffix,title) in enumerate([('native','UIKit'),('before','NXKit before'),('nx','NXKit now')]):
        draw.text((column*402+8,rowIndex*226+4),name+' / '+title,fill='white')
        for phaseIndex,phase in enumerate(['rest','held']):
            file=images/(name+'-'+phase+'-'+suffix+'.png')
            if file.exists():preview.paste(Image.open(file).convert('RGB').resize((402,100),Image.Resampling.LANCZOS),(column*402,rowIndex*226+24+phaseIndex*100))
preview.save(out/'overview.png')
page='''<!doctype html><html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Tab bar press · UIKit / NXKit</title><style>:root{color-scheme:dark}body{font:16px/1.5 system-ui;margin:24px;background:#191a1e;color:#eee;max-width:1250px}label{display:inline-block;margin:8px 20px 8px 0}select{font:inherit}#stage{max-width:804px;background:repeating-conic-gradient(#444 0 25%,#333 0 50%) 50%/16px 16px}img{width:100%;display:block}pre{white-space:pre-wrap;font-size:13px}a{color:#9cf}</style><h1>Tab bar press · UIKit / NXKit</h1><p>Real native UITabBar touch-down/hold and retained Demo touch routing. Resting and held frames share coordinates. The lens grows eight points per edge; platter inflation is independent. Long neutral glass brightens color instead of adding a white button overlay. Geometry and flat material probes are scored separately from full image differences. Captions now use measured 10pt semibold/medium styles and tight frames. <a href="caption-comparison.html">Inspect native 3× caption pixels and glyph measurements</a>.</p><label>Scene <select id="scene"></select></label><label>Phase <select id="phase"><option value="held">Held</option><option value="rest">Resting</option></select></label><label>View <select id="mode"><option value="overlay">Overlay50%</option><option value="native">UIKit</option><option value="nx">NXKit now</option><option value="before">NXKit before</option><option value="difference">Absolute difference</option><option value="residual">Remove matches within2/255</option></select></label><div id="stage"><img id="image" alt="Aligned tab bar press comparison"></div><pre id="score"></pre><p><a href="README.md">Method and limits</a> · <a href="metrics.json">Measurements</a></p><script>const data=__DATA__,scene=document.getElementById('scene'),phase=document.getElementById('phase'),mode=document.getElementById('mode');data.forEach(r=>scene.add(new Option(r.id+(r.validation?' (validation)':''),r.id)));function update(){const r=data.find(r=>r.id===scene.value);mode.querySelector('[value="before"]').disabled=!r.beforeAvailable;if(mode.value==='before'&&!r.beforeAvailable)mode.value='nx';document.getElementById('image').src='images/'+r.id+'-'+phase.value+'-'+mode.value+'.png';document.getElementById('score').textContent=JSON.stringify({nativeLiftedBounds:r.nativeLiftedBounds,lensBoundsMaxError:r.lensBoundsMaxError,platterScaleMaxError:r.platterScaleMaxError,pressDeltaMaxChannelError:r.pressDeltaMaxChannelError,...r.phases[phase.value]},null,2)}scene.onchange=phase.onchange=mode.onchange=update;update();</script></html>'''
(out/'comparison.html').write_text(page.replace('__DATA__',json.dumps(records).replace('<','\\u003c')))
if contentReport:
    page=(out/'comparison.html').read_text().replace('Tab bar press · UIKit / NXKit','Tab content · UIKit / NXKit')
    page=page.replace('The lens grows eight points per edge; platter inflation is independent.','Symbols now use native measured image bounds; bitmap layers use linear sampling during fractional placement and glass inflation.')
    page=page.replace('platterScaleMaxError:r.platterScaleMaxError,','platterScaleMaxError:r.platterScaleMaxError,symbolFrameMaxCoordinateError:r.symbolFrameMaxCoordinateError,captionFrameMaxCoordinateError:r.captionFrameMaxCoordinateError,nativeCaptionFrames:r.nativeCaptionFrames,nativeSelectedCaptionFrames:r.nativeSelectedCaptionFrames,nativeSymbolFrames:r.nativeSymbolFrames,')
    (out/'comparison.html').write_text(page)
    assert all(r.get('symbolFrameMaxCoordinateError',1)<.35 for r in records)
# These are independent native-output gates, not expected formulas reproduced
# from the shader. Allow the known resting color/optics approximation to remain.
assert all(r['lensBoundsMaxError']<.01 and r['platterScaleMaxError']<.001 for r in records)
assert all(r['pressDeltaMaxChannelError']<=15 for r in records)
