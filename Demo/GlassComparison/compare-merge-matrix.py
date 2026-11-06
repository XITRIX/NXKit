"""Color-managed, geometry-aligned comparison of native retained glass pairs.

Usage: python compare-merge-matrix.py CAPTURES OUTPUT RENDERER [BASELINE_OUTPUT]
Use RENDERER=- to recompute metrics from existing rasters.
Native original PNGs/traces are preserved. A baseline output can be reused for
candidate experiments without relinking or rerendering its frozen implementation.
Requires Pillow and NumPy. The renderer consumes the generated v2 manifest.
"""
from pathlib import Path
import json
import subprocess
import sys
import numpy as np
from PIL import Image, ImageDraw
from comparison_images import load_srgb, COLOR_MANAGEMENT

captures,output,renderer=map(Path,sys.argv[1:4]);output.mkdir(parents=True,exist_ok=True)
baseline=Path(sys.argv[4]) if len(sys.argv)>4 else None
records=[];manifest=[]
phases=['initial','held','pulled','release-5']
for directory in sorted(captures.iterdir()):
    if not directory.is_dir() or not (directory/'trace.json').exists():continue
    trace=json.loads((directory/'trace.json').read_text());config=trace['configuration']
    if not config.get('pair'):continue
    ids={entry['name']:entry['id'] for entry in trace['schema'] if entry['name'] in ['glass0','glass1']}
    resolved=trace.get('resolvedItemTints',{})
    colors=[None if resolved.get(f'glass{i}') is None else [round(channel*255) for channel in resolved[f'glass{i}']] for i in range(2)]
    for snapshot in trace['snapshots']:
        if snapshot['name'] not in phases or not snapshot['success']:continue
        frame=min(trace['frames'],key=lambda frame:abs(frame['t']-snapshot['t']));rows={row[0]:row for row in frame['layers']}
        rectangles=[rows[ids[f'glass{i}']][1:5] for i in range(2)]
        if any(value is None for rect in rectangles for value in rect):raise ValueError('Nonfinite native rectangle')
        events=[event for event in trace['events'] if event['t']<=snapshot['t']]
        touch=(events[-1] if events else trace['events'][0]);glow=int(snapshot['name'] in ['held','pulled'])
        path=output/'rasters'/f"{directory.name}-{snapshot['name']}.ppm";path.parent.mkdir(exist_ok=True)
        background=config.get('backgroundRGB',[128,128,128])
        artwork=str((Path(__file__).resolve().parents[1]/'resources/img/glass_demo_scene.png')) if config.get('artwork') else ''
        values=[int(config.get('dark',False)),glow,touch['x'],touch['y'],*rectangles[0],*rectangles[1],1,int(config.get('reverseOrder',False)),config.get('spacing',8),int(config.get('clear',False)),*background]
        line='v2 '+json.dumps(str(path))+' '+' '.join(map(str,values))+' '+json.dumps(artwork)
        for color in colors:line+=' '+' '.join(map(str,[1,*color] if color else [0,0,0,0,0]))
        manifest.append(line)
        records.append(dict(case=directory.name,phase=snapshot['name'],configuration=config,colors=colors,
            native=str(directory/f"{snapshot['name']}.png"),raster=str(path),rectangles=rectangles,
            geometryTimeDelta=frame['t']-snapshot['t'],scale=trace['scale'],glow=glow))
manifest_path=output/'manifest.txt';manifest_path.write_text('\n'.join(manifest)+'\n')
if str(renderer) != "-":
    subprocess.run([str(renderer),str(manifest_path)],check=True)

def body(image,rect):
    x,y,w,h=rect;values=np.asarray(image,dtype=float)
    return np.mean([values[round(2*(y+h*(.5+dy/52))),round(2*(x+w*(.5+dx/52)))] for dx in [-10,-8,8,10] for dy in [-10,-8,8,10]],axis=0).tolist()

metrics=[];groups={}
for record in records:
    native=load_srgb(record['native']);current=load_srgb(record['raster'])
    if native.size!=current.size:raise ValueError('Native and raster stages must have identical pixel size')
    prior=load_srgb(baseline/'rasters'/Path(record['raster']).name) if baseline else None
    # All cases share a 720x440 stage centred at (360,220); this fixed waist
    # region does not fit or translate the images to minimize their error.
    roi=(344,200,376,240);scale=record['scale'];crop=tuple(round(v*scale) for v in roi)
    n=np.asarray(native.crop(crop),dtype=float);c=np.asarray(current.crop(crop),dtype=float)
    yy,xx=np.mgrid[crop[1]:crop[3],crop[0]:crop[2]];xx=(xx+.5)/scale;yy=(yy+.5)/scale
    mask=np.ones(xx.shape,dtype=bool)
    for x,y,w,h in record['rectangles']:
        lx=(xx-x)*52/w-26;ly=(yy-y)*52/h-26
        mask &= ~(((abs(lx)<11)&(abs(ly)<2))|((abs(lx)<2)&(abs(ly)<11)))
    entry={key:record[key] for key in ['case','phase','configuration','colors','rectangles','geometryTimeDelta','glow']}
    entry['nativeBodyRGB']=[body(native,rect) for rect in record['rectangles']]
    entry['currentBodyRGB']=[body(current,rect) for rect in record['rectangles']]
    entry['currentSeamMAE']=float(np.mean(np.abs(c-n)[mask]));entry['currentSeamP95']=float(np.percentile(np.abs(c-n)[mask],95))
    entry['currentMeanResidualRGB']=np.mean((c-n)[mask],axis=0).tolist()
    if prior:
        before=np.asarray(prior.crop(crop),dtype=float);entry['baselineSeamMAE']=float(np.mean(np.abs(before-n)[mask]))
    metrics.append(entry)
    display=(286*2,174*2,438*2,266*2);panels=[('UIKit 27.0 / ICC to sRGB',native)]
    if prior:panels.append(('NXKit baseline',prior))
    panels.append(('NXKit candidate' if prior else 'NXKit baseline',current))
    # Difference pixels are rendered without any geometric or color fitting.
    diff=np.clip(np.abs(np.asarray(current,dtype=float)-np.asarray(native,dtype=float))*4,0,255).astype('uint8')
    panels.append(('Absolute RGB difference x4',Image.fromarray(diff)))
    row=Image.new('RGB',(456*len(panels),322),'#202020');draw=ImageDraw.Draw(row)
    draw.text((8,5),record['case']+' / '+record['phase'],fill='white')
    for i,(label,image) in enumerate(panels):
        row.paste(image.crop(display).resize((456,276)),(456*i,44));draw.text((456*i+8,25),label,fill='white')
    key=record['case'];groups.setdefault(key,[]).append(row)
for case,rows in groups.items():
    canvas=Image.new('RGB',(rows[0].width,sum(row.height for row in rows)),'#202020');y=0
    for row in rows:canvas.paste(row,(0,y));y+=row.height
    canvas.save(output/f'{case}.png')
report=dict(colorManagement=COLOR_MANAGEMENT,states=len(metrics),seamROI=roi,
    method='Native measured presentation rectangles injected into NXKit. Fixed waist ROI, glyph mask only; no fitted registration or color correction.',
    currentMeanSeamMAE=float(np.mean([entry['currentSeamMAE'] for entry in metrics])),checkpoints=metrics)
if baseline:report['baselineMeanSeamMAE']=float(np.mean([entry['baselineSeamMAE'] for entry in metrics]))
(output/'metrics.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps({key:value for key,value in report.items() if key!='checkpoints'},indent=2))
