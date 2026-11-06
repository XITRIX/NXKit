"""Compare native caption styles/geometry and native-scale screen pixels.

Usage: verify-tab-captions.py RASTER_BUILD OUTPUT
OUTPUT/native/caption-fonts.json comes from TabCaptionReference.m via simctl
spawn. Screen pixels reuse the retained native 3x full-screen captures.
"""
from pathlib import Path
import hashlib, io, json, subprocess, sys
import numpy as np
from PIL import Image, ImageCms

build=Path(sys.argv[1]).resolve();out=Path(sys.argv[2]).resolve()
repo=Path(__file__).resolve().parents[2]
fonts=json.loads((out/'native/caption-fonts.json').read_text())
assert len(fonts['labels'])==8 and fonts['scale']==3
assert all(r['points']==10 for r in fonts['labels'])
assert all(r['font']=='.SFUI-Semibold' for r in fonts['labels'][:4])
assert all(r['font']=='.SFUI-Medium' for r in fonts['labels'][4:])
shape=subprocess.check_output([str(build/'TextRendererTests'),'--tab-shaping','3','0',str(repo/'Submodules/skia/resources/fonts/Roboto-Regular.ttf')],text=True).splitlines()
shapeErrors=[]
for i,label in enumerate(fonts['labels']):
    header=shape[i*2].split();positions=shape[i*2+1].split()
    assert header[0]==label['text'] and int(header[1])==(600 if i<4 else 500)
    assert len(positions)==len(label['glyphs'])
    for item,(glyph,x) in zip(positions,label['glyphs']):
        actualGlyph,actualX=item.split(':');assert int(actualGlyph)==glyph
        shapeErrors.append(abs(float(actualX)-x))
assert max(shapeErrors)<.0001
records=[]
screen=repo/'docs/liquid-glass-lens/screen'
cases=json.loads((screen/'metrics.json').read_text())['records']
images=out/'caption-images';images.mkdir(exist_ok=True)
for dark in (False,True):
    theme='dark' if dark else 'light';prefix=out/('caption-3x-'+theme)
    subprocess.run([str(build/'TabPressRenderer'),str(prefix),str(int(dark)),*(['0','0','0'] if dark else ['242','242','247']),'3'],check=True)
    metadata=json.loads(Path(str(prefix)+'-rest.json').read_text())
    actual=metadata['captions']+metadata['selectedCaptions']
    frameErrors=[]
    for reference,nx in zip(fonts['labels'],actual):
        frame=list(nx['frame']);frame[1]-=120
        assert nx['points']==reference['points']
        frameErrors.append(float(np.max(np.abs(np.array(frame)-reference['frame']))))
    assert max(frameErrors)<.001
    case=next(r for r in cases if r['id']=='initial-'+theme)
    request=case['request'];native=case['native'];w,h=native['bounds'][2:]
    ppm=out/('caption-screen-'+theme+'.ppm')
    subprocess.run([str(build/'GlassScreenTests'),str(ppm),str(w),str(h),*map(str,native['safe']),str(int(dark)),str(request['tab']),str(request['accent']),str(request['backdrop']),str(int(request['details'])),str(request['added']),str(int(request['reversed'])),str(request['scroll']),'3'],check=True)
    reference=Image.open(screen/'native'/('initial-'+theme+'.png'))
    icc=reference.info.get('icc_profile');reference=reference.convert('RGB')
    if icc:reference=ImageCms.profileToProfile(reference,ImageCms.ImageCmsProfile(io.BytesIO(icc)),ImageCms.createProfile('sRGB'),outputMode='RGB')
    current=Image.open(ppm).convert('RGB');assert reference.size==current.size
    # Identical absolute window coordinates. No translated/aligned text crops.
    crop=(63,2490,1143,2532)
    a=reference.crop(crop);b=current.crop(crop)
    a.save(images/(theme+'-native.png'));b.save(images/(theme+'-nx.png'))
    Image.blend(a,b,.5).save(images/(theme+'-overlay.png'))
    error=np.abs(np.array(a,dtype=np.int16)-np.array(b,dtype=np.int16))
    Image.fromarray(error.astype('uint8')).save(images/(theme+'-difference.png'))
    pixels=np.array(b);alpha=np.where(error.max(2)<=2,0,255).astype('uint8')
    Image.fromarray(np.dstack((pixels,alpha))).save(images/(theme+'-residual.png'))
    records.append(dict(theme=theme,captionFrameMaxCoordinateError=max(frameErrors),glyphPositionMaxError=max(shapeErrors),captionFrames=actual,pixelCrop=crop,channelMAE=float(error.mean())))
    if dark:
        Image.open(Path(str(prefix)+'-rest.ppm')).crop((0,360,1206,546)).save(out/'rest-3x.png')
        (out/'rest-3x.json').write_text(Path(str(prefix)+'-rest.json').read_text())
    for phase in ('rest','held'):
        Path(str(prefix)+'-'+phase+'.ppm').unlink();Path(str(prefix)+'-'+phase+'.json').unlink()
    ppm.unlink();Path(str(ppm)+'.json').unlink()
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
sources=['Demo/GlassComparison/TabCaptionReference.m','Demo/GlassComparison/verify-tab-captions.py','Demo/GlassComparison/TabPressRenderer.cpp','Demo/app/Screens/GlassDemoViewController/GlassScreenTypography.hpp','Demo/app/Screens/GlassDemoViewController/LensTabBarView.cpp','UIKit/lib/LensMaterial.cpp','UIKitCore/lib/UILabel.cpp','UIKitCore/lib/TextRendering.hpp','UIKitCore/tests/TextRenderingTests.cpp','Demo/tests/GlassScreenTests.cpp']
result=dict(method='Public native UITabBar font probe; Core Text glyph positions versus actual SkParagraph glyph positions. Frame gates at native 3x in both themes. Pixel crops use identical absolute window coordinates from retained native full-screen 3x captures and new retained Demo renders. No image fitting, resampling, shifts or font substitutions. Raster scores include glass and glyph-edge differences and are not the geometry gate.',records=records,sourceSHA256={p:sha(repo/p) for p in sources},rendererSHA256={n:sha(build/n) for n in ['TabPressRenderer','GlassScreenTests','TextRendererTests']},nativeFontsSHA256=sha(out/'native/caption-fonts.json'),nativeScreensSHA256={t:sha(screen/'native'/('initial-'+t+'.png')) for t in ['light','dark']})
(out/'caption-metrics.json').write_text(json.dumps(result,indent=2)+'\n')
page='''<!doctype html><html lang="en"><meta charset="utf-8"><title>Tab captions · native 3×</title><style>body{font:16px/1.5 system-ui;color:#eee;background:#191a1e;margin:24px}label{margin-right:20px}img{display:block;margin-top:20px;width:1080px;max-width:100%}pre{white-space:pre-wrap}a{color:#9cf}</style><h1>Tab captions · native 3×</h1><p>Actual UIKit: 10pt semibold normal captions, 10pt medium selected ink. Tight frames and Core Text glyph positions match NXKit. Original full-screen native pixels and current Demo pixels share absolute coordinates. Remaining edge/color residuals are visible; geometry equality does not mean identical rasterization or glass.</p><label>Theme <select id="theme"><option>dark</option><option>light</option></select></label><label>View <select id="mode"><option value="overlay">50% overlay</option><option value="native">UIKit</option><option value="nx">NXKit</option><option value="difference">Absolute difference</option><option value="residual">Remove matches within 2/255</option></select></label><img id="image" alt="Native-scale tab captions"><pre id="score"></pre><a href="comparison.html">Full tab comparison</a><script>const data=__DATA__,theme=document.getElementById('theme'),mode=document.getElementById('mode');function update(){document.getElementById('image').src='caption-images/'+theme.value+'-'+mode.value+'.png';document.getElementById('score').textContent=JSON.stringify(data.find(r=>r.theme===theme.value),null,2)}theme.onchange=mode.onchange=update;update()</script></html>'''
(out/'caption-comparison.html').write_text(page.replace('__DATA__',json.dumps(records)))
print(json.dumps([{k:r[k] for k in ['theme','captionFrameMaxCoordinateError','glyphPositionMaxError','channelMAE']} for r in records],indent=2))
