"""Capture public UIKit state driven by XCTest OS touches.
Usage: capture-native.py SIMULATOR OUTPUT [CASES] [--collect-only]
"""
from pathlib import Path
import sys,subprocess,shutil,json,hashlib
source=Path(__file__).resolve().parent;device=sys.argv[1];out=Path(sys.argv[2]).resolve();out.mkdir(parents=True,exist_ok=True)
cases=Path(sys.argv[3]) if len(sys.argv)>3 and not sys.argv[3].startswith('--') else source/'cases.json';project=out/'project'
if '--collect-only' not in sys.argv:
 subprocess.run([sys.executable,str(source/'build-native.py'),str(project),str(cases)],check=True)
 command=['xcodebuild','-project',str(project/'Interaction.xcodeproj'),'-scheme','Interaction','-destination',f'platform=iOS Simulator,id={device}','-derivedDataPath',str(project/'build'),'-parallel-testing-enabled','NO','test']
 with (out/'test.log').open('w') as log:subprocess.run(command,check=True,stdout=log,stderr=subprocess.STDOUT)
container=Path(subprocess.check_output(['xcrun','simctl','get_app_container',device,'org.nxkit.ScrollReference','data'],text=True).strip());native=out/'native';native.mkdir(exist_ok=True)
for c in json.loads(cases.read_text()):shutil.copy2(container/'Documents'/(c['id']+'.json'),native/(c['id']+'.json'))
# Retain the exact sources used to create the project, not a later working tree.
files=['ScrollReference.m','ScrollUITests.m','build-native.py']
for f in files:
 snapshot=project/(f+'.captured')
 if not snapshot.exists():raise RuntimeError('Missing capture source snapshot: '+str(snapshot))
 shutil.copy2(snapshot,out/(f+'.captured'))
shutil.copy2(project/'Cases.json',out/'cases.json.captured')
(out/'native-manifest.json').write_text(json.dumps({'evidence':'Public UIKit state, actual OS touches through XCTest; the test runner uses private XCTest event-path plumbing. No private UIKit physics or production API calls.','sourceSHA256':{f:hashlib.sha256((out/(f+'.captured')).read_bytes()).hexdigest() for f in files},'casesSHA256':hashlib.sha256((project/'Cases.json').read_bytes()).hexdigest()},indent=2)+'\n')
