#!/usr/bin/env python3
"""Compare the shared full-screen Demo composition with the native SwiftUI List."""
from pathlib import Path
import hashlib,io,json,shutil,subprocess,sys
import numpy as np
from PIL import Image,ImageCms,ImageDraw
native=Path(sys.argv[1]);renderer=Path(sys.argv[2]).resolve();out=Path(sys.argv[3]);out.mkdir(parents=True,exist_ok=True)
images=out/'images';images.mkdir(exist_ok=True);raw=out/'native';raw.mkdir(exist_ok=True)
cases=json.loads((native/'cases.json').read_text());records=[]
profile=ImageCms.ImageCmsProfile(ImageCms.createProfile('sRGB')).tobytes()
for case in cases:
 name=case['id'];meta=json.loads((native/(name+'.json')).read_text());w,h=meta['bounds'][2:];safe=meta['safe']
 source=Image.open(native/(name+'.png'));icc=source.info.get('icc_profile');source=source.convert('RGB')
 if icc:source=ImageCms.profileToProfile(source,ImageCms.ImageCmsProfile(io.BytesIO(icc)),ImageCms.createProfile('sRGB'),outputMode='RGB')
 source=source.resize((round(w*2),round(h*2)),Image.Resampling.LANCZOS)
 path=images/(name+'.ppm')
 args=[str(renderer),str(path),str(w),str(h),*[str(v) for v in safe],str(int(case['dark'])),str(case['tab']),str(case['accent']),str(case['backdrop']),str(int(case['details'])),str(case['added']),str(int(case['reversed'])),str(case['scroll'])]
 subprocess.run(args,check=True)
 nx=Image.open(path).convert('RGB');geometry=json.loads(Path(str(path)+'.json').read_text());path.unlink();Path(str(path)+'.json').unlink()
 a=np.array(source,dtype=np.int16);b=np.array(nx,dtype=np.int16);error=np.abs(a-b);exact=error.max(axis=2)==0;within=error.max(axis=2)<=2
 source.save(images/(name+'-native.png'),icc_profile=profile);nx.save(images/(name+'-nx.png'),icc_profile=profile)
 Image.blend(source,nx,.5).save(images/(name+'-overlay.png'),icc_profile=profile)
 Image.fromarray(error.astype('uint8')).save(images/(name+'-difference.png'),icc_profile=profile)
 for suffix,mask in [('exact',exact),('within2',within)]:
  residual=np.concatenate([b.astype('uint8'),np.where(mask,0,255).astype('uint8')[...,None]],axis=2);Image.fromarray(residual).save(images/(name+'-'+suffix+'.png'))
 platter=next(p['frame'] for p in meta['barParts'] if p['class']=='UIKit._UITabBarItemPlatterView');lens=next(p['frame'] for p in meta['barParts'] if p['class']=='_UILiquidLensView')
 record={'id':name,'request':case,'native':meta,'nx':geometry,'channelMAE':float(error.mean()),'exactPixelFraction':float(exact.mean()),'platterMaxCoordinateErrorPoints':float(np.max(np.abs(np.array(platter)-geometry['tab']))),'lensMaxCoordinateErrorPoints':float(np.max(np.abs(np.array(lens)-geometry['lens'])))}
 if case['scroll']==0:
  rows=sorted((r for r in meta['cells'] if abs(r[3]-52)<.01),key=lambda r:r[1])[:5]
  record['firstFiveRowsMaxCoordinateErrorPoints']=float(np.max(np.abs(np.array(rows)-geometry['rows'])))
 records.append(record)
 for suffix in ['.json','.png']:shutil.copy2(native/(name+suffix),raw/(name+suffix))
 print(name,'MAE',round(record['channelMAE'],3),'platter/lens',record['platterMaxCoordinateErrorPoints'],record['lensMaxCoordinateErrorPoints'],flush=True)
metrics={'method':'Native 3x window captures color-managed to sRGB then normalized to2x using Lanczos; NX retained raster at2x. No fitted translations/colors/warps. Geometry is measured independently in points. Pixel scores include different platform symbol metrics, text, scroll-edge and material approximations; not isolated glass scores.','records':records}
(out/'metrics.json').write_text(json.dumps(metrics,indent=2)+'\n')
repo=Path(__file__).resolve().parents[2]
files=['Demo/GlassComparison/GlassScreenReference.swift','Demo/app/Screens/GlassDemoViewController/GlassScreenView.cpp','Demo/app/Screens/GlassDemoViewController/GlassScreenTypography.hpp','Demo/app/Screens/GlassDemoViewController/GlassScreenViewController.cpp','Demo/app/Screens/GlassDemoViewController/LensTabBarView.cpp','NXKit/lib/NXNavigationController.cpp','NXKit/lib/NXNavigationItem.cpp','Demo/tests/GlassScreenTests.cpp','UIKitCore/lib/UILabel.cpp','UIKitCore/lib/FontResolution.cpp','UIKitCore/lib/FontTracking.hpp','UIKitCore/lib/TextRendering.hpp','UIKitCore/lib/platforms/apple/AppleSystemFont.mm','UIKitCore/lib/platforms/apple/AppleSystemImage.mm','UIKitCore/lib/UIImage.cpp','Demo/app/Screens/GlassDemoViewController/DemoSystemSymbol.hpp','UIKit/lib/LensMaterial.cpp','UIKit/lib/GlassShader.cpp','UIKit/lib/GlassInteraction.hpp','UIKitCore/lib/CALayer.cpp','UIKitCore/lib/NXAffineTransform.cpp','UIKit/lib/BackdropEffectView.cpp']
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
(out/'provenance.json').write_text(json.dumps({'rendererSHA256':sha(renderer),'sourceSHA256':{f:sha(repo/f) for f in files},'nativeBuild':json.loads((native.parent/'build.json').read_text()),'captureHashes':{p.name:sha(p) for p in sorted(raw.glob('*'))}},indent=2)+'\n')
preview=Image.new('RGB',(402*3,894),(24,24,24));draw=ImageDraw.Draw(preview)
for col,(suffix,title) in enumerate([('native','UIKit / SwiftUI'),('nx','NXKit Demo'),('difference','Absolute difference')]):
 draw.text((col*402+8,4),title,fill='white');im=Image.open(images/('initial-light-'+suffix+'.png')).resize((402,874),Image.Resampling.LANCZOS);preview.paste(im,(col*402,20))
preview.save(out/'overview.png')
page='''<!doctype html><html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Full-screen glass · UIKit / NXKit</title><style>:root{color-scheme:dark}body{font:16px/1.5 system-ui;margin:24px;max-width:1300px;background:#191a1e;color:#eee}h1{font-size:27px}p{max-width:1000px;color:#bec6d1}label{display:inline-block;margin:8px 18px 8px 0}select{font:inherit}#stage{width:402px;max-width:100%;background:repeating-conic-gradient(#444 0 25%,#333 0 50%) 50%/16px 16px}img{width:100%;display:block}a{color:#9cf}pre{white-space:pre-wrap;font-size:13px}</style><h1>Full-screen glass · UIKit / NXKit</h1><p>The same six states use a real SwiftUI inset-grouped List and native bars, versus the live NXKit Demo composition. Both use the measured iPhone safe areas. Native 3× pixels are normalized to 2×; no translation or color fitting is applied. Native SF Symbols are used on Apple platforms; symbol metrics, scroll-edge blur and glass optics still differ.</p><label>Scene <select id="scene" aria-label="Scene"></select></label><label>View <select id="mode" aria-label="View"><option value="overlay">Overlay 50%</option><option value="native">UIKit / SwiftUI</option><option value="nx">NXKit Demo</option><option value="difference">Absolute difference</option><option value="exact">Remove exact matches</option><option value="within2">Remove matches within2/255</option></select></label><div id="stage"><img id="image" alt="Aligned full-screen comparison"></div><pre id="score"></pre><p><a href="README.md">Method, actions and limits</a> · <a href="metrics.json">Full measurements</a> · <a href="provenance.json">Build provenance</a></p><script>const records=__DATA__,scene=document.getElementById('scene'),mode=document.getElementById('mode'),image=document.getElementById('image');records.forEach(r=>scene.add(new Option(r.id,r.id)));function update(){const r=records.find(r=>r.id===scene.value);image.src='images/'+r.id+'-'+mode.value+'.png';document.getElementById('score').textContent=JSON.stringify({channelMAE:r.channelMAE,platterMaxCoordinateErrorPoints:r.platterMaxCoordinateErrorPoints,lensMaxCoordinateErrorPoints:r.lensMaxCoordinateErrorPoints,firstFiveRowsMaxCoordinateErrorPoints:r.firstFiveRowsMaxCoordinateErrorPoints},null,2)}scene.onchange=mode.onchange=update;update();</script></html>'''
(out/'comparison.html').write_text(page.replace('__DATA__',json.dumps(records).replace('<','\\u003c')))
