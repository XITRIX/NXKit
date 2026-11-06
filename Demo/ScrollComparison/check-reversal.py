"""Replay the retained native rubber-band reversal cases and verify continuation.
Usage: check-reversal.py SCROLL_REPLAY [REPORT_DIRECTORY]
"""
from pathlib import Path
import json,sys,tempfile,subprocess
import numpy as np
exe=Path(sys.argv[1]).resolve();report=Path(sys.argv[2]).resolve() if len(sys.argv)>2 else Path(__file__).resolve().parents[2]/'docs/scroll-physics/reversal'
count=0
with tempfile.TemporaryDirectory() as directory:
 for path in sorted((report/'native').glob('scroll-reverse-*.json')):
  d=json.loads(path.read_text());name=d['configuration']['id'];edge=d['configuration']['offset'];direction=-1 if edge else 1
  timeline=sorted([(e.get('deliveredAt',e['t']),0,i,'E',e) for i,e in enumerate(d['events'])]+[(f[0],1,i,'F',f) for i,f in enumerate(d['frames'])],key=lambda row:row[:3])
  output=Path(directory)/(name+'.txt');subprocess.run([str(exe),str(report/(name+'-input.txt')),str(output)],check=True,stdout=subprocess.DEVNULL)
  rows=[line.split() for line in output.read_text().splitlines()];assert len(rows)==len(timeline)
  errors=[(float(row[2])-native[4][1])**2+(float(row[3])-native[4][2])**2 for row,native in zip(rows,timeline) if native[3]=='F'];error=float(np.mean(errors)**.5)
  limit=12 if name=='scroll-reverse-top-dense-deep' else 6 if 'deep' in name else 5 if 'slow' in name else 2
  assert error<=limit,(name,error,limit)
  final=float(rows[-1][3]);native_final=d['frames'][-1][2]
  if 'slow' in name:assert abs(final-edge)<.001,(name,'slow release should settle at the edge')
  else:
   assert direction*(final-edge)>5,(name,'inward inertia was trapped at the edge')
   assert direction*(native_final-edge)>5,(name,'native fixture did not continue into content')
  count+=1;print(f'{name}: {error:.3f} pt RMS')
assert count==12,('expected all 12 native reversal captures',count)
print(f'{count} native reversal checks passed')
