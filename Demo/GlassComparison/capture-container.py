"""Capture settled native UIKit container states using a test-owned simulator.
Usage: python capture-container.py DEVICE APP_DIRECTORY OUTPUT_DIRECTORY
The fixture retains its views while spacing/size/appearance change. Real effect
pixels come from simctl's framebuffer, never drawViewHierarchy/UIGraphics.
"""
import json
from pathlib import Path
import subprocess
import sys
import time

device, app, output = sys.argv[1:]
output = Path(output).resolve()
output.mkdir(parents=True, exist_ok=True)
bundle = 'org.nxkit.NativeContainerReference'
def simctl(*args):
    return subprocess.run(['xcrun','simctl',*args], check=True, capture_output=True, text=True).stdout.strip()
simctl('install',device,str(Path(app).resolve()))
simctl('launch','--terminate-running-process',device,bundle)
documents = Path(simctl('get_app_container',device,bundle,'data'))/'Documents'
variants = [('initial',0,1,False), ('near',-6,1,False), ('far',18,1,False),
            ('large',0,1.25,False), ('dark',0,1,True), ('return',0,1,False)]
for name, gap, size, dark in variants:
    request = dict(id=name, gapOffset=gap, sizeScale=size, dark=dark)
    staged = documents/'request.tmp'
    staged.write_text(json.dumps(request))
    staged.replace(documents/'request.json')
    deadline = time.monotonic()+15
    while True:
        try: metadata = json.loads((documents/'container.json').read_text())
        except (FileNotFoundError, json.JSONDecodeError): metadata = {}
        if metadata.get('request') == request: break
        if time.monotonic()>deadline: raise RuntimeError(f'Native fixture did not acknowledge {name}')
        time.sleep(.05)
    time.sleep(.8)
    frame = output/name
    frame.mkdir(exist_ok=True)
    simctl('io',device,'screenshot',str(frame/'screen.png'))
    (frame/'capture.json').write_text(json.dumps(metadata,indent=2)+'\n')
    print(name, metadata['os'], flush=True)
