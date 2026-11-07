from pathlib import Path
import runpy,sys
source=Path(__file__).resolve().parent
output=Path(sys.argv[1]).resolve();output.mkdir(parents=True,exist_ok=True)
(output/'cases.json').write_text('[{"id":"control-scroll"}]')
sys.argv=[str(source.parent/'GlassComparison/build-interaction.py'),str(output),str(output/'cases.json')]
runpy.run_path(sys.argv[0],run_name='__main__')
p=output/'Interaction.xcodeproj/project.pbxproj'
p.write_text(p.read_text().replace(str(source.parent/'GlassComparison/InteractionReference.m'),str(source/'Reference.m')).replace(str(source.parent/'GlassComparison/InteractionUITests.m'),str(source/'UITests.m')).replace('org.nxkit.GlassInteractionReference','org.nxkit.ControlScrollReference').replace('org.nxkit.GlassInteractionUITests','org.nxkit.ControlScrollUITests'))
