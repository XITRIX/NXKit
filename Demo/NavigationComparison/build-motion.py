"""Generate the public UIKit live navigation-animation capture app."""
import runpy,sys,json,hashlib
from pathlib import Path
here=Path(__file__).resolve().parent;out=Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
(out/'cases.json').write_text('[{"name":"motion"}]')
sys.argv=[str(here.parent/'GlassComparison/build-interaction.py'),str(out),str(out/'cases.json')];runpy.run_path(sys.argv[0],run_name='__main__')
p=out/'Interaction.xcodeproj/project.pbxproj';p.write_text(p.read_text().replace(str(here.parent/'GlassComparison/InteractionReference.m'),str(here/'MotionReference.m')).replace('org.nxkit.GlassInteractionReference','org.nxkit.NavigationMotionReference'))
(out/'source-manifest.json').write_text(json.dumps({f.name:hashlib.sha256(f.read_bytes()).hexdigest() for f in [here/'MotionReference.m',here/'build-motion.py']},indent=2)+'\n')
