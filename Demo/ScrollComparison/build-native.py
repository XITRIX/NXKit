"""Reuse the standalone UIKit/XCTest project generator for scroll oracles."""
from pathlib import Path
import sys,runpy,plistlib
source=Path(__file__).resolve().parent;glass=source.parent/'GlassComparison'
out=Path(sys.argv[1]).resolve();cases=Path(sys.argv[2]).resolve()
sys.argv=[str(glass/'build-interaction.py'),str(out),str(cases)]
runpy.run_path(str(glass/'build-interaction.py'),run_name='__main__')
p=out/'Interaction.xcodeproj/project.pbxproj';s=p.read_text().replace(str(glass/'InteractionReference.m'),str(source/'ScrollReference.m')).replace(str(glass/'InteractionUITests.m'),str(source/'ScrollUITests.m')).replace('org.nxkit.GlassInteractionReference','org.nxkit.ScrollReference').replace('org.nxkit.GlassInteractionUITests','org.nxkit.ScrollUITests');p.write_text(s)
p=out/'Info.plist';d=plistlib.loads(p.read_bytes());d['UISupportedInterfaceOrientations']=['UIInterfaceOrientationPortrait','UIInterfaceOrientationLandscapeLeft','UIInterfaceOrientationLandscapeRight'];p.write_bytes(plistlib.dumps(d))

import shutil,hashlib,json
files=['ScrollReference.m','ScrollUITests.m','build-native.py']
for name in files:shutil.copy2(source/name,out/(name+'.captured'))
(out/'source-manifest.json').write_text(json.dumps({name:hashlib.sha256((source/name).read_bytes()).hexdigest() for name in files},indent=2)+'\n')
