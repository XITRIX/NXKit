#!/usr/bin/env python3
"""Split repeated lens phases into contiguous cycles before calculating FPS.

Never bridge the idle/other-phase gaps between separate drag cycles. Timing
quantiles include submitted frames only; presentedTime is actual delivery.
"""
import csv, json, sys
from pathlib import Path
from analyze_frame_metrics import distribution

def analyze(path):
    lines=Path(path).read_text().splitlines()
    rows=list(csv.DictReader(line for line in lines if not line.startswith('#')))
    cpu=[r for r in rows if r['kind']=='cpu']
    runs=[]
    for row in cpu:
        if not runs or runs[-1]['phase']!=row['phase']:
            runs.append(dict(phase=row['phase'],rows=[]))
        runs[-1]['rows'].append(row)
    result=dict(metadata=[s for s in lines if s.startswith('#')],runs=[])
    for run in runs:
        if not run['phase'].startswith('lens-'):continue
        frames={r['frame'] for r in run['rows']}
        rendered=[r for r in run['rows'] if r['presented']=='1']
        times=sorted(float(r['time_s']) for r in rows if r['kind']=='present' and r['frame'] in frames and r['presented']=='1')
        intervals=[(b-a)*1000 for a,b in zip(times,times[1:])]
        entry=dict(phase=run['phase'],rendered_frames=len(rendered),presented_frames=len(times),
            fps=(len(times)-1)/(times[-1]-times[0]) if len(times)>1 else None,
            interval_ms=distribution(intervals),thermal=sorted({int(r['thermal']) for r in run['rows']}))
        for key in ['total_ms','layout_ms','render_ms','submit_ms','acquire_ms']:
            entry[key]=distribution([float(r[key]) for r in rendered])
        for key in ['library_compiles','pipeline_compiles']:
            values=[int(r[key]) for r in rendered if r.get(key)]
            entry[key]=[min(values),max(values)] if values else []
        entry['gpu_ms']=distribution([float(r['gpu_ms']) for r in rows if r['kind']=='gpu' and r['frame'] in frames and r['gpu_ms']])
        result['runs'].append(entry)
    return result
if __name__=='__main__':print(json.dumps(analyze(sys.argv[1]),indent=2))
