"""Build-project generator for the public UIKit navigation oracle."""
import runpy,sys,json,plistlib,hashlib
from pathlib import Path
here=Path(__file__).resolve().parent
out=Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
(out/'cases.json').write_text('[{"name":"navigation"}]')
sys.argv=[str(here.parent/'GlassComparison/build-interaction.py'),str(out),str(out/'cases.json')]
runpy.run_path(sys.argv[0],run_name='__main__')
p=out/'Interaction.xcodeproj/project.pbxproj'
s=p.read_text().replace(str(here.parent/'GlassComparison/InteractionReference.m'),str(here/'NativeReference.m')).replace('org.nxkit.GlassInteractionReference','org.nxkit.NavigationReference')
p.write_text(s)
(out/'source-manifest.json').write_text(json.dumps({f.name:hashlib.sha256(f.read_bytes()).hexdigest() for f in [here/'NativeReference.m',here/'build-native.py']},indent=2)+'\n')
