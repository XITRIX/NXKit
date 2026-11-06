"""Build the lens oracle with a separate real-touch XCUITest target.

Usage: python build-lens-motion.py OUTPUT CASES_JSON
Reuses this repository's standalone Xcode project generator; private lens
selectors stay in the native oracle and never enter portable NXKit.
"""
from pathlib import Path
import hashlib
import json
import runpy
import shutil
import sys

output=Path(sys.argv[1]).resolve()
source=Path(__file__).resolve().parent
sys.argv=[str(source/'build-interaction.py'),str(output),sys.argv[2]]
runpy.run_path(str(source/'build-interaction.py'),run_name='__main__')
project=output/'Interaction.xcodeproj/project.pbxproj'
text=project.read_text()
text=text.replace(str(source/'InteractionReference.m'),str(source/'LensReference.m'))
text=text.replace(str(source/'InteractionUITests.m'),str(source/'LensMotionUITests.m'))
text=text.replace('org.nxkit.GlassInteractionReference','org.nxkit.LensMotionReference')
text=text.replace('org.nxkit.GlassInteractionUITests','org.nxkit.LensMotionUITests')
project.write_text(text)
hashes={}
for name in ['LensReference.m','LensMotionUITests.m']:
    digest=hashlib.sha256((source/name).read_bytes()).hexdigest()
    hashes[name]=digest
    shutil.copy2(source/name,output/(name+'.captured'))
(output/'source-manifest.json').write_text(json.dumps(hashes,indent=2)+'\n')
