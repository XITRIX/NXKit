"""Retain the native oracle's complete output and compact motion CSV.

Usage: collect.py SIMULATOR_UUID OUTPUT_DIRECTORY BUILD_DIRECTORY
"""
from pathlib import Path
import csv
import gzip
import hashlib
import json
import shutil
import subprocess
import sys

device, destination, build = sys.argv[1:]
output = Path(destination).resolve()
build = Path(build).resolve()
output.mkdir(parents=True, exist_ok=True)
container = Path(subprocess.check_output([
    'xcrun', 'simctl', 'get_app_container', device,
    'org.nxkit.SwitchReference', 'data'], text=True).strip())
cases = json.loads((build / 'Cases.json').read_text())
manifest = {'simulator': device, 'cases': [], 'files': {}}
for case in cases:
    source = container / 'Documents' / case['id']
    target = output / case['id']
    target.mkdir(exist_ok=True)
    geometry = json.loads((source / 'geometry.json').read_text())
    trace = json.loads((source / 'trace.json').read_text())
    assert trace['configuration'] == case
    shutil.copy2(source / 'geometry.json', target / 'geometry.json')
    with (target / 'trace.json.gz').open('wb') as file:
        with gzip.GzipFile(filename='', mode='wb', fileobj=file, mtime=0) as archive:
            archive.write((source / 'trace.json').read_bytes())
    lens = [row for row in geometry['visibleSwitch']['views']
            if row['class'] == '_UILiquidLensView'][0]
    manifest['cases'].append({
        'id': case['id'], 'os': trace['os'], 'scale': trace['scale'],
        'frames': len(trace['frames']), 'matrixCases': len(geometry['cases']),
        'restingLensFrame': lens['frame'], 'touches': len(trace['touches']),
        'actions': trace['actions'],
    })
    if case['id'] == 'on-hold-dark':
        down, up = trace['touches'][0]['dispatchT'], trace['touches'][-1]['dispatchT']
        rows = []
        for frame in trace['frames']:
            lens_row = next(row for row in frame['layers']
                            if row['path'] == 'slider.0.2')
            phase = 'lift' if frame['t'] < up else 'lower'
            row = {'phase': phase,
                   'timeFromContact': frame['t'] - (down if phase == 'lift' else up)}
            for prefix, key in [('model', 'modelFrame'), ('presentation', 'presentationFrame')]:
                for component, value in zip(['X', 'Y', 'Width', 'Height'], lens_row[key]):
                    row[prefix + component] = value
            rows.append(row)
        with (output / 'lift-motion.csv').open('w') as file:
            writer = csv.DictWriter(file, fieldnames=list(rows[0]))
            writer.writeheader()
            writer.writerows(rows)
shutil.copy2(build / 'source-manifest.json', output / 'source-manifest.json')
for file in sorted(output.rglob('*')):
    if file.is_file() and file.name != 'manifest.json':
        manifest['files'][str(file.relative_to(output))] = hashlib.sha256(file.read_bytes()).hexdigest()
(output / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
print(json.dumps(manifest['cases'], indent=2))
