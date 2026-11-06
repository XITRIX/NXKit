"""Build/run the native UIKit touch oracle and retain its raw traces.

Usage: python capture-lens-motion.py DEVICE OUTPUT CASES_JSON [--video]
Requires a booted, dedicated simulator; it never resets simulator services.
XCTest supplies OS touch events. Native model/presentation geometry is output,
not a prescribed approximation. The optional video is illustrative: its clock
is not synchronized to UITouch.timestamp. Use trace.json for timed measurements.
"""
from pathlib import Path
import hashlib
import json
import shutil
import signal
import subprocess
import sys

device, output, cases_path = sys.argv[1], Path(sys.argv[2]).resolve(), Path(sys.argv[3]).resolve()
output.mkdir(parents=True, exist_ok=True)
source = Path(__file__).resolve().parent
cases = json.loads(cases_path.read_text())
project_output = output / 'gesture'
subprocess.run([sys.executable, str(source/'build-lens-motion.py'), str(project_output), str(cases_path)], check=True)
command = ['xcodebuild', '-project', str(project_output/'Interaction.xcodeproj'), '-scheme', 'Interaction',
           '-destination', f'platform=iOS Simulator,id={device}', '-derivedDataPath', str(project_output/'build'),
           '-parallel-testing-enabled', 'NO']
with (output/'build.log').open('w') as log:
    subprocess.run(command+['build-for-testing'], stdout=log, stderr=subprocess.STDOUT, check=True)
video = None
if '--video' in sys.argv[4:]:
    video = subprocess.Popen(['xcrun','simctl','io',device,'recordVideo','--codec=h264','--force',str(output/'native-motion.mp4')])
try:
    with (output/'test.log').open('w') as log:
        result = subprocess.run(command+['test-without-building'], stdout=log, stderr=subprocess.STDOUT)
finally:
    if video:
        video.send_signal(signal.SIGINT)
        video.wait(timeout=30)
documents = Path(subprocess.check_output(['xcrun','simctl','get_app_container',device,'org.nxkit.LensMotionReference','data'],text=True).strip())/'Documents'
captured=[]
for case in cases:
    name='motion-'+case['id']
    if (documents/name).exists():
        shutil.copytree(documents/name,output/name,dirs_exist_ok=True)
        captured.append(case['id'])
files=['LensReference.m','LensMotionUITests.m','build-lens-motion.py','capture-lens-motion.py']
hashes={name:hashlib.sha256((source/name).read_bytes()).hexdigest() for name in files}
for name in files:
    shutil.copy2(source/name,output/(name+'.captured'))
(output/'manifest.json').write_text(json.dumps(dict(device=device,cases=cases,captured=captured,sourceSHA256=hashes,
    testExitCode=result.returncode,evidence='Native UIKit controls responding to OS touch events delivered by XCTest',
    videoClock='Illustrative only; not synchronized to UITouch.timestamp'),indent=2)+'\n')
sys.exit(result.returncode)
