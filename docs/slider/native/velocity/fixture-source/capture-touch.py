"""Run public XCUITest press/drag paths, retaining UIKit touch/value/lens output."""
from pathlib import Path
import hashlib,json,shutil,subprocess,sys
device=sys.argv[1];output=Path(sys.argv[2]).resolve();output.mkdir(parents=True,exist_ok=True)
source=Path(__file__).resolve().parent
project=output/'project'
subprocess.run([sys.executable,str(source/'build-touch.py'),str(project)],check=True)
command=['xcodebuild','-project',str(project/'Interaction.xcodeproj'),'-scheme','Interaction','-destination',f'platform=iOS Simulator,id={device}','-derivedDataPath',str(project/'build'),'-parallel-testing-enabled','NO','test']
names=['thumb-center','thumb-offset','track-right','track-right-drag','noncontinuous']
if '--calibration' in sys.argv:
    command+=['-only-testing:InteractionUITests/SliderUITests/testTrackingCalibration','-only-testing:InteractionUITests/SliderUITests/testNativeSnapshots']
    names=['zero','slow','fast','offset-slow','thumb-snapshots','thumb-snapshots-dark']
if '--velocity' in sys.argv:
    command+=['-only-testing:InteractionUITests/SliderUITests/testVelocityAndCustom']
    names=['velocity-200','velocity-400','custom-thumb']
with (output/'test.log').open('w') as log: result=subprocess.run(command,stdout=log,stderr=subprocess.STDOUT)
container=Path(subprocess.check_output(['xcrun','simctl','get_app_container',device,'org.nxkit.SliderReference','data'],text=True).strip())
for name in names:
    shutil.copytree(container/'Documents'/name,output/name,dirs_exist_ok=True)
manifest={'device':device,'exitCode':result.returncode,'sourceSHA256':{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in [source/'SliderReference.m',source/'SliderUITests.m',source/'build-touch.py',source/'capture-touch.py']},'coverage':'Actual native UIKit slider, public XCUITest-delivered OS touches; display-link and touch/value logs; synthetic UITouch not used'}
(output/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
print(json.dumps(manifest))
sys.exit(result.returncode)
