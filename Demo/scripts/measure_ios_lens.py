#!/usr/bin/env python3
"""Measure a finite live tab-lens replay on an installed optimized iPhone Demo.

All cases use normal dirty rendering and pacing. --original-inputs is a
rendering control, not an application appearance option. Always restore a
normal launch after capture, including when CoreDevice disconnects.
"""
import argparse, json, subprocess
from pathlib import Path
from analyze_lens_metrics import analyze

def run(args):
    out=Path(args.output);out.mkdir(parents=True,exist_ok=True)
    env={'MTL_DEBUG_LAYER':'0','NXKIT_FRAME_METRICS':'1','NXKIT_FRAME_METRICS_CONSOLE':'1',
         'NXKIT_IOS_DEMO_REPLAY':'1','NXKIT_IOS_DEMO_LENS_REPLAY':'1',
         'NXKIT_IOS_DEMO_LENS_SECONDS':str(args.seconds),'NXKIT_CONTINUOUS_RENDERING':'0','NXKIT_UNCAPPED':'0',
         'NXKIT_LENS_FILTER_FOREGROUND':str(int(args.original_inputs)),
         'NXKIT_LENS_SURFACE_INPUTS':str(int(args.original_inputs)),
         'NXKIT_METAL_BUNDLED_ONLY':str(int(args.require_archive)),
         'NXKIT_METAL_REQUIRE_ARCHIVE_HIT':str(int(args.require_archive)),
         'NXKIT_GPU_PASS_METRICS':str(int(args.gpu_passes))}
    if args.harvest:env['NXKIT_METAL_HARVEST_DIR']=args.harvest
    (out/'configuration.json').write_text(json.dumps(dict(device=args.device,environment=env),indent=2)+'\n')
    base=['xcrun','devicectl','device','process','launch','--device',args.device,'--terminate-existing']
    try:
        timeout=args.seconds+75
        with (out/'console.log').open('w') as log:
            try:result=subprocess.run(base+['--console','--environment-variables',json.dumps(env),'--timeout',str(timeout),'ru.xitrix.skiatest'],stdout=log,stderr=subprocess.STDOUT,timeout=timeout+10);exitcode=result.returncode
            except subprocess.TimeoutExpired:exitcode='host-watchdog'
        text=(out/'console.log').read_text(errors='replace')
        begin='NXKIT_FRAME_METRICS_BEGIN\n';end='NXKIT_FRAME_METRICS_END'
        if begin not in text or end not in text.split(begin,1)[1]:raise RuntimeError('Missing complete console metrics export: '+str(out/'console.log'))
        csv=text.split(begin,1)[1].split(end,1)[0];(out/'frames.csv').write_text(csv)
        summary=analyze(out/'frames.csv');summary['capture_exit']=exitcode
        runs=summary['runs'];drags=[r for r in runs if r['phase']=='lens-drag']
        if len(drags)!=args.seconds//10 or any(r['presented_frames']<100 for r in drags):raise RuntimeError('Incomplete lens-drag replay')
        if not any(r['phase']=='lens-idle' for r in runs):raise RuntimeError('Missing terminal idle')
        if any(s.startswith('# pipeline_failures=') and not s.endswith('=0') for s in summary['metadata']):raise RuntimeError('Metal pipeline failure')
        (out/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
        for i,r in enumerate(drags):print('drag',i,'FPS',round(r['fps'],2),'GPU p50',round(r['gpu_ms']['p50'],3),'render p50',round(r['render_ms']['p50'],3),'thermal',r['thermal'])
    finally:
        with (out/'restore.log').open('w') as log:
            subprocess.run(base+['--environment-variables','{"MTL_DEBUG_LAYER":"0"}','--timeout','20','ru.xitrix.skiatest'],stdout=log,stderr=subprocess.STDOUT,timeout=30,check=True)
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--device',required=True);p.add_argument('--output',required=True)
    p.add_argument('--seconds',type=int,choices=range(10,601,10),default=60)
    p.add_argument('--original-inputs',action='store_true');p.add_argument('--require-archive',action='store_true')
    p.add_argument('--gpu-passes',action='store_true');p.add_argument('--harvest')
    run(p.parse_args())
