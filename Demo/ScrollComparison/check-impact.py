"""Check fast edge impacts against the 13 retained public UIKit captures.

Usage: check-impact.py SCROLL_REPLAY [REPORT_DIRECTORY]
Inputs preserve native coordinates and delivery timestamps; no registration.
"""
from pathlib import Path
import json
import math
import subprocess
import sys
import tempfile

exe = Path(sys.argv[1]).resolve()
report = Path(sys.argv[2]).resolve() if len(sys.argv) > 2 else Path(__file__).resolve().parents[2] / 'docs/scroll-physics/impact'
captures = sorted((report / 'native').glob('scroll-impact-*.json'))
assert len(captures) == 13, 'Expected all eight impact and five hold captures'
with tempfile.TemporaryDirectory() as directory:
    for path in captures:
        data = json.loads(path.read_text())
        name = data['configuration']['id']
        timeline = sorted(
            [(e.get('deliveredAt', e['t']), 0, i, 'E', e) for i, e in enumerate(data['events'])]
            + [(f[0], 1, i, 'F', f) for i, f in enumerate(data['frames'])], key=lambda row: row[:3])
        output = Path(directory) / (name + '.txt')
        subprocess.run([str(exe), str(report / (name + '-input.txt')), str(output)], check=True, stdout=subprocess.DEVNULL)
        rows = [line.split() for line in output.read_text().splitlines()]
        assert len(rows) == len(timeline), (name, 'incomplete replay')
        errors = [(float(r[2]) - sample[4][1]) ** 2 + (float(r[3]) - sample[4][2]) ** 2
                  for r, sample in zip(rows, timeline) if sample[3] == 'F']
        rms = math.sqrt(sum(errors) / len(errors))
        # Activation/coalescing differences remain in the captures. This limit
        # guards the complete measured waveform rather than fitting it away.
        assert rms < 12, (name, 'offset RMS', rms)
        direction = 1 if '-bottom-' in name else -1
        # The reference's content is 1600pt tall, with automatic insets disabled.
        boundary = 1600 - data['stage'][3] if direction > 0 else 0
        release = next(e for e in data['events'] if any(t[1] == 3 for t in e['touches']))
        end = release.get('deliveredAt', release['t'])
        expected_peak = max(direction * (f[2] - boundary) for f in data['frames'] if f[0] >= end)
        actual_peak = max(direction * (float(r[3]) - boundary) for r in rows if r[0] == 'F' and float(r[1]) >= end)
        assert abs(actual_peak - expected_peak) < 15, (name, 'peak stretch', actual_peak, expected_peak)
        assert abs(float(rows[-1][3]) - boundary) < .001, (name, 'edge return did not settle')
        print(f'{name}: {rms:.3f}pt RMS; peak {actual_peak:.2f}pt / UIKit {expected_peak:.2f}pt')
print('13 native impact/hold checks passed')
