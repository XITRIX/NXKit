"""Verify generated IDs refresh without discarding user benchmark settings."""
import pathlib
import subprocess
import tempfile
import xml.etree.ElementTree as ET

script = pathlib.Path(__file__).resolve().parents[2] / 'Demo/scripts/performance_scheme.py'
with tempfile.TemporaryDirectory() as tmp:
    project = pathlib.Path(tmp) / 'Demo.xcodeproj'
    schemes = project / 'xcshareddata/xcschemes'
    schemes.mkdir(parents=True)
    source = schemes / 'Demo.xcscheme'
    destination = schemes / 'Demo Performance.xcscheme'
    source.write_text('<Scheme><BuildAction><BuildableReference BlueprintIdentifier="old"/></BuildAction><LaunchAction/><ProfileAction/></Scheme>')
    def generate():
        subprocess.run(['python3', str(script), str(project), 'Demo'], check=True)
    generate()
    tree = ET.parse(destination)
    launch = tree.getroot().find('LaunchAction')
    env = launch.find('EnvironmentVariables')
    assert {e.get('key'):e.get('value') for e in env} == {'NXKIT_UNCAPPED':'0', 'MTL_DEBUG_LAYER':'0'}
    next(e for e in env if e.get('key') == 'NXKIT_UNCAPPED').set('value', '1')
    ET.SubElement(env, 'EnvironmentVariable', key='NXKIT_GLASS_STRESS_REPLAY', value='1', isEnabled='YES')
    ET.SubElement(launch, 'CommandLineArguments')
    tree.write(destination)
    source.write_text(source.read_text().replace('old', 'new'))
    generate()
    root = ET.parse(destination).getroot()
    assert root.find('.//BuildableReference').get('BlueprintIdentifier') == 'new'
    env = {e.get('key'):e.get('value') for e in root.find('LaunchAction/EnvironmentVariables')}
    assert env == {'NXKIT_UNCAPPED':'1', 'NXKIT_GLASS_STRESS_REPLAY':'1', 'MTL_DEBUG_LAYER':'0'}
    assert root.find('LaunchAction/CommandLineArguments') is not None
    for action in ('LaunchAction', 'ProfileAction'):
        node = root.find(action)
        assert node.get('enableGPUValidationMode') == '1'
        assert node.get('enableGPUFrameCaptureMode') == '3'
        assert node.get('buildConfiguration') == 'RelWithDebInfo'
print('Performance scheme defaults, target IDs and user overrides passed')
