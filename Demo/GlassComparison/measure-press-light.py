"""Measure native/current held/settled body color without seam or glyph pixels.

Usage: python measure-press-light.py CAPTURES OUTPUT [CHECKPOINT_RENDERER]
CAPTURES contains case directories with InteractionReference trace.json and PNGs.
Optional renderer uses native geometry, bypassing NXKit touch dynamics.
"""
from pathlib import Path
import json
import subprocess
import sys
import numpy as np
from PIL import Image
from comparison_images import load_srgb, COLOR_MANAGEMENT

captures,output=map(Path,sys.argv[1:3]);output.mkdir(parents=True,exist_ok=True)
records=[];lines=[]
for directory in sorted(captures.iterdir()):
    if not directory.is_dir() or not (directory/'trace.json').exists():continue
    trace=json.loads((directory/'trace.json').read_text());configuration=trace['configuration']
    if configuration.get('width',52)!=52 or configuration.get('height',52)!=52:continue
    pair=configuration.get('pair',False)
    resolved=trace.get('resolvedSystemBlue')
    tint=[0,122,255] if configuration.get('fixedTint',False) or resolved is None else [round(channel*255) for channel in resolved[:3]]
    target='glass1' if pair else 'glass0'
    layer=next(entry['id'] for entry in trace['schema'] if entry['name']==target)
    neighbor=next((entry['id'] for entry in trace['schema'] if entry['name']=='glass0'),None)
    for phase in ['held','release-5']:
        snap=next((snap for snap in trace['snapshots'] if snap['name']==phase and snap['success']),None)
        if snap is None:continue
        frame=min(trace['frames'],key=lambda frame:abs(frame['t']-snap['t']));rows={row[0]:row for row in frame['layers']}
        rect=rows[layer][1:5];rectangles=[rows[neighbor][1:5],rect] if pair else [rect,[0,0,1,1]]
        touch=next(event for event in trace['events'] if event['phase']==0)
        path=output/f'{directory.name}-{phase}.ppm'
        record=dict(case=directory.name,phase=phase,native=str(directory/f'{phase}.png'),raster=str(path),
            dark=configuration.get('dark',False),tinted=configuration.get('tinted',False),pair=pair,
            rectangle=rect,geometryTimeDelta=frame['t']-snap['t'],scale=trace['scale'],tintRGB=tint,
            tintInputAssumed=bool(configuration.get('tinted',False) and resolved is None and not configuration.get('fixedTint',False)))
        records.append(record)
        values=[int(record['dark']),int(phase=='held'),touch['x'],touch['y'],*rectangles[0],*rectangles[1],int(pair),int(record['tinted']),*tint]
        lines.append(json.dumps(str(path))+' '+' '.join(map(str,values)))
manifest=output/'manifest.txt';manifest.write_text('\n'.join(lines)+'\n')
if len(sys.argv)>3:subprocess.run([sys.argv[3],str(manifest)],check=True)
for record in records:
    f=record['rectangle'];scale=record['scale']
    for name,path in [('native',record['native']),('raster',record['raster'])]:
        image=load_srgb(path);values=[]
        # Four quadrants, >7 local points from glyph axes and >11 from rim.
        for x in [-10,-8,8,10]:
            for y in [-10,-8,8,10]:
                px=f[0]+f[2]*(.5+x/52);py=f[1]+f[3]*(.5+y/52)
                values.append(image.getpixel((round(px*scale),round(py*scale))))
        record[name+'MeanRGB']=np.mean(values,axis=0).tolist()
    record['bodyMAE']=float(np.mean(np.abs(np.array(record['nativeMeanRGB'])-record['rasterMeanRGB'])))
responses=[]
for case in sorted({record['case'] for record in records}):
    checkpoints={record['phase']:record for record in records if record['case']==case}
    if not all(phase in checkpoints for phase in ['held','release-5']):continue
    response=dict(case=case,dark=checkpoints['held']['dark'],tinted=checkpoints['held']['tinted'],pair=checkpoints['held']['pair'])
    for source in ['native','raster']:
        held=np.array(checkpoints['held'][source+'MeanRGB']);rest=np.array(checkpoints['release-5'][source+'MeanRGB'])
        headroom=255-rest;valid=headroom>20
        response[source+'WhiteEquivalentAlpha']=float(np.dot((held-rest)[valid],headroom[valid])/np.dot(headroom[valid],headroom[valid]))
        response[source+'DeltaRGB']=(held-rest).tolist()
        response[source+'PerChannelAlpha']=np.divide(held-rest,headroom,out=np.zeros(3),where=headroom>0).tolist()
    native_base=np.array(checkpoints['release-5']['nativeMeanRGB'])
    native_held=np.array(checkpoints['held']['nativeMeanRGB'])
    projected_held=native_base+response['rasterWhiteEquivalentAlpha']*(255-native_base)
    response['overlayOnlyMAE']=float(np.mean(np.abs(native_held-projected_held)))
    response['overlayOnlyPredictedHeldRGB']=projected_held.tolist()
    responses.append(response)
report=dict(colorManagement=COLOR_MANAGEMENT,samples='16 equal-weight local diagonal body samples per retained view; no fitted alignment or color normalization.',
    limitation='White-equivalent alpha describes measured response, not UIKit shader implementation. Resting tint error is reported separately.',
    checkpoints=records,responses=responses)
(output/'press-light.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(responses,indent=2))
