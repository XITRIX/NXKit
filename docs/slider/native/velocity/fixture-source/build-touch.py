from pathlib import Path
import json,runpy,sys
source=Path(__file__).resolve().parent; output=Path(sys.argv[1]).resolve();output.mkdir(parents=True,exist_ok=True)
(output/'cases.json').write_text('[{"id":"slider"}]')
sys.argv=[str(source.parent/'GlassComparison/build-interaction.py'),str(output),str(output/'cases.json')]
runpy.run_path(sys.argv[0],run_name='__main__')
project=output/'Interaction.xcodeproj/project.pbxproj';text=project.read_text().replace(str(source.parent/'GlassComparison/InteractionReference.m'),str(source/'SliderReference.m')).replace(str(source.parent/'GlassComparison/InteractionUITests.m'),str(source/'SliderUITests.m')).replace('org.nxkit.GlassInteractionReference','org.nxkit.SliderReference').replace('org.nxkit.GlassInteractionUITests','org.nxkit.SliderUITests');project.write_text(text)
