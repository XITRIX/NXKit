#!/usr/bin/env python3
"""Apply NXKit's small Graphite/Metal integration patch without resetting Skia."""
from pathlib import Path
import subprocess

root=Path(__file__).resolve().parents[2]
skia=root/'Submodules/skia'
patch=Path(__file__).with_name('patches')/'skia-metal-pipeline-cache.patch'
command=['git','-C',str(skia),'apply']
if subprocess.run(command+['--reverse','--check',str(patch)],capture_output=True).returncode==0:
    print('NXKit Metal cache patch is already applied')
else:
    subprocess.run(command+['--check',str(patch)],check=True)
    subprocess.run(command+[str(patch)],check=True)
    print('Applied NXKit Metal cache patch; rebuild Skia archives before linking NXKit')
