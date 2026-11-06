"""Compare real UIKit tab taps with routed NXKit presentation samples.
Usage: python analyze-tab-transfer.py NATIVE_CAPTURE PORTABLE_CSV_DIRECTORY OUTPUT
Temporal alignment isolates trajectory shape; it is not an input-latency score.
"""
import csv
import gzip
import json
import math
from pathlib import Path
import sys

native, portable, output = map(Path, sys.argv[1:])
output.mkdir(parents=True, exist_ok=True)
comparisons = []
scenarios = [('first-fourth', '0-3'), ('fourth-first', '3-0'), ('adjacent', '0-1'), ('reselect', '1-1'), ('first-fourth-dark', '0-3'), ('hold-fourth', None)]
for name, replay in scenarios:
    raw = (native / ('motion-' + name) / 'trace.json').read_bytes()
    trace = json.loads(raw)
    (output / (name + '.json.gz')).write_bytes(gzip.compress(raw, mtime=0))
    entry = next(s for s in trace['schema'] if s['viewClass'] == '_UILiquidLensView')
    rows = [(f['t'], r) for f in trace['frames'] for r in f['layers'] if r[0] == entry['id']]
    with (output / (name + '.csv')).open('w') as file:
        writer = csv.writer(file)
        writer.writerow(['t'] + trace['columns'])
        writer.writerows([[t] + r for t, r in rows])
    down = next((t for (t, r), (_, previous) in zip(rows[1:], rows) if r[16] == 54 and previous[16] == 70), None)
    raised = next(t for t, r in rows if r[16] == 70)
    item = dict(scenario=name, nativeFrames=len(rows), nativeTouchEvents=trace['touchEvents'],
                nativeLiftStarts=raised, nativeLowerStarts=down,
                nativeRestSize=rows[0][1][15:17],
                nativeMaxSize=[max(r[i] for _, r in rows) for i in [15, 16]],
                nativeElasticX=[min(r[9] for _, r in rows), max(r[9] for _, r in rows)],
                nativeElasticY=[min(r[12] for _, r in rows), max(r[12] for _, r in rows)])
    if replay:
        samples = [{k: float(v) for k, v in r.items()} for r in csv.DictReader((portable / (replay + '.csv')).open())]
        def interpolate(t):
            if t <= samples[0]['t']:
                return samples[0]['centerX']
            for a, b in zip(samples, samples[1:]):
                if t <= b['t']:
                    return a['centerX'] + (b['centerX'] - a['centerX']) * (t - a['t']) / (b['t'] - a['t'])
            return samples[-1]['centerX']
        start, end = rows[0][1][23], rows[-1][1][23]
        pstart = 51.5 if name != 'fourth-first' else 308.5
        # Reselect is stationary; compensate its different slot in the replay.
        shift = start - (137.166667 if name == 'reselect' else pstart)
        scored = [(t, r[23]) for t, r in rows if .1 <= t <= .7]
        def rmse(delay):
            return math.sqrt(sum((interpolate(max(0, t-delay)) + shift - x)**2 for t, x in scored) / len(scored))
        delay = raised if name == 'reselect' else min((i / 1000 for i in range(151)), key=rmse)
        portable_down = next(r['t'] for r in samples if r['lifted'] == 0)
        item.update(alignmentSeconds=delay, alignmentMethod='native lift activation' if name == 'reselect' else 'best position fit', centerRMSEPoints=rmse(delay),
                    portableLowerStarts=portable_down,
                    alignedLowerDifferenceSeconds=portable_down+delay-down,
                    portableElasticX=[min(r['elasticX'] for r in samples), max(r['elasticX'] for r in samples)],
                    portableElasticY=[min(r['elasticY'] for r in samples), max(r['elasticY'] for r in samples)])
    comparisons.append(item)
(output / 'comparison.json').write_text(json.dumps(dict(
    evidence='Native UIKit controls responding to OS touches vs NXKit routed-touch replay. Full native layers retained as gzip JSON.',
    limitation='Best temporal alignment scores position shape, not end-to-end latency. Short-tap elasticity remains approximate; snapshots are excluded from these scores.',
    comparisons=comparisons), indent=2) + '\n')
for item in comparisons:
    print(item['scenario'], {key: item[key] for key in ['nativeLowerStarts', 'alignmentSeconds', 'centerRMSEPoints', 'alignedLowerDifferenceSeconds'] if key in item})
