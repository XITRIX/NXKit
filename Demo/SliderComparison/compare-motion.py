#!/usr/bin/env python3
"""Compare recorded native presentation curves and phase-aligned motion crops."""
import argparse,csv,gzip,json,math,shutil
from pathlib import Path
from PIL import Image,ImageDraw
p=argparse.ArgumentParser();p.add_argument('before',type=Path);p.add_argument('after',type=Path);a=p.parse_args()
root=Path(__file__).resolve().parents[2];base=root/'docs/slider/lift-motion';out=base/'comparison';out.mkdir(exist_ok=True)
manifest=json.loads((base/'video/manifest.json').read_text());metrics={}
def rows(path):return list(csv.DictReader(path.open()))
def at(data,t,key):
    if t<=float(data[0]['t']):return float(data[0][key])
    for x,y in zip(data,data[1:]):
        lo,hi=float(x['t']),float(y['t'])
        if lo<=t<=hi:return float(x[key])+(float(y[key])-float(x[key]))*(t-lo)/(hi-lo)
    return float(data[-1][key])
def our_image(folder,t):
    file=min(folder.glob('*.ppm'),key=lambda f:abs(int(f.stem)/1000-t));return Image.open(file).convert('RGB')
for dark in [False,True]:
    theme='dark' if dark else 'light';case=next(c for rec in manifest['recordings']for c in rec['cases']if c['id']=='stationary-'+theme)
    trace=json.loads(gzip.decompress((base/'native/stationary'/('stationary-'+theme)/'trace.json.gz').read_bytes()));curve=rows(base/'native/stationary'/('stationary-'+theme)/'analysis/curve.csv')
    begin=next(e['t'] for e in trace['actions']if e.get('method')=='beginTracking');end=trace['touches'][-1]['dispatchT'];delay=.0184 if not dark else .0148
    old=rows(a.before/(theme+'-hold')/'frames.csv');new=rows(a.after/(theme+'-hold')/'frames.csv')
    capture=json.loads((a.after/(theme+'-hold')/'metadata.json').read_text())
    assert all(capture[k]==3 for k in ['displayScale','lensContentsScale','opticalContentsScale','warpContentsScale']), 'Optics must actually render at 3x; output dimensions alone are insufficient'
    assert capture['dark']==dark and capture['pixelWidth']==1020 and manifest['scale']==3
    errors={}
    for phase,origin,oursOrigin,duration in [('lift',begin+delay,.05,.35),('rest',end+.0165,.75,.55)]:
        values=[]
        for row in curve:
            t=float(row['t'])-origin
            if 0<=t<=duration:
                values.append((t,float(row['width']),max(0,min(1,float(row['whiteOpacity'])))))
        stats={}
        for label,data in [('before',old),('after',new)]:
            ws=[at(data,oursOrigin+t,'presentW')-w for t,w,alpha in values];al=[at(data,oursOrigin+t,'restingAlpha')-alpha for t,w,alpha in values]
            stats[label]={'widthRMSEPoints':math.sqrt(sum(x*x for x in ws)/len(ws)),'widthMaximumPoints':max(abs(x)for x in ws),'whiteRMSE':math.sqrt(sum(x*x for x in al)/len(al))}
        errors[phase]=stats
    metrics[theme]=errors
    for label,folder in [('before',a.before),('after',a.after)]:
        for kind in ['hold','tap']:
            target=out/label/(theme+'-'+kind);target.mkdir(parents=True,exist_ok=True);shutil.copy2(folder/(theme+'-'+kind)/'frames.csv',target/'frames.csv')
            if label=='after':shutil.copy2(folder/(theme+'-'+kind)/'metadata.json',target/'metadata.json')
    images=[];sheet=[]
    for phase,origin,oursOrigin,duration in [('Press',begin+delay,.05,.35),('Release',end+.0165,.75,.55)]:
        for frame in range(int(duration*60)+1):
            t=frame/60
            native=min(case['frames'],key=lambda r:abs(r['traceTime']-(origin+t)))
            pictures=[Image.open(base/'video'/native['image']).convert('RGB'),our_image(a.after/(theme+'-hold'),oursOrigin+t)]
            result=Image.new('RGB',(600,260),'#dddddd');draw=ImageDraw.Draw(result)
            for i,(name,pic)in enumerate(zip(['UIKit (3x)','NXKit Raster (3x)'],pictures)):
                result.paste(pic.crop((165,60,465,270)),(i*300,45));draw.text((i*300+8,7),name,fill='black');draw.text((i*300+8,24),f'{phase} {t*1000:.0f} ms',fill='black')
            images.append(result)
            if frame in [0,3,6,9,15,21]:sheet.append(result)
    images[0].save(out/(theme+'.gif'),save_all=True,append_images=images[1:],duration=33,loop=0)
    contact=Image.new('RGB',(600,260*len(sheet)),'#dddddd')
    for i,pic in enumerate(sheet):contact.paste(pic,(0,260*i))
    contact.save(out/(theme+'-frames.png'))
(out/'metrics.json').write_text(json.dumps({'note':'Visual crops compare native 3x with asserted 3x Raster optics. The previous 1x visual baseline is omitted; its scale-independent geometry/opacity CSV remains for before/after curve metrics. Visual crops are phase-aligned by measured commit offset; native video marker phase has up to one compositor frame uncertainty. Capture requested 120 Hz but native records are about 60 Hz. Curve metric uses captured timestamps, no synthetic frame rate. Dark first-frame scheduling outlier is retained. After renderer samples its explicit driver at 120 Hz.','values':metrics},indent=2)+'\n')
print(json.dumps(metrics,indent=2))
