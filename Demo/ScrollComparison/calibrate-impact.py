"""Infer the edge-impact spring frequency from public UIKit peak measurements.
Usage: calibrate-impact.py NATIVE_DIRECTORY OUTPUT_JSON
No position/time registration or per-case correction is applied.
"""
from pathlib import Path
import json
import math
import sys

native, output = map(Path, sys.argv[1:3])
records = []
for path in sorted(native.glob('scroll-impact-*.json')):
    data = json.loads(path.read_text())
    name = data['configuration']['id']
    direction = 1 if '-bottom-' in name else -1
    boundary = 1600 - data['stage'][3] if direction > 0 else 0
    release = next(e for e in data['events'] if any(t[1] == 3 for t in e['touches']))
    end = release.get('deliveredAt', release['t'])
    stretch = direction * (release['sample'][2] - boundary)
    velocity = direction * next(e['velocity'][1] * 1000 for e in data['delegates'] if e['name'] == 'willEnd')
    peak = max(direction * (f[2] - boundary) for f in data['frames'] if f[0] >= end)
    if stretch <= 0:
        hit_velocity = velocity + (-1000 * math.log(.998)) * stretch
        omega = hit_velocity / (math.e * peak)
        predicted_peak = hit_velocity / (math.e * 11)
    else:
        # For outward release, the fitted coefficient is the raw release speed:
        # x(t) = (stretch + releaseSpeed*t) * exp(-omega*t).
        low, high = .001, velocity / stretch
        for _ in range(80):
            middle = (low + high) / 2
            value = velocity / middle * math.exp(-1 + middle * stretch / velocity)
            if value > peak:
                low = middle
            else:
                high = middle
        omega = (low + high) / 2
        predicted_peak = velocity / 11 * math.exp(-1 + 11 * stretch / velocity) if velocity > 11 * stretch else stretch
    records.append({'id': name, 'releaseStretch': stretch, 'releaseSpeed': velocity,
                    'nativePeak': peak, 'inferredAngularFrequency': omega,
                    'predictedPeakAt11': predicted_peak})
training = [r for r in records if r['id'].endswith('-flight')]
assert len(training) == 2
output.write_text(json.dumps({'measurement': 'Public UIKit release velocity, bounds and peak stretch; no fitted registration.',
    'trainingCases': [r['id'] for r in training],
    'meanFlightFrequency': sum(r['inferredAngularFrequency'] for r in training) / len(training),
    'portableImpactFrequency': 11, 'cases': records}, indent=2) + '\n')
print('Free-flight angular frequency:', sum(r['inferredAngularFrequency'] for r in training) / len(training))
