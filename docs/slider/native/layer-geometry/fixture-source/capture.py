"""Capture native defaults/geometry/control dispatch on an already booted Simulator."""
from pathlib import Path
import hashlib,json,shutil,subprocess,sys,time
device=sys.argv[1];output=Path(sys.argv[2]).resolve();output.mkdir(parents=True,exist_ok=True)
source=Path(__file__).resolve().parent
subprocess.run([sys.executable,str(source/'build.py'),str(output/'build')],check=True)
subprocess.run(['xcrun','simctl','install',device,str(output/'build/SliderReference.app')],check=True)
for dark in [False,True]:
    case={'id':'dark' if dark else 'light','dark':dark}
    subprocess.run(['xcrun','simctl','terminate',device,'org.nxkit.SliderReference'],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
    import os
    env=dict(os.environ,SIMCTL_CHILD_SLIDER_CASE=json.dumps(case))
    subprocess.run(['xcrun','simctl','launch',device,'org.nxkit.SliderReference'],check=True,env=env)
    time.sleep(7)
container=Path(subprocess.check_output(['xcrun','simctl','get_app_container',device,'org.nxkit.SliderReference','data'],text=True).strip())
for name in ['light','dark']:shutil.copytree(container/'Documents'/name,output/name,dirs_exist_ok=True)
manifest={'device':device,'sources':{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in [source/'SliderReference.m',source/'build.py',source/'capture.py']},'coverage':'Programmatic native geometry and action dispatch; not OS touch evidence'}
(output/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
print('Captured native UISlider geometry and UIControl dispatch, light and dark')
