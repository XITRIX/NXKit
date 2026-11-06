"""Compare optical checkpoints with injected native presentation rectangles.

Usage: python compare-merge-checkpoints.py CAPTURE_ROOT OUTPUT_ROOT [BEFORE_RENDERER AFTER_RENDERER]
Uses unchanged native screenshots/traces. Motion and pixel timing parity are
not inferred: nearest sampled presentation frames and lighting assumptions are
recorded explicitly. Optional renderers consume generated text manifests.
"""
from pathlib import Path
import json
import subprocess
import sys
import numpy as np
from PIL import Image, ImageDraw
from comparison_images import load_srgb, COLOR_MANAGEMENT

captures, output = map(Path,sys.argv[1:3])
output.mkdir(parents=True,exist_ok=True)
records=[]
manifests={kind:[] for kind in ['before','after']}
for directory in sorted(captures.glob('pair-*')):
    if not directory.is_dir(): continue
    trace=json.loads((directory/'trace.json').read_text())
    ids={entry['name']:entry['id'] for entry in trace['schema'] if entry['name'] in ['glass0','glass1']}
    for snap in trace['snapshots']:
        if not snap['success']: continue
        frame=min(trace['frames'],key=lambda frame:abs(frame['t']-snap['t']))
        rows={row[0]:row for row in frame['layers']}
        rectangles=[rows[ids[f'glass{i}']][1:5] for i in range(2)]
        # The fixture captures held/pulled while down, release images after up.
        # Native lighting opacity is not logged; use the existing NXKit glow.
        glow=1 if snap['name'] in ['held','pulled'] else 0
        events=[event for event in trace['events'] if event['t']<=snap['t']]
        touch=events[-1]
        record=dict(case=directory.name,checkpoint=snap['name'],dark=trace['configuration']['dark'],
            stage=trace['stage'],scale=trace['scale'],rectangles=rectangles,
            snapshotTime=snap['t'],geometryTime=frame['t'],geometryTimeDelta=frame['t']-snap['t'],
            glow=glow,touch=[touch['x'],touch['y']],native=str(directory/f"{snap['name']}.png"))
        records.append(record)
        for kind in manifests:
            path=output/kind/f"{directory.name}-{snap['name']}.ppm";path.parent.mkdir(exist_ok=True)
            record[kind]=str(path)
            values=[int(record['dark']),glow,*record['touch'],*rectangles[0],*rectangles[1]]
            manifests[kind].append(json.dumps(str(path))+' '+' '.join(map(str,values)))
for kind,lines in manifests.items():
    manifest=output/f'{kind}.txt';manifest.write_text('\n'.join(lines)+'\n')
    if len(sys.argv)==5: subprocess.run([sys.argv[3 if kind=='before' else 4],str(manifest)],check=True)
if any(not Path(record[kind]).exists() for record in records for kind in manifests):
    print('Manifests written; render them and rerun without renderer arguments.');sys.exit(0)

metrics=[]; montage_rows=[]
roi=(286,174,438,266)
for record in records:
    native=load_srgb(record['native'])
    sx,sy,sw,sh=record['stage'];scale=record['scale']
    if native.size != (round(sw*scale),round(sh*scale)):
        native=native.crop(tuple(round(v*scale) for v in (sx,sy,sx+sw,sy+sh)))
    images=[native,load_srgb(record['before']),load_srgb(record['after'])]
    crop=tuple(round(v*scale) for v in roi)
    # Fixed seam ROI is shared across inputs; no image registration, color
    # normalization or alignment is fitted to reduce the reported error.
    seam=(344,200,374,240)
    seam_crop=tuple(round(v*scale) for v in seam)
    ref=np.asarray(native.crop(seam_crop),dtype=float)
    entry={k:record[k] for k in ['case','checkpoint','dark','rectangles','snapshotTime','geometryTimeDelta','glow','touch']}
    for kind,image in zip(['before','after'],images[1:]):
        values=np.asarray(image.crop(seam_crop),dtype=float)
        entry[kind+'SeamMAE']=float(np.mean(np.abs(values-ref)))
        entry[kind+'SeamP95']=float(np.percentile(np.abs(values-ref),95))
    metrics.append(entry)
    if record['checkpoint'] in ['held','pulled','release-2','release-5']:
        row=Image.new('RGB',(3*456,322),'#202020');draw=ImageDraw.Draw(row)
        draw.text((8,5),f"{record['case']} / {record['checkpoint']} — native geometry; glow {record['glow']}",fill='white')
        for index,(label,image) in enumerate(zip(['UIKit 27.0','NXKit before','NXKit after'],images)):
            row.paste(image.crop(crop).resize((456,276)),(456*index,44));draw.text((456*index+8,25),label,fill='white')
        montage_rows.append(row)
canvas=Image.new('RGB',(1368,sum(row.height for row in montage_rows)),'#202020')
y=0
for row in montage_rows:canvas.paste(row,(0,y));y+=row.height
canvas.save(output/'merge-native-before-after.png')
summary=dict(colorManagement=COLOR_MANAGEMENT,cases=len(metrics),nativeOS='27.0',seamROI=seam,
    meanBeforeSeamMAE=float(np.mean([m['beforeSeamMAE'] for m in metrics])),
    meanAfterSeamMAE=float(np.mean([m['afterSeamMAE'] for m in metrics])),
    limitations=['Native geometry is injected, not produced by NXKit touch dynamics.',
        'Nearest sampled geometry is up to one display frame from framebuffer snapshot.',
        'Native glow opacity is unmeasured; held/pulled use NXKit glow=1, release uses0.',
        'Gray underlay only; results do not prove matching refraction on artwork.'],checkpoints=metrics)
(output/'metrics.json').write_text(json.dumps(summary,indent=2)+'\n')
print(json.dumps({key:value for key,value in summary.items() if key!='checkpoints'},indent=2))
