#!/usr/bin/env python3
"""Finite full-screen replay on iOS Simulator; never contacts a physical device."""
import argparse
import json
import os
from pathlib import Path
import plistlib
import subprocess
import time
from analyze_frame_metrics import analyze


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--simulator',required=True)
    parser.add_argument('--app',required=True,type=Path)
    parser.add_argument('--output',required=True,type=Path)
    parser.add_argument('--scroll-seconds',type=int,default=12,choices=range(12,601),metavar='12..600')
    parser.add_argument('--idle-resume',action='store_true',help='Check idle cache reclamation and the next scrolling gesture')
    parser.add_argument('--disable-idle-cleanup',action='store_true',help='Diagnostic control for the idle/resume comparison')
    args=parser.parse_args()
    args.output.mkdir(parents=True,exist_ok=True)
    bundle=plistlib.loads((args.app/'Info.plist').read_bytes())['CFBundleIdentifier']
    base=['xcrun','simctl']
    subprocess.run(base+['install',args.simulator,str(args.app)],check=True,timeout=60)
    settings={'MTL_DEBUG_LAYER':'0','NXKIT_FRAME_METRICS':'1','NXKIT_IOS_DEMO_REPLAY':'1',
              'NXKIT_IOS_DEMO_SCROLL_SECONDS':str(args.scroll_seconds),
              'NXKIT_IOS_DEMO_IDLE_RESUME':str(int(args.idle_resume)),
              'NXKIT_DISABLE_IDLE_GPU_CLEANUP':str(int(args.disable_idle_cleanup)),
              'NXKIT_UNCAPPED':'0','NXKIT_CONTINUOUS_RENDERING':'0'}
    environment=os.environ|{'SIMCTL_CHILD_'+k:v for k,v in settings.items()}
    try:
        with (args.output/'launch.log').open('w')as log:
            subprocess.run(base+['launch','--terminate-running-process',args.simulator,bundle],
                           env=environment,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=30)
        time.sleep(46)
        container=Path(subprocess.check_output(base+['get_app_container',args.simulator,bundle,'data'],text=True,timeout=20).strip())
        path=args.output/'frames.csv'
        expected={'scroll-first','navigation','scroll-second','idle'}
        if args.idle_resume:expected.add('scroll-resume')
        # launch returns before application/shader preparation completes.
        # Wait for the replay's marker, with a bound, rather than mistaking a
        # slightly later startup for missing scroll/navigation behavior.
        deadline=time.monotonic()+30+(args.scroll_seconds-12)+(27 if args.idle_resume else 0)
        while True:
            data=(container/'Documents/frame-metrics.csv').read_bytes()
            path.write_bytes(data[:data.rfind(b'\n')+1])
            result=analyze(path)
            complete=expected.issubset(result['phases'])
            moving=expected-{'idle'}
            if complete and all(result['phases'][phase]['rendered_frames']>0 for phase in moving):break
            if time.monotonic()>=deadline:raise RuntimeError('Incomplete simulator replay: missing or unrendered motion phase')
            time.sleep(1)
        result['platform']='iOS Simulator; host GPU timings do not establish physical iPhone FPS or thermal behavior'
        (args.output/'summary.json').write_text(json.dumps(result,indent=2)+'\n')
        for phase in ('scroll-first','navigation','scroll-second')+(('scroll-resume',) if args.idle_resume else ()):
            row=result['phases'][phase]
            print(phase,'submission rate:',row['submitted_fps'],'frame wall p50:',row['total_ms']['p50'])
    finally:
        with (args.output/'restore.log').open('w') as log:
            subprocess.run(base+['launch','--terminate-running-process',args.simulator,bundle],
                           env=os.environ|{'SIMCTL_CHILD_MTL_DEBUG_LAYER':'0'},check=True,timeout=30,
                           stdout=log,stderr=subprocess.STDOUT)


if __name__=='__main__':main()
