"""Build and exercise the native UIKit oracle using public XCUITest touches."""
from pathlib import Path
import hashlib,json,shutil,subprocess,sys,time
from summarize import summarize
source=Path(__file__).resolve().parent;device=sys.argv[1];output=Path(sys.argv[2]).resolve();output.mkdir(parents=True,exist_ok=True)
digests={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in source.glob('*') if p.is_file()}
start=time.time()
(output/'fixture-snapshot').mkdir(exist_ok=True)
for p in source.glob('*'):
 if p.is_file():shutil.copy2(p,output/'fixture-snapshot'/p.name)
project=output/'project';subprocess.run([sys.executable,str(source/'build.py'),str(project)]+(['--primary'] if '--primary' in sys.argv else []),check=True)
cmd=['xcodebuild','-project',str(project/'Interaction.xcodeproj'),'-scheme','Interaction','-destination',f'platform=iOS Simulator,id={device}','-derivedDataPath',str(project/'build'),'-parallel-testing-enabled','NO','test']
with (output/'test.log').open('w') as log: result=subprocess.run(cmd,stdout=log,stderr=subprocess.STDOUT)
container=Path(subprocess.check_output(['xcrun','simctl','get_app_container',device,'org.nxkit.ControlScrollReference','data'],text=True).strip())
for p in (container/'Documents').glob('*'):
 if p.is_dir() and (p/'trace.json').exists() and (p/'trace.json').stat().st_mtime>=start:shutil.copytree(p,output/p.name,dirs_exist_ok=True)
manifest={'startedUnix':start,'subset':'primary-ten' if '--primary' in sys.argv else 'full-28','device':device,'exitCode':result.returncode,'sourceSHA256':digests,'scope':'Native UISlider/UISwitch with actual XCTest OS touches, vertical/horizontal UIScrollView and outside baseline. No synthetic UITouch or recognizer policy changes.'}
summary=summarize(output)
manifest['capturedCases']=len(summary)
(output/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
(output/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n');print(json.dumps({'manifest':manifest,'observedResults':summary}));sys.exit(result.returncode)
