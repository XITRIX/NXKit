"""One bounded native rest-fill ablation, with passive native video and per-phase layer snapshots."""
from pathlib import Path
import json, shutil, signal, subprocess, sys, time

device, output = sys.argv[1], Path(sys.argv[2]).resolve()
output.mkdir(parents=True, exist_ok=True)
source = Path(__file__).resolve().parent
cases = json.loads((source / 'lift-fill-ablation-cases.json').read_text())
start_time = time.time()
if '--reuse-build' in sys.argv:
    project = output / 'project'
    command = ['xcodebuild', '-project', str(project / 'Interaction.xcodeproj'), '-scheme',
               'Interaction', '-destination', f'platform=iOS Simulator,id={device}',
               '-derivedDataPath', str(project / 'build'), '-parallel-testing-enabled',
               'NO', 'test-without-building']
else:
    command = [sys.executable, str(source / 'capture-lift.py'), device,
               str(output), str(source / 'lift-fill-ablation-cases.json')]
video = subprocess.Popen(['xcrun', 'simctl', 'io', device, 'recordVideo', '--codec=h264',
                          '--force', str(output / 'native-ablation.mp4')],
                         stdout=(output / 'video.log').open('w'), stderr=subprocess.STDOUT)
process = subprocess.Popen(command,
                           stdout=(output / 'capture-driver.log').open('w'), stderr=subprocess.STDOUT)
try:
    process.wait()
finally:
    video.send_signal(signal.SIGINT)
    video.wait(timeout=30)
containers = Path.home() / 'Library/Developer/CoreSimulator/Devices' / device / 'data/Containers/Data/Application'
for case in cases:
    latest = max(containers.glob('*/Documents/' + case['id']), key=lambda path: (path / 'trace.json').stat().st_mtime)
    shutil.copytree(latest, output / case['id'], dirs_exist_ok=True)
(output / 'video-provenance.json').write_text(json.dumps({'testExitCode': process.returncode,
    'capture': 'Passive simctl recordVideo; phase times and markerIDs in native traces',
    'startHostUnix': start_time, 'cases': cases}, indent=2) + '\n')
sys.exit(process.returncode)
