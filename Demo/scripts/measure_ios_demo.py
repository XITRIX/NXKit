#!/usr/bin/env python3
"""Run a finite real-device demo replay and export frame metrics over console.

Requires the optimized instrumented Demo installed on an unlocked iPhone.
Always restores ordinary Demo launch, including when capture/export fails.
"""
import argparse
import json
from pathlib import Path
import subprocess
import sys
from analyze_frame_metrics import analyze


def run(device, output, mode='full', uncapped=False, scroll_seconds=12, gpu_passes=False, orientation='portrait', require_archive=False, unbounded_lens=False, full_lens_source=False, metal_harvest=None, original_clear_load=False, raster_paths=False, idle_resume=False, disable_idle_cleanup=False):
    output=Path(output)
    output.mkdir(parents=True,exist_ok=True)
    base=['xcrun','devicectl','device','process','launch','--device',device,'--terminate-existing']
    env={'MTL_DEBUG_LAYER':'0','NXKIT_FRAME_METRICS':'1','NXKIT_FRAME_METRICS_CONSOLE':'1',
         'NXKIT_IOS_DEMO_REPLAY':'1','NXKIT_UNCAPPED':str(int(uncapped)),
         'NXKIT_IOS_DEMO_IDLE_RESUME':str(int(idle_resume)),'NXKIT_DISABLE_IDLE_GPU_CLEANUP':str(int(disable_idle_cleanup)),'NXKIT_RASTER_PATHS':str(int(raster_paths)),'NXKIT_CONTINUOUS_RENDERING':'0','NXKIT_IOS_DEMO_NO_GLASS':str(int(mode in ('no-glass','no-effects'))),
         'NXKIT_IOS_DEMO_SCROLL_SECONDS':str(scroll_seconds),
         'NXKIT_GPU_PASS_METRICS':str(int(gpu_passes)),
         'NXKIT_UNBOUNDED_LENS':str(int(unbounded_lens)),
         'NXKIT_FULL_LENS_SOURCE':str(int(full_lens_source)),
         'SKIA_DISABLE_KNOWN_CLEAR_MSAA_LOAD':str(int(original_clear_load)),
         'SKIA_METAL_CLEAR_LOAD_DIAGNOSTICS':'1',
         'SDL_ORIENTATIONS':('Portrait' if orientation=='portrait' else 'LandscapeLeft LandscapeRight'),
         'NXKIT_METAL_REQUIRE_ARCHIVE_HIT':str(int(require_archive)),
         'NXKIT_METAL_BUNDLED_ONLY':str(int(require_archive)),
         'NXKIT_IOS_DEMO_NO_POCKETS':str(int(mode in ('no-pockets','no-effects')))}
    if metal_harvest:
        env['NXKIT_METAL_HARVEST_DIR']=metal_harvest
        env['NXKIT_METAL_DIAGNOSTICS']='1'
        env['NXKIT_METAL_BUNDLED_ONLY']='1'
    (output/'configuration.json').write_text(json.dumps({'device':device,'mode':mode,'environment':env},indent=2)+'\n')
    # CoreDevice launch and cold archive preparation precede the replay's
    # clock. Leave bounded startup time instead of truncating a valid replay
    # before its marked export on a cold device connection.
    timeout=120+scroll_seconds+(27 if idle_resume else 0)
    command=base+['--console','--environment-variables',json.dumps(env),'--timeout',str(timeout),'ru.xitrix.skiatest']
    try:
        with (output/'console.log').open('w') as log:
            try:
                result=subprocess.run(command,stdout=log,stderr=subprocess.STDOUT,timeout=timeout+10)
                exit_code=result.returncode
            except subprocess.TimeoutExpired:
                exit_code='host-timeout'
        text=(output/'console.log').read_text(errors='replace')
        begin='NXKIT_FRAME_METRICS_BEGIN\n'
        end='NXKIT_FRAME_METRICS_END'
        if begin not in text or end not in text.split(begin,1)[1]:
            raise RuntimeError(f'No complete device metrics export (capture exit {exit_code}); see {output}/console.log')
        csv=text.split(begin,1)[1].split(end,1)[0]
        path=output/'frames.csv'
        path.write_text(csv)
        summary=analyze(path)
        if summary['pipeline_failures']:
            raise RuntimeError('Metal pipeline failures in the replay: '+str(summary['pipeline_failures']))
        required={'scroll-first','navigation','scroll-second','idle'}
        if idle_resume:required.add('scroll-resume')
        if not required.issubset(summary['phases']) or any(summary['phases'].get(phase,{}).get('rendered_frames',0)==0 for phase in required-{'idle'}):
            raise RuntimeError('Incomplete replay phases: '+str(summary['phases'].keys()))
        summary['capture_exit']=exit_code
        summary['capture_note']='CoreDevice console time-limit exit is expected after the complete marked export.'
        (output/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
        for phase in ('scroll-first','navigation','scroll-second')+(('scroll-resume',) if idle_resume else ()):
            data=summary['phases'][phase]
            print(phase, 'FPS:',round(data['presented_fps'],2),
                  'Frame wall p50 ms:',round(data['total_ms']['p50'],2),
                  'GPU CB p50 ms:',round(data['gpu_command_buffer_ms']['p50'],2) if data['gpu_command_buffer_ms'] else 'unavailable')
    finally:
        restored=False
        logs=[]
        for attempt in range(2):
            try:
                result=subprocess.run(base+['--environment-variables','{"MTL_DEBUG_LAYER":"0"}',
                                            '--timeout','20','--json-output',str(output/'restore.json'),'ru.xitrix.skiatest'],
                                      stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,timeout=30)
                logs.append(f'Attempt {attempt+1}:\n'+result.stdout)
                if result.returncode==0:
                    restored=True
                    break
            except subprocess.TimeoutExpired as error:
                logs.append(f'Attempt {attempt+1}: {error}\n')
        (output/'restore.log').write_text('\n'.join(logs))
        if not restored:
            raise RuntimeError('Ordinary launch could not be restored; see '+str(output/'restore.log'))



if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--device',required=True)
    parser.add_argument('--output',required=True)
    parser.add_argument('--mode',choices=('full','no-glass','no-pockets','no-effects'),default='full')
    parser.add_argument('--uncapped',action='store_true')
    parser.add_argument('--scroll-seconds',type=int,default=12,choices=range(12,601),metavar='12..600',
                        help='Length of the second scrolling phase for sustained device measurements')
    parser.add_argument('--gpu-passes',action='store_true',help='Sparse GPU stage counters; use separately from clean frame measurements')
    parser.add_argument('--orientation',choices=('portrait','landscape'),default='portrait')
    parser.add_argument('--unbounded-lens',action='store_true',help='Diagnostic original lens compositing control')
    parser.add_argument('--full-lens-source',action='store_true',help='Diagnostic original full picture source control')
    parser.add_argument('--metal-harvest',help='Absolute output directory inside the device app sandbox; capture missing native variants')
    parser.add_argument('--disable-idle-cleanup',action='store_true',help='Diagnostic control for cache-reclamation/resume comparison')
    parser.add_argument('--idle-resume',action='store_true',help='Check cache cleanup during 15 seconds idle followed by scrolling')
    parser.add_argument('--raster-paths',action='store_true',help='Diagnostic Graphite raster path antialiasing control')
    parser.add_argument('--original-clear-load',action='store_true',help='Diagnostic original Metal MSAA texture-load control')
    parser.add_argument('--require-archive',action='store_true',help='Validate bundled-only native pipelines with runtime compilation disabled')
    args=parser.parse_args()
    try:
        run(args.device,args.output,args.mode,args.uncapped,args.scroll_seconds,args.gpu_passes,args.orientation,args.require_archive,args.unbounded_lens,args.full_lens_source,args.metal_harvest,args.original_clear_load,args.raster_paths,args.idle_resume,args.disable_idle_cleanup)
    except (RuntimeError,subprocess.SubprocessError) as error:
        print(str(error),file=sys.stderr)
        sys.exit(1)
