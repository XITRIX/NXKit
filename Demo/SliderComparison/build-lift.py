"""Build an isolated, snapshot-sourced native stationary-contact oracle."""
from pathlib import Path
import hashlib,json,plistlib,runpy,shutil,sys
source=Path(__file__).resolve().parent;output=Path(sys.argv[1]).resolve();cases=Path(sys.argv[2]).resolve();output.mkdir(parents=True,exist_ok=True)
sys.argv=[str(source.parent/'GlassComparison/build-interaction.py'),str(output),str(cases)];runpy.run_path(sys.argv[0],run_name='__main__')
for name in ['LiftReference.m','LiftUITests.m']:shutil.copy2(source/name,output/name)
project=output/'Interaction.xcodeproj/project.pbxproj';text=project.read_text().replace(str(source.parent/'GlassComparison/InteractionReference.m'),str(output/'LiftReference.m')).replace(str(source.parent/'GlassComparison/InteractionUITests.m'),str(output/'LiftUITests.m')).replace('org.nxkit.GlassInteractionReference','org.nxkit.SliderLiftReference').replace('org.nxkit.GlassInteractionUITests','org.nxkit.SliderLiftUITests');project.write_text(text)
info=plistlib.loads((output/'Info.plist').read_bytes());info['CADisableMinimumFrameDurationOnPhone']=True;(output/'Info.plist').write_bytes(plistlib.dumps(info))
(output/'source-manifest.json').write_text(json.dumps({name:hashlib.sha256((output/name).read_bytes()).hexdigest() for name in ['LiftReference.m','LiftUITests.m']},indent=2)+'\n')
