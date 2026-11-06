"""Capture the actual UIKit gallery screens and their retained checkpoints.
Usage: python capture-gallery.py DEVICE APP OUTPUT [--baseline-only|--fixed-buttons]
"""
import json
from pathlib import Path
import subprocess
import sys
import time
import re

device,app,output=sys.argv[1:4]
output=Path(output).resolve(); output.mkdir(parents=True,exist_ok=True)
bundle='org.nxkit.NativeGalleryReference'
def simctl(*args):
    return subprocess.run(['xcrun','simctl',*args],check=True,capture_output=True,text=True).stdout.strip()
simctl('install',device,str(Path(app).resolve()))
simctl('launch','--terminate-running-process',device,bundle)
documents=Path(simctl('get_app_container',device,bundle,'data'))/'Documents'
states=[]
spec=Path(__file__).resolve().parents[1]/'app/Screens/GlassDemoViewController/GlassDemoSpec.h'
demo_count=len(re.findall(r'^\s*\{"',spec.read_text(),re.MULTILINE))
fixed_buttons='--fixed-buttons' in sys.argv
for dark in [0,1]:
    for demo in (range(13,demo_count) if fixed_buttons else ([0,1] if "--material-controls" in sys.argv else ([12] if "--two-tints" in sys.argv else range(demo_count)))):
        states.append(dict(case=demo,dark=dark,progress=0,backdrop=4 if demo==11 else 0))
        if fixed_buttons:
            states.append(dict(case=demo,dark=dark,progress=0,backdrop=4))
        elif '--two-tints' in sys.argv:
            for progress in [.25,.5,.75,1]: states.append(dict(case=demo,dark=dark,progress=progress,backdrop=0))
            states.append(dict(case=demo,dark=dark,progress=1,backdrop=0,reverse=1))
            states.append(dict(case=demo,dark=dark,progress=0,backdrop=0,reverse=0))
        elif '--material-controls' in sys.argv:
            for backdrop in [1,2,3]: states.append(dict(case=demo,dark=dark,progress=0,backdrop=backdrop))
        elif '--baseline-only' not in sys.argv:
            if demo in [4,5,6,7,8]:
                for progress in [.5,1,0]: states.append(dict(case=demo,dark=dark,progress=progress,backdrop=0))
            if demo in [2,3]:
                for backdrop in [1,2,3]: states.append(dict(case=demo,dark=dark,progress=0,backdrop=backdrop))
records=[]
for index,state in enumerate(states):
    name=f'{index:03d}-case{state["case"]}-dark{state["dark"]}-p{state["progress"]}-bg{state["backdrop"]}'
    request=dict(id=name,**state)
    staged=documents/'request.tmp'; staged.write_text(json.dumps(request)); staged.replace(documents/'request.json')
    deadline=time.monotonic()+15
    while True:
        try: metadata=json.loads((documents/'gallery.json').read_text())
        except (FileNotFoundError,json.JSONDecodeError): metadata={}
        if metadata.get('request')==request: break
        if time.monotonic()>deadline: raise RuntimeError(f'UIKit did not acknowledge {name}')
        time.sleep(.05)
    time.sleep(.6)
    frame=output/name; frame.mkdir(exist_ok=True)
    simctl('io',device,'screenshot',str(frame/'screen.png'))
    metadata=json.loads((documents/'gallery.json').read_text())
    if metadata['width']!=720: raise RuntimeError('Comparison requires a full-size iPad stage')
    (frame/'capture.json').write_text(json.dumps(metadata,indent=2)+'\n')
    records.append(dict(name=name,**state))
    (output/'series.json').write_text(json.dumps(records,indent=2)+'\n')
    print(name,metadata['os'],flush=True)
