"""Compare native edge-pop captures and portable progress-matched replay.

Usage: compare-interactive.py NATIVE_DIRECTORY REPLAY_DIRECTORY OUTPUT_DIRECTORY
Requires Pillow. Native content presentation progress drives the portable pan;
this isolates navigation-bar poses and is not an input-latency comparison.
No fitted per-case time offsets, image alignment or color correction are used.
"""
import gzip,hashlib,json,math,statistics,sys
from pathlib import Path
from PIL import Image,ImageDraw
native,replay,out=map(Path,sys.argv[1:4]);out.mkdir(parents=True,exist_ok=True)
records=json.loads((replay/'replay.json').read_text());by={}
for r in records:by.setdefault(r['name'],[]).append(r)
all_errors=[];all_held=[];cases=[];raw={}
for name,rows in sorted(by.items()):
    reference=json.loads((native/name/'trace.json').read_text());raw[name]=reference
    errors=[];held=[];mismatches=0;matched=0
    for r in rows:
        if not r['active'] or r['progress']<=.03:continue
        n=reference['frames'][r['index']]
        ng=sorted([v['frame'] for v in n['views'] if 'UIPlatformGlassInteractionView' in v['class']],key=lambda f:f[0]);og=sorted(r['groups'],key=lambda f:f[0])
        if len(ng)!=len(og):mismatches+=1;continue
        d=[a-b for aa,bb in zip(ng,og) for a,b in zip(aa,bb)];errors.extend(d);matched+=1
        if abs(n['translation']-(n['sourceContent']or[0])[0])<1 and r['t']>.25:held.extend(d)
    rms=lambda values:math.sqrt(sum(x*x for x in values)/len(values)) if values else None
    all_errors+=errors;all_held+=held
    finish=name.endswith('finish');expected=2 if finish else 3
    native_count=reference['frames'][-1]['count']
    if rows[-1]['stack']!=expected:raise ValueError('Portable outcome did not match '+name)
    cases.append({'name':name,'nativeFrames':len(reference['frames']),'replayFrames':len(rows),'matchedTrackingFrames':matched,'trackingCountMismatches':mismatches,'trackingRectRmsPt':rms(errors),'heldRectRmsPt':rms(held),'maximumRectCoordinateErrorPt':max(map(abs,errors),default=None),'nativeFinalCount':native_count,'intendedFinalCount':expected,'outcomeAgrees':native_count==rows[-1]['stack'],'portableFinalCount':rows[-1]['stack']})
    # Preserve a compact, readable visual for every arrangement/outcome/theme.
    pictures=[(i,r) for i,r in enumerate(reference['frames']) if 'image'in r and (replay/name/f'{i:04d}.png').exists()]
    frames=[];durations=[]
    for j,(index,r) in enumerate(pictures):
        image=Image.new('RGB',(1072,132),'#eeeeee');draw=ImageDraw.Draw(image)
        draw.text((8,4),f'UIKit {name} t={r["t"]:.3f}s'+(' [unexpected native completion]' if native_count!=expected else ''),fill='black');draw.text((544,4),'Portable (matched content progress during drag)',fill='black')
        image.paste(Image.open(native/name/r['image']).convert('RGB').resize((536,107)),(0,24));image.paste(Image.open(replay/name/f'{index:04d}.png').convert('RGB').resize((536,107)),(536,24));frames.append(image)
        next_time=pictures[j+1][1]['t'] if j+1<len(pictures) else r['t']+.65;durations.append(max(20,round((next_time-r['t'])*1000)))
    if frames:frames[0].save(out/(name+'.gif'),save_all=True,append_images=frames[1:],duration=durations,loop=0)
    # Contact sheets keep precise native/portable frame associations reviewable.
    if pictures:
        sheet=Image.new('RGB',(1072,132*5),'#eeeeee')
        for j,t in enumerate([0,.3,.7,1.2,1.8]):
            closest=min(range(len(pictures)),key=lambda i:abs(pictures[i][1]['t']-t));sheet.paste(frames[closest],(0,j*132))
        sheet.save(out/(name+'.png'))
report={'method':__doc__,'cases':cases,'trackingRectRmsPt':rms(all_errors),'heldRectRmsPt':rms(all_held),'maximumRectCoordinateErrorPt':max(map(abs,all_errors)),'trackingScalarSamples':len(all_errors),'nativeFrames':sum(len(d['frames']) for d in raw.values()),'portableFrames':len(records),'notes':['Rectangles compare logical material carriers, including hidden-background slots.','Only tracking frames are scored. Release/return springs, tint and blur are reviewed in the paired clips.','Native capture timestamps are retained; screenshots occur every fourth display-link sample, not every 120 Hz sample.','All cases, including the delayed 6-5 dark cancellation sample, are retained.','The portable renderer draws the full window into a navigation-bar crop.']}
(out/'comparison.json').write_text(json.dumps(report,indent=2)+'\n')
for name,data in [('native-traces.json',raw),('portable-traces.json',records)]:
    encoded=json.dumps(data,separators=(',',':')).encode();(out/(name+'.gz')).write_bytes(gzip.compress(encoded,mtime=0))
print(json.dumps({key:report[key] for key in ['nativeFrames','portableFrames','trackingRectRmsPt','heldRectRmsPt','maximumRectCoordinateErrorPt']},indent=2))
