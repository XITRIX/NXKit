"""Capture actual native stationary slider contacts; optionally record passive video."""
from pathlib import Path
import hashlib,json,shutil,signal,subprocess,sys
device,output,cases=sys.argv[1],Path(sys.argv[2]).resolve(),Path(sys.argv[3]).resolve();output.mkdir(parents=True,exist_ok=True);source=Path(__file__).resolve().parent;project=output/'project'
subprocess.run([sys.executable,str(source/'build-lift.py'),str(project),str(cases)],check=True)
command=['xcodebuild','-project',str(project/'Interaction.xcodeproj'),'-scheme','Interaction','-destination',f'platform=iOS Simulator,id={device}','-derivedDataPath',str(project/'build'),'-parallel-testing-enabled','NO']
with (output/'build.log').open('w') as log:subprocess.run(command+['build-for-testing'],stdout=log,stderr=subprocess.STDOUT,check=True)
video=None
if '--video' in sys.argv:video=subprocess.Popen(['xcrun','simctl','io',device,'recordVideo','--codec=h264','--force',str(output/'native-motion.mp4')],stdout=(output/'video.log').open('w'),stderr=subprocess.STDOUT)
try:
    with (output/'test.log').open('w') as log:result=subprocess.run(command+['test-without-building'],stdout=log,stderr=subprocess.STDOUT)
finally:
    if video:video.send_signal(signal.SIGINT);video.wait(timeout=30)
container=Path(subprocess.check_output(['xcrun','simctl','get_app_container',device,'org.nxkit.SliderLiftReference','data'],text=True).strip())
captured=[]
for case in json.loads(cases.read_text()):
    if(container/'Documents'/case['id']).exists():shutil.copytree(container/'Documents'/case['id'],output/case['id'],dirs_exist_ok=True);captured.append(case['id'])
manifest={'device':device,'cases':json.loads(cases.read_text()),'captured':captured,'testExitCode':result.returncode,'sourceSHA256':json.loads((project/'source-manifest.json').read_text()),'nativeEvidence':'Actual UIKit slider responds to XCTest OS touches; stationary contact lifecycle, no private setLifted/manual tracking calls','videoClock':'Visual marker correlates display-link frameID; compositor phase can differ by a display interval; encoded video is not native P3 pixel evidence'}
(output/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n');print(json.dumps(manifest));sys.exit(result.returncode)
