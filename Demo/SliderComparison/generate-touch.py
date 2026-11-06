#!/usr/bin/env python3
"""Retain independent OS-delivered UIKit touch inputs and settled endpoints."""
import argparse, hashlib, json
from pathlib import Path

root=Path(__file__).resolve().parents[2]
parser=argparse.ArgumentParser();parser.add_argument('--check',action='store_true');args=parser.parse_args()
cases=['calibration/zero','calibration/slow','calibration/offset-slow','calibration/fast',
       'velocity/velocity-200','velocity/velocity-400']
lines=['#pragma once','// Generated from retained OS-touch inputs; UIKit 27.0, 300pt track.',
       'struct SliderNativeTouchSample { int phase; double time; NXKit::NXPoint point; };',
       'struct SliderNativeTouchTrace { const char* id; float finalValue; std::vector<SliderNativeTouchSample> samples; };',
       'inline const SliderNativeTouchTrace sliderNativeTouches[] = {']
for case in cases:
    directory=root/'docs/slider/native'/case
    source=(directory/'touches.json').read_bytes();samples=json.loads(source)
    events=json.loads((directory/'events.json').read_text())
    final=[e['value'] for e in events if e.get('event')==4096][-1]
    lines.append('// Touch input SHA256: '+hashlib.sha256(source).hexdigest())
    lines.append('{'+json.dumps(case)+','+str(final)+',{')
    start=samples[0]['timestamp']
    for sample in samples:
        lines.append('{'+str(sample['phase'])+','+str(1+sample['timestamp']-start)+',{'+','.join(str(p) for p in sample['point'])+'}},')
    lines.append('}},')
lines+=['};',''];text='\n'.join(lines)
output=root/'UIKit/tests/SliderNativeTouchData.hpp'
if args.check:
    assert output.read_text()==text,'Native slider touch expectations need regeneration'
    print('Verified',len(cases),'independent UIKit touch traces')
else:output.write_text(text)
