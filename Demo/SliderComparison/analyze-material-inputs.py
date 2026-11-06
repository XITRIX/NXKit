"""Summarize bounded native glassBackground reads; no UIKit execution."""
import csv, gzip, json, math, statistics, sys
from pathlib import Path

source, output = Path(sys.argv[1]), Path(sys.argv[2])
output.mkdir(parents=True, exist_ok=True)
data = json.loads(gzip.decompress(source.read_bytes()) if source.suffix == '.gz' else source.read_bytes())
start, end = data['touches'][0]['dispatchT'], data['touches'][-1]['dispatchT']
rows = []
for frame in data['frames']:
    layers = {row['id']: row for row in frame['layers']}
    material = next((row for row in frame['layers'] if any(f.get('type') == 'glassBackground' for f in row.get('modelFilters', []))), None)
    if material is None:
        continue
    model = next(f for f in material['modelFilters'] if f['type'] == 'glassBackground')
    presentation = next(f for f in material['presentationFilters'] if f['type'] == 'glassBackground')
    rows.append((frame, layers, material, model, presentation))
assert rows
keys = sorted(key for key in rows[0][3] if key.startswith('input'))
summary = {'source': str(source), 'layerID': rows[0][2]['id'], 'path': rows[0][2]['path'],
           'fields': {}, 'limits': 'One light-mode native stationary contact; raw CF color/size descriptions retained. No dark-mode material endpoint or regrab claim.'}
for key in keys:
    values = [row[4].get(key) for row in rows]
    summary['fields'][key] = {'liftedModel': rows[0][3].get(key), 'retiredModel': rows[-1][3].get(key),
                              'presentationDistinctCount': len({json.dumps(value, sort_keys=True) for value in values})}
    if all(isinstance(value, (int, float)) for value in values):
        summary['fields'][key].update(presentationMinimum=min(values), presentationMaximum=max(values))
velocities = []
omega = 2 * math.pi / .4
for frame, _, _, _, presentation in rows:
    q = presentation['inputFaceOpacity']
    if frame['t'] < end or not 0 < q < .999:
        continue
    low, high = 0, 1
    for _ in range(60):
        t = (low + high) / 2
        if (1 + omega * t) * math.exp(-omega * t) > q:
            low = t
        else:
            high = t
    t = (low + high) / 2
    inner_q = presentation['inputInnerRefractionAmount'] / summary['fields']['inputInnerRefractionAmount']['liftedModel']
    velocities.append((q - inner_q) / (t * math.exp(-omega * t)))
summary['inferredCriticalDecayFit'] = {'response': .4, 'faceInitialDecayVelocity': 0,
    'innerInitialDecayVelocityMean': statistics.mean(velocities),
    'innerInitialDecayVelocityMinimum': min(velocities), 'innerInitialDecayVelocityMaximum': max(velocities),
    'sampleCount': len(velocities), 'scope': 'Reconstruction inferred from interior presentation inputs; not recovered native source.'}
(output / 'endpoints.json').write_text(json.dumps(summary, indent=2) + '\n')
with (output / 'inputs.csv').open('w') as handle:
    writer = csv.writer(handle)
    writer.writerow(['dispatchRelativeSeconds', 'endRelativeSeconds', 'frameID', 'whiteOpacity', 'innerShadowOpacity', 'rootOpacityPair', 'externalHighlightOpacity'] + keys)
    for frame, layers, material, model, presentation in rows:
        pair = next((f.get('inputAmount') for f in layers[5]['presentationFilters'] if f.get('type') == 'opacityPair'), None)
        writer.writerow([frame['t'] - start, frame['t'] - end, frame['frameID'], layers[7]['presentationOpacity'],
                         layers.get(17, {}).get('shadowOpacity'), pair, layers.get(32, {}).get('presentationOpacity')]
                        + [presentation.get(key) for key in keys])
print(f'{len(rows)} material samples; {len(keys)} explicitly requested inputs; layer {summary["layerID"]}')
