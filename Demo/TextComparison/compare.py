#!/usr/bin/env python3
"""Aligned native UILabel vs retained NXKit, with identical-font controls."""
from pathlib import Path
import hashlib,io,json,shutil,subprocess,sys
import numpy as np
from PIL import Image,ImageCms,ImageDraw
native=Path(sys.argv[1]).resolve();before=Path(sys.argv[2]).resolve();renderer=Path(sys.argv[3]).resolve();out=Path(sys.argv[4]).resolve()
root=Path(__file__).resolve().parents[2];font=root/'Submodules/skia/resources/fonts/Roboto-Regular.ttf'
images=out/'images';raw=out/'native';images.mkdir(parents=True,exist_ok=True);raw.mkdir(exist_ok=True)
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
profile=ImageCms.ImageCmsProfile(ImageCms.createProfile('sRGB')).tobytes();records=[]
build=json.loads((native.parent/'build.json').read_text())
def rgb(path):
 im=Image.open(path);icc=im.info.get('icc_profile');im=im.convert('RGB')
 if icc:im=ImageCms.profileToProfile(im,ImageCms.ImageCmsProfile(io.BytesIO(icc)),ImageCms.createProfile('sRGB'),outputMode='RGB')
 return im
for scale in [1,2,3]:
 for dark in [False,True]:
  name=f'{scale}x-'+('dark' if dark else 'light');metadata=json.loads((native/(name+'.json')).read_text())
  assert metadata['sourceSHA256']==build['sourceSHA256'];assert metadata['fontSHA256']==sha(font)
  assert all(s['displayScale']==scale for s in metadata['samples'])
  path=images/(name+'.ppm');subprocess.run([str(renderer),str(path),str(scale),str(int(dark)),str(font)],check=True)
  current=rgb(path);nxmeta=json.loads(Path(str(path)+'.json').read_text());path.unlink();Path(str(path)+'.json').unlink()
  reference=rgb(native/(name+'.png'));old=rgb(before/(name+'.ppm'))
  a=np.array(reference,dtype=float);b=np.array(current,dtype=float);previous=np.array(old,dtype=float)
  for suffix,im in [('native',reference),('before',old),('current',current),('overlay',Image.blend(reference,current,.5))]:im.save(images/(name+'-'+suffix+'.png'),icc_profile=profile)
  diff=np.abs(a-b);Image.fromarray(diff.astype('uint8')).save(images/(name+'-difference.png'),icc_profile=profile)
  residual=np.concatenate([b.astype('uint8'),np.where(diff.max(2)==0,0,255).astype('uint8')[...,None]],2);Image.fromarray(residual).save(images/(name+'-exact.png'))
  for index,s in enumerate(metadata['samples']):
   _,y,_,h=s['frame'];start,end=round(y*scale),round((y+h)*scale)
   ink=lambda pixels: pixels[start:end].mean(2) if dark else 255-pixels[start:end].mean(2)
   n=ink(a);c=ink(b);p=ink(previous)
   cy=lambda d:float((d*np.arange(d.shape[0])[:,None]).sum()/max(1,d.sum())/scale)
   union=(n>1)|(c>1)|(p>1)
   metric=lambda d:{'inkRatio':float(d.sum()/max(1,n.sum())),'verticalCentroidErrorPoints':cy(d)-cy(n),'inkUnionMAE':float(np.abs(d-n)[union].mean()) if union.any() else 0}
   record={'scene':name,'id':s['id'],'sameFontFile':s['custom'],'native':s,'nx':nxmeta['samples'][index],'before':metric(p),'after':metric(c)}
   records.append(record)
  for suffix in ['.json','.png','-baseline.png']:shutil.copy2(native/(name+suffix),raw/(name+suffix))
  print(name,'captured',flush=True)
controls=[r for r in records if r['sameFontFile'] and r['id'] not in ['empty','multiline']]
summary={'sameFileSingleLineCases':len(controls),'beforeMeanAbsoluteInkRatioError':float(np.mean([abs(r['before']['inkRatio']-1) for r in controls])),'afterMeanAbsoluteInkRatioError':float(np.mean([abs(r['after']['inkRatio']-1) for r in controls])),'beforeMeanAbsoluteVerticalCentroidErrorPoints':float(np.mean([abs(r['before']['verticalCentroidErrorPoints']) for r in controls])),'afterMeanAbsoluteVerticalCentroidErrorPoints':float(np.mean([abs(r['after']['verticalCentroidErrorPoints']) for r in controls])),'afterWorstAbsoluteVerticalCentroidErrorPoints':max(abs(r['after']['verticalCentroidErrorPoints']) for r in controls)}
(out/'metrics.json').write_text(json.dumps({'method':'Untranslated point-aligned native UILabel and actual NXKit UILabel. All images color-managed to sRGB at their native 1x/2x/3x scale; no resampling, fitted shifts, font substitutions or weight fitting. Same-file rows use identical Roboto bytes; system rows compare native iOS UIFont against production Apple resolution using macOS NSFont. Ink coverage and centroid are foreground-weighted; empty rows are excluded from accuracy summaries.','summary':summary,'records':records},indent=2)+'\n')
files=['UIKitCore/lib/UILabel.cpp','UIKitCore/include/UILabel.h','UIKitCore/lib/CATextLayer.cpp','UIKitCore/lib/UITextView.cpp','UIKitCore/lib/FontResolution.cpp','UIKitCore/lib/FontTracking.hpp','UIKitCore/lib/TextRendering.hpp','UIKitCore/lib/platforms/apple/AppleSystemFont.mm','UIKitCore/lib/platforms/apple/AppleSystemFont.hpp','UIKitCore/lib/platforms/apple/ios/SkiaCtx_ios.mm','UIKitCore/lib/platforms/apple/macos/SkiaCtx_macos.mm','UIKitCore/tests/TextRenderingTests.cpp','Demo/TextComparison/Reference.swift','Demo/TextComparison/compare.py']
(out/'provenance.json').write_text(json.dumps({'sourceSHA256':{f:sha(root/f) for f in files},'rendererSHA256':sha(renderer),'beforeRendererSHA256':sha(before/'TextRendererTests'),'beforeLabelSourceSHA256':sha(before/'UILabel.cpp'),'nativeBuild':build,'captureSHA256':{p.name:sha(p) for p in sorted(raw.glob('*'))}},indent=2)+'\n')
preview=Image.new('RGB',(1600,612),(24,24,24));draw=ImageDraw.Draw(preview)
for col,(suffix,title) in enumerate([('native','UIKit'),('before','NXKit before'),('current','NXKit corrected'),('difference','Absolute difference')]):
 draw.text((col*400+8,4),title,fill='white');im=Image.open(images/('3x-light-'+suffix+'.png')).resize((400,592),Image.Resampling.LANCZOS);preview.paste(im,(col*400,20))
preview.save(out/'overview.png')
# A readable body-sized comparison for the final review.
strip=Image.new('RGB',(800,3*124),(24,24,24));draw=ImageDraw.Draw(strip)
for row,(suffix,title) in enumerate([('native','UIKit · same Roboto file, 17 pt regular'),('before','NXKit before'),('current','NXKit corrected')]):
 draw.text((8,row*124+4),title,fill='white');im=Image.open(images/('3x-light-'+suffix+'.png')).crop((0,104*3,400*3,156*3)).resize((800,104),Image.Resampling.LANCZOS);strip.paste(im,(0,row*124+20))
strip.save(out/'body-comparison.png')
page='''<!doctype html><html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Text rendering · UIKit / NXKit</title><style>:root{color-scheme:dark}body{font:16px/1.5 system-ui;background:#191a1e;color:#eee;margin:24px;max-width:1100px}h1{font-size:28px}p{max-width:960px;color:#c4ccd4}label{display:inline-block;margin:8px 20px 12px 0}select{font:inherit}#stage{width:800px;max-width:100%;background:repeating-conic-gradient(#555 0 25%,#333 0 50%) 50%/16px 16px}img{display:block;width:100%}pre{white-space:pre-wrap;font-size:13px}a{color:#9cf}</style><h1>Text rendering · UIKit / NXKit</h1><p>Native UILabel and the real NXKit renderer at matching 1×, 2× and 3× scales. The first five rows and multiline control use identical Roboto font bytes. Later rows exercise system regular, semibold, bold and Unicode fallback. No fitted translation or weight adjustment is applied. System fonts come from iOS and macOS respectively; remaining bold/emoji and multiline differences are explicit.</p><label>Scene <select id="scene" aria-label="Scene">__OPTIONS__</select></label><label>View <select id="mode" aria-label="View"><option value="overlay">Overlay 50%</option><option value="native">UIKit</option><option value="before">NXKit before</option><option value="current">NXKit corrected</option><option value="difference">Absolute difference</option><option value="exact">Remove exact matches</option></select></label><div id="stage"><img id="image" alt="Aligned text rendering comparison"></div><pre id="score"></pre><p><a href="README.md">Findings, method and limitations</a> · <a href="metrics.json">All 66 measurements</a> · <a href="provenance.json">Provenance</a> · <a href="../liquid-glass-lens/screen/comparison.html">Updated full-screen glass comparison</a></p><script>const summary=__SUMMARY__,records=__DATA__,scene=document.getElementById('scene'),mode=document.getElementById('mode');function update(){document.getElementById('image').src='images/'+scene.value+'-'+mode.value+'.png';const rows=records.filter(r=>r.scene===scene.value&&r.sameFontFile&&r.id!=='empty').map(r=>({id:r.id,inkRatio:r.after.inkRatio,verticalErrorPoints:r.after.verticalCentroidErrorPoints}));document.getElementById('score').textContent=JSON.stringify({summary,rows},null,2)}scene.value='3x-light';scene.onchange=mode.onchange=update;update();</script></html>'''
options=''.join(f'<option>{scale}x-{theme}</option>' for scale in [1,2,3] for theme in ['light','dark'])
(out/'comparison.html').write_text(page.replace('__OPTIONS__',options).replace('__SUMMARY__',json.dumps(summary)).replace('__DATA__',json.dumps(records).replace('<','\\u003c')))
# A focused acceptance check: same-file single-line weight/placement across all six scenes.
assert all(abs(r['after']['inkRatio']-1)<.002 for r in controls)
assert all(abs(r['after']['verticalCentroidErrorPoints'])<=.51 for r in controls)
assert summary['afterMeanAbsoluteVerticalCentroidErrorPoints']<summary['beforeMeanAbsoluteVerticalCentroidErrorPoints']
print(json.dumps(summary,indent=2))
