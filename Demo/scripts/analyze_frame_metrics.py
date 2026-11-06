#!/usr/bin/env python3
"""Summarize real drawable presentation intervals, CPU work and Metal timings.

GPU rows are asynchronous command-buffer execution intervals, not submission
duration. Presentation FPS uses elapsed presentedTime, never 1/CPU time.
"""
import argparse
import collections
import csv
import json
import math
from pathlib import Path


def distribution(values):
    values = sorted(values)
    if not values:
        return None
    def percentile(p):
        index = (len(values) - 1) * p
        lo = math.floor(index)
        hi = math.ceil(index)
        return values[lo] + (values[hi] - values[lo]) * (index - lo)
    return dict(count=len(values), mean=sum(values)/len(values),
                p50=percentile(.5), p95=percentile(.95), p99=percentile(.99), max=values[-1])


def analyze(path):
    lines = Path(path).read_text().splitlines()
    metadata = [line[2:] for line in lines if line.startswith('# ')]
    rows = list(csv.DictReader(line for line in lines if not line.startswith('#')))
    cpu = {int(r['frame']): r for r in rows if r['kind'] == 'cpu'}
    result = {'metadata': metadata, 'phases': {}}
    result['pipeline_failures']=max((int(line.split('=',1)[1]) for line in metadata
                                    if line.startswith('pipeline_failures=')),default=0)
    def summarize(frames):
        rendered = [r for r in frames.values() if r['presented'] == '1']
        presents = sorted(float(r['time_s']) for r in rows if r['kind'] == 'present'
                          and int(r['frame']) in frames and r['presented'] == '1'
                          and float(r['time_s']) > 0)
        submissions = sorted(float(r['time_s']) for r in rendered)
        intervals = [(b-a)*1000 for a,b in zip(presents,presents[1:])]
        gpu = []
        for r in rows:
            if r['kind']=='gpu' and int(r['frame']) in frames and r['presented']=='1':
                gpu.append(float(r['gpu_ms']))
        phase_result = dict(rendered_frames=len(rendered), presented_frames=len(presents),
                            submitted_fps=(len(submissions)-1)/(submissions[-1]-submissions[0]) if len(submissions)>1 else None,
                            presented_fps=(len(presents)-1)/(presents[-1]-presents[0]) if len(presents)>1 else None,
                            presentation_interval_ms=distribution(intervals),
                            gpu_command_buffer_ms=distribution(gpu),
                            thermal_counts=dict(collections.Counter(r['thermal'] for r in frames.values())))
        for field in ('total_ms','layout_ms','render_ms','submit_ms','acquire_ms'):
            phase_result[field] = distribution([float(r[field]) for r in rendered])
        for counter in ('library_compiles','pipeline_compiles','archive_hits'):
            counts=[int(r[counter]) for r in rendered if r.get(counter)]
            if counts:phase_result[counter]=dict(first=min(counts),last=max(counts),increase=max(counts)-min(counts))
        work = {}
        for counter in ('bounded_masks','isolated_masks','preserved_mask_pixels',
                        'bounded_lenses','bounded_lens_groups','cropped_lens_sources','lens_source_pixels','uniform_mask_prefixes'):
            counts = [int(r[counter]) for r in rendered if r.get(counter)]
            if counts:
                work[counter] = distribution(counts)
        if work:
            phase_result['render_counters'] = work
        # Tolerate floating point/timebase rounding at a display interval boundary.
        phase_result['intervals_over_8_4_ms'] = sum(x > 8.4 for x in intervals)
        phase_result['intervals_over_16_8_ms'] = sum(x > 16.8 for x in intervals)
        return phase_result
    for phase in sorted({r['phase'] for r in cpu.values()}):
        frames = {frame: r for frame,r in cpu.items() if r['phase'] == phase}
        result['phases'][phase] = summarize(frames)
    sustained = {frame:r for frame,r in cpu.items() if r['phase']=='scroll-second'}
    result['sustained_windows_30s'] = []
    if sustained:
        start = min(float(r['time_s']) for r in sustained.values())
        windows = collections.defaultdict(dict)
        for frame,r in sustained.items():
            windows[int((float(r['time_s'])-start)//30)][frame] = r
        for index,frames in sorted(windows.items()):
            if len(frames)>1:
                result['sustained_windows_30s'].append(dict(start_seconds=index*30,**summarize(frames)))
    return result


if __name__ == '__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('csv')
    args=parser.parse_args()
    print(json.dumps(analyze(args.csv),indent=2))
