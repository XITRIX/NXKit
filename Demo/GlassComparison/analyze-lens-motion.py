"""Summarize recorded native layer motion without fitting a physical model.
Usage: python analyze-lens-motion.py TRACE_JSON OUTPUT_DIRECTORY
All raw coordinates and clocks remain available in TRACE_JSON. Selection of the
active lens uses observed movement; no reference geometry is subtracted.
"""
from pathlib import Path
import csv
import json
import sys

trace_path, output = Path(sys.argv[1]), Path(sys.argv[2])
output.mkdir(parents=True, exist_ok=True)
trace=json.loads(trace_path.read_text())
columns=trace['columns']
lenses=[]
for entry in trace['schema']:
    if entry['viewClass'] != '_UILiquidLensView':
        continue
    rows=[(frame['t'],row) for frame in trace['frames'] for row in frame['layers'] if row[0]==entry['id']]
    if not rows:
        continue
    spread=lambda index:max(row[index] for _,row in rows)-min(row[index] for _,row in rows)
    lenses.append((spread(1)+spread(2)+spread(3)+spread(4),entry,rows))
_,active,rows=max(lenses,key=lambda item:item[0])
with (output/'active-lens.csv').open('w') as file:
    writer=csv.writer(file);writer.writerow(['t']+columns)
    writer.writerows([[time]+row for time,row in rows])
with (output/'touches.csv').open('w') as file:
    writer=csv.writer(file);writer.writerow(['t','dispatchT','phase','x','y','targetClass'])
    writer.writerows([[touch['t'],touch['dispatchT'],touch['phase'],*touch['point'],touch['targetClass']] for touch in trace['touchEvents']])
if trace.get('flexDiagnostics'):
    with (output/'flex.csv').open('w') as file:
        writer=csv.writer(file)
        writer.writerow(['t','layerID','scaleX','scaleY','driftX','driftY','translationY','positionX','positionY','velocityX','velocityY','accelerationX','accelerationY','offsetX','offsetY','hysteresisTimeInterval','maxNumberOfSamples','prefersDirectionlessAcceleration','useSampleWindowForDifferentiation','shouldUsePresentationLayers'])
        for sample in trace['flexDiagnostics']:
            if sample['layerID'] != active['id']:
                continue
            values=sample['values'];integrator=values['velocityIntegrator']
            row=[sample['t'],sample['layerID']]+[values.get(key) for key in ['scaleX','scaleY','driftX','driftY','translationY']]
            for key in ['position','velocity','acceleration','offset']:
                row+=integrator.get(key,[None,None])
            row+=[integrator.get(key) for key in ['hysteresisTimeInterval','maxNumberOfSamples','prefersDirectionlessAcceleration','useSampleWindowForDifferentiation']]+[values.get('shouldUsePresentationLayers')]
            writer.writerow(row)
summary=dict(configuration=trace['configuration'],os=trace['os'],osBuild=trace['osBuild'],scale=trace['scale'],
    evidenceType=trace['evidenceType'],activeLens=active,frames=len(rows),touchEvents=len(trace['touchEvents']),
    firstTouch=trace['touchEvents'][0],lastTouch=trace['touchEvents'][-1],nativeActions=trace.get('nativeActions',[]),
    initial={columns[index]:value for index,value in enumerate(rows[0][1])},
    ranges={name:[min(row[index] for _,row in rows),max(row[index] for _,row in rows)] for index,name in enumerate(columns) if index},
    observation='Ranges include native lift inflation, control placement, affine deformation and release. They are not isolated drag coefficients.')
(output/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
print(json.dumps(summary,indent=2))
