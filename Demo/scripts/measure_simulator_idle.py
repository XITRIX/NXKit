#!/usr/bin/env python3
"""Measure settled app CPU on Simulator, without frame diagnostics or a debugger."""
import argparse
import json
import os
from pathlib import Path
import plistlib
import re
import subprocess
import time


def cpu_seconds(pid):
    value = subprocess.check_output(['ps', '-p', str(pid), '-o', 'time='], text=True).strip()
    minutes, seconds = value.split(':')
    return int(minutes) * 60 + float(seconds)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--simulator', required=True)
    parser.add_argument('--app', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    parser.add_argument('--screen', choices=('menu', 'fullscreen'), default='fullscreen')
    parser.add_argument('--seconds', type=float, default=20)
    parser.add_argument('--samples', type=int, default=3)
    args = parser.parse_args()
    if args.seconds < 10 or args.samples < 1:
        parser.error('Use at least ten seconds per sample and one sample')
    args.output.mkdir(parents=True, exist_ok=True)
    bundle = plistlib.loads((args.app / 'Info.plist').read_bytes())['CFBundleIdentifier']
    settings = {'MTL_DEBUG_LAYER': '0', 'NXKIT_FRAME_METRICS': '0', 'NXKIT_UNCAPPED': '0',
                'NXKIT_CONTINUOUS_RENDERING': '0', 'NXKIT_IOS_DEMO_LENS_REPLAY': '0',
                'NXKIT_IOS_DEMO_REPLAY': str(int(args.screen == 'fullscreen')),
                'NXKIT_IOS_DEMO_SCROLL_SECONDS': '12', 'NXKIT_IOS_DEMO_IDLE_RESUME': '0'}
    environment = os.environ | {'SIMCTL_CHILD_' + k: v for k, v in settings.items()}
    base = ['xcrun', 'simctl']
    subprocess.run(base + ['install', args.simulator, str(args.app.resolve())], check=True, timeout=60)
    launch = subprocess.check_output(base + ['launch', '--terminate-running-process', args.simulator, bundle],
                                     env=environment, text=True, timeout=30)
    pid = int(re.search(r':\s*(\d+)\s*$', launch).group(1))
    (args.output / 'launch.log').write_text(launch)
    # The fullscreen replay ends at 42 seconds; exclude all startup and motion.
    time.sleep(55 if args.screen == 'fullscreen' else 15)
    samples = []
    for _ in range(args.samples):
        before = cpu_seconds(pid)
        start = time.monotonic()
        time.sleep(args.seconds)
        after = cpu_seconds(pid)
        wall = time.monotonic() - start
        row = {'wall_seconds': wall, 'cpu_seconds': after - before,
               'cpu_percent': 100 * (after - before) / wall}
        samples.append(row)
        print(json.dumps(row), flush=True)
    result = {'app': str(args.app.resolve()), 'simulator': args.simulator, 'screen': args.screen,
              'pid': pid, 'settings': settings, 'samples': samples,
              'cpu_percent': 100 * sum(s['cpu_seconds'] for s in samples) / sum(s['wall_seconds'] for s in samples),
              'units': 'Host process CPU; 100% is one host CPU core. Not physical iPhone power or CPU.'}
    (args.output / 'cpu.json').write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
