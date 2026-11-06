"""Generate the public UIKit layout/touch reference project.
python3 Demo/NavigationComparison/LargeTitles/build.py /tmp/large-title-native
xcodebuild -project /tmp/large-title-native/Interaction.xcodeproj -scheme Interaction \
  -destination 'platform=iOS Simulator,id=YOUR_DEVICE' -parallel-testing-enabled NO test
"""
from pathlib import Path
import runpy
import sys
here=Path(__file__).resolve().parent
out=Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
(out/'cases.json').write_text('[{}]')
sys.argv=[str(here.parent.parent/'GlassComparison/build-interaction.py'),str(out),str(out/'cases.json')]
runpy.run_path(sys.argv[0],run_name='__main__')
p=out/'Interaction.xcodeproj/project.pbxproj'
s=p.read_text().replace(str(here.parent.parent/'GlassComparison/InteractionReference.m'),str(here/'NativeReference.m')).replace(str(here.parent.parent/'GlassComparison/InteractionUITests.m'),str(here/'TouchTests.m')).replace('org.nxkit.GlassInteractionReference','org.nxkit.LargeTitles')
p.write_text(s)
