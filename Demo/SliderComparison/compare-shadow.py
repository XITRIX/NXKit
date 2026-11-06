#!/usr/bin/env python3
"""Unscaled 3x detail comparison of native / before / corrected lens shadows."""
import argparse,gzip,json
from pathlib import Path
from PIL import Image,ImageDraw
p=argparse.ArgumentParser();p.add_argument('before',type=Path);p.add_argument('after',type=Path);a=p.parse_args()
base=Path(__file__).resolve().parents[2]/'docs/slider/lift-motion'
manifest=json.loads((base/'video/manifest.json').read_text())
out=base/'comparison';out.mkdir(exist_ok=True)
rows=[];sources=[]
for theme,phase,t in [('light','Press',.35),('light','Release',.25),('dark','Release',.25)]:
    case=next(c for r in manifest['recordings'] for c in r['cases'] if c['id']=='stationary-'+theme)
    trace=json.loads(gzip.decompress((base/'native/stationary'/('stationary-'+theme)/'trace.json.gz').read_bytes()))
    begin=next(e['t'] for e in trace['actions'] if e.get('method')=='beginTracking')
    end=trace['touches'][-1]['dispatchT']
    nativeTime=(begin+(.0184 if theme=='light' else .0148) if phase=='Press' else end+.0165)+t
    ourTime=(.05 if phase=='Press' else .75)+t
    native=min(case['frames'],key=lambda f:abs(f['traceTime']-nativeTime))
    images=[Image.open(base/'video'/native['image']).convert('RGB')]
    files=[]
    for folder in [a.before,a.after]:
        folder=folder/(theme+'-hold');meta=json.loads((folder/'metadata.json').read_text())
        assert all(meta[k]==3 for k in ['displayScale','lensContentsScale','opticalContentsScale','warpContentsScale'])
        file=min(folder.glob('*.ppm'),key=lambda f:abs(int(f.stem)/1000-ourTime))
        images.append(Image.open(file).convert('RGB'));files.append(file)
    row=Image.new('RGB',(900,260),'#dddddd');draw=ImageDraw.Draw(row)
    for i,(label,im) in enumerate(zip(['UIKit (3x)','Before (3x)','After (3x)'],images)):
        assert im.size==(1020,330)
        crop=im.crop((165,60,465,270));row.paste(crop,(i*300,45))
        draw.text((i*300+8,7),label,fill='black');draw.text((i*300+8,24),f'{theme} / {phase} {1000*t:.0f} ms',fill='black')
        crop.save(out/f'shadow-{theme}-{phase.lower()}-{i}.png')
    rows.append(row)
    sources.append({'theme':theme,'phase':phase,'relativeTime':t,'native':native,'beforeTime':int(files[0].stem)/1000,'afterTime':int(files[1].stem)/1000})
result=Image.new('RGB',(900,len(rows)*260),'#ddd')
for i,row in enumerate(rows):result.paste(row,(0,i*260))
result.save(out/'shadow-detail.png')
(out/'shadow-detail.json').write_text(json.dumps({'note':'All crops retain original 3x pixels without resampling; native source is compressed video with up to one frame of phase uncertainty. Before is the corrected 3x capture with the former material/shadow behavior, separate from the historical 1x geometry baseline.','sources':sources},indent=2)+'\n')
print(out/'shadow-detail.png')
