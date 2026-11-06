"""Generate the native UIKit cycling demo and its exhaustive capture fixture."""
import hashlib
import json
import runpy
import sys
from pathlib import Path

here=Path(__file__).resolve().parent
out=Path(sys.argv[1]).resolve()
sys.argv=[str(here.parent/'GlassComparison/build-interaction.py'),str(out),str(here/'CycleCases.json')]
runpy.run_path(sys.argv[0],run_name='__main__')
project=out/'Interaction.xcodeproj/project.pbxproj'
source=project.read_text().replace(str(here.parent/'GlassComparison/InteractionReference.m'),str(here/'CycleReference.m')).replace('org.nxkit.GlassInteractionReference','org.nxkit.NavigationCycleReference')
source=source.replace('"scene.png"','"CycleCases.json"').replace('"lastKnownFileType" = "image.png"','"lastKnownFileType" = "text.json"')
project.write_text(source)
(out/'CycleCases.json').write_bytes((here/'CycleCases.json').read_bytes())
(out/'source-manifest.json').write_text(json.dumps({p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in [here/'CycleReference.m',here/'CycleCases.json',here/'build-cycle.py']},indent=2)+'\n')
