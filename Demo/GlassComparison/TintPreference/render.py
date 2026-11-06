#!/usr/bin/env python3
"""Render every generated reference case through the production retained tree."""
from pathlib import Path
import json,subprocess,sys
renderer,app,out=map(lambda s:Path(s).resolve(),sys.argv[1:4]);out.mkdir(parents=True,exist_ok=True)
cases=json.loads((app/'cases.json').read_text());lines=[]
for amount in [0,.25,.5,.75,1]:
 for c in cases:
  path=out/f'{amount:g}-{c["id"]}.ppm'
  line=json.dumps(str(path))+' '+json.dumps(str(app/c["background"]))+f' {int(c["dark"])} {amount} '
  line+=' '.join(' '.join(map(str,q['frame']))+f' {int(q["clear"])} {int(q["tinted"])}' for q in c['probes']);lines.append(line)
manifest=out/'manifest.txt';manifest.write_text('\n'.join(lines)+'\n');subprocess.run([str(renderer),str(manifest)],check=True)
