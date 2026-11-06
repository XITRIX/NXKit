"""Render the production NXKit gallery with the native series' retained states.
Usage: python render-gallery.py RENDERER ARTWORK NATIVE_SERIES OUTPUT
"""
from concurrent.futures import ThreadPoolExecutor
import json
from pathlib import Path
import subprocess
import sys
renderer,artwork,series,output=map(Path,sys.argv[1:])
output=output.resolve(); output.mkdir(parents=True,exist_ok=True)
records=json.loads((series/'series.json').read_text()); groups={}
for row in records: groups.setdefault((row['case'],row['dark']),[]).append(row)
def render(rows):
    manifest=output/f'case{rows[0]["case"]}-dark{rows[0]["dark"]}.txt'
    manifest.write_text(''.join(f'"{r["name"]}.ppm" {r["case"]} {r["dark"]} {r["progress"]} {r["backdrop"]} {r.get("reverse",0)}\n' for r in rows))
    log=output/f'{manifest.stem}.log'
    with log.open('w') as stream:
        subprocess.run([str(renderer.resolve()),str(artwork.resolve()),str(output),str(manifest)],check=True,stdout=stream,stderr=subprocess.STDOUT)
    print(manifest.stem,flush=True)
with ThreadPoolExecutor(max_workers=4) as executor: list(executor.map(render,groups.values()))
