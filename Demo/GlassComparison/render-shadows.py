"""Render the native shadow matrix through production retained NXKit views.

Usage: python render-shadows.py SHADOW_RENDERER MATRIX OUTPUT
MATRIX contains native/series.json and backgrounds/<name>.png.
"""
import json
import subprocess
import sys
from pathlib import Path

renderer, matrix, output = map(lambda p: Path(p).resolve(), sys.argv[1:4])
output.mkdir(parents=True, exist_ok=True)
lines = []
for entry in json.loads((matrix / 'native/series.json').read_text()):
    request = entry['request']
    fields = [json.dumps(str(output / (request['id'] + '.ppm'))),
              json.dumps(str(matrix / 'backgrounds' / (request['background'] + '.png'))),
              str(int(request['dark'])), request['tint'],
              str(int(request.get('clear', False))), str(int(request.get('group', False))),
              str(request.get('spacing', 8))]
    fields += [str(value) for frame in entry['frames'] for value in frame]
    lines.append(' '.join(fields))
manifest = output / 'manifest.txt'
manifest.write_text('\n'.join(lines) + '\n')
subprocess.run([str(renderer), str(manifest)], check=True)
