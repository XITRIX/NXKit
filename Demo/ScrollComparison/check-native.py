"""Validate a built ScrollReplay against retained native inputs without a baseline binary.
Usage: check-native.py SCROLL_REPLAY [REPORT_DIRECTORY]
"""
from pathlib import Path
import json,sys,tempfile,subprocess
import numpy as np
exe=Path(sys.argv[1]).resolve()
report=Path(sys.argv[2]).resolve() if len(sys.argv)>2 else Path(__file__).resolve().parents[2]/'docs/scroll-physics'
limits={'scroll-flick':1,'scroll-catch-inertia':.5,'scroll-pull':2,'scroll-bounce-catch':6,'scroll-bounce-regrab':6,'scroll-resize-drag':1,'scroll-resize-bounce':3,'scroll-rotate-bounce':2,'scroll-slow-release':.5,'scroll-release-20':.5,'scroll-release-40':.5,'scroll-linear-249':5,'scroll-linear-250':5,'scroll-linear-251':5}
with tempfile.TemporaryDirectory() as directory:
 for name,limit in limits.items():
  d=json.loads((report/'native'/(name+'.json')).read_text())
  timeline=sorted([(e.get('deliveredAt',e['t']),0,i,'E',e) for i,e in enumerate(d['events'])]+[(f[0],1,i,'F',f) for i,f in enumerate(d['frames'])],key=lambda row:row[:3])
  output=Path(directory)/(name+'.txt')
  subprocess.run([str(exe),str(report/(name+'-input.txt')),str(output)],check=True,stdout=subprocess.DEVNULL)
  rows=[line.split() for line in output.read_text().splitlines()]
  assert len(rows)==len(timeline),(name,'replay length mismatch')
  errors=[(float(row[2])-native[4][1])**2+(float(row[3])-native[4][2])**2 for row,native in zip(rows,timeline) if native[3]=='F']
  error=float(np.mean(errors)**.5)
  assert error<=limit,(name,error,limit)
  expected={'scroll-linear-249':False,'scroll-linear-250':True,'scroll-linear-251':True,'scroll-catch-inertia':False,'scroll-slow-release':False}
  if name in expected:
   ends=[row for row,native in zip(rows,timeline) if native[3]=='E' and all(t[1] in (3,4) for t in native[4]['touches'])]
   assert bool(int(ends[-1][9]))==expected[name],(name,'release deceleration state')
  print(f'{name}: {error:.3f} pt RMS')
print(f'{len(limits)} retained UIKit replay checks passed')
