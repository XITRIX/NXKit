"""Build before/current optical renderers from an existing Ninja raster build.

Usage: python build-merge-checkpoints.py RASTER_BUILD OUTPUT_DIRECTORY [--baseline-press] [--baseline-ref REF]
       [--candidate-container SOURCE] [--core-library ARCHIVE] [--baseline-core-library ARCHIVE]
The baseline is HEAD's container shader; neutral Core and other UIKit objects are identical.
Legacy --core-library flags select the UIKit material archive after the module split.
Reads the existing build's compiler/linker commands and writes only OUTPUT.
"""
from pathlib import Path
import hashlib
import json
import shlex
import shutil
import subprocess
import sys

build, output = (Path(value).resolve() for value in sys.argv[1:3])
output.mkdir(parents=True,exist_ok=True)
root=Path(__file__).resolve().parents[2]
baseline_ref=sys.argv[sys.argv.index('--baseline-ref')+1] if '--baseline-ref' in sys.argv else 'HEAD'
commands=subprocess.check_output(['ninja','-C',str(build),'-t','commands','ContainerTests'],text=True).splitlines()
def compile_args(source_fragment,source,target):
    line=next(line for line in commands if source_fragment+' -c ' in line)
    args=shlex.split(line); result=[]; index=0
    while index<len(args):
        if args[index] in ['-MF','-MT','-MQ']:
            index+=2;continue
        if args[index] in ['-MD','-MMD']:
            index+=1;continue
        result.append(args[index]);index+=1
    result[result.index('-o')+1]=str(target);result[-1]=str(source)
    return result
renderer=root/'Demo/GlassComparison/MergeCheckpointRenderer.cpp'
renderer_object=output/'MergeCheckpointRenderer.o'
subprocess.run(compile_args('GlassContainerTests.cpp.o',renderer,renderer_object),cwd=build,check=True)
def baseline_file(current, legacy):
    result=subprocess.run(['git','-C',str(root),'show',baseline_ref+':'+current],capture_output=True)
    if result.returncode==0:return result.stdout
    return subprocess.check_output(['git','-C',str(root),'show',baseline_ref+':'+legacy])
baseline=baseline_file('UIKit/lib/CAGlassContainerLayer.cpp','UIKitCore/lib/CAGlassContainerLayer.cpp')
baseline_source=output/'CAGlassContainerLayer-before.cpp';baseline_source.write_bytes(baseline.replace(b'BackdropEffect::glass()',b'UIGlassEffect(UIGlassEffect::Style::clear).backdropEffect()'))
baseline_object=output/'CAGlassContainerLayer.cpp.o'
subprocess.run(compile_args('CAGlassContainerLayer.cpp.o',baseline_source,baseline_object),cwd=build,check=True)
current_library=Path(sys.argv[sys.argv.index('--core-library')+1]).resolve() if '--core-library' in sys.argv else build/'UIKit/libXITRIXUIKit.a'
baseline_library=Path(sys.argv[sys.argv.index('--baseline-core-library')+1]).resolve() if '--baseline-core-library' in sys.argv else current_library
current_copy=output/'libUIKitAfter.a';shutil.copy2(current_library,current_copy)
baseline_copy=output/'libUIKitBefore.a';shutil.copy2(baseline_library,baseline_copy)
if '--candidate-container' in sys.argv[3:]:
    source=Path(sys.argv[sys.argv.index('--candidate-container')+1]).resolve()
    candidate=output/'candidate';candidate.mkdir(exist_ok=True)
    target=candidate/'CAGlassContainerLayer.cpp.o'
    subprocess.run(compile_args('CAGlassContainerLayer.cpp.o',source,target),cwd=build,check=True)
    subprocess.run(['ar','r',str(current_copy),str(target)],check=True)

subprocess.run(['ar','r',str(baseline_copy),str(baseline_object)],check=True)
press_baseline=None
if '--baseline-press' in sys.argv[3:]:
    press_baseline=baseline_file('UIKit/lib/GlassShader.cpp','UIKitCore/lib/BackdropEffect.cpp')
    adapted=press_baseline.decode()
    if 'bool isAutomaticUniformName' in adapted:
        shader=adapted[adapted.index('const std::string glassShader'):adapted.index('bool isAutomaticUniformName')]
        factory=adapted[adapted.index('BackdropEffect BackdropEffect::glassShaderEffect('):adapted.index('std::optional<std::vector<NXFloat>> BackdropEffect::uniform(')]
        factory=factory.replace('BackdropEffect BackdropEffect::glassShaderEffect(', 'BackdropEffect NXKit::Internal::makeGlassShaderEffect(')
        adapted='#include "GlassShader.hpp"\n#include "LayerCornerGeometry.hpp"\nusing namespace NXKit;\nnamespace {\n'+shader+'}\n'+factory
    source=output/'GlassShader-before.cpp';source.write_text(adapted)
    target=output/'GlassShader.cpp.o'
    subprocess.run(compile_args('GlassShader.cpp.o',source,target),cwd=build,check=True)
    subprocess.run(['ar','r',str(baseline_copy),str(target)],check=True)
link=shlex.split(commands[-1].split(' && ')[1])
link=[str(renderer_object) if arg.endswith('GlassContainerTests.cpp.o') else arg for arg in link]
for name,library in [('Before',baseline_copy),('After',current_copy)]:
    args=[str(library) if arg=='UIKit/libXITRIXUIKit.a' else arg for arg in link]
    args[args.index('-o')+1]=str(output/f'MergeCheckpoint{name}')
    subprocess.run(args,cwd=build,check=True)
provenance=dict(baselineRevision=subprocess.check_output(['git','-C',str(root),'rev-parse',baseline_ref],text=True).strip(),
    baselineContainerSHA256=hashlib.sha256(baseline).hexdigest(),
    baselineAdapterSHA256=hashlib.sha256(baseline_source.read_bytes()).hexdigest(),
    currentContainerSHA256=hashlib.sha256((root/'UIKit/lib/CAGlassContainerLayer.cpp').read_bytes()).hexdigest(),
    otherCoreObjects='Identical objects from the supplied raster archive; dynamics are bypassed by the checkpoint renderer.',
    coreArchiveSHA256=hashlib.sha256(current_library.read_bytes()).hexdigest())
if baseline_library != current_library:
    provenance['baselineCoreArchiveSHA256']=hashlib.sha256(baseline_library.read_bytes()).hexdigest()
    provenance['otherCoreObjects']='Separate supplied before/after core archives; dynamics are bypassed by the checkpoint renderer.'
if '--candidate-container' in sys.argv[3:]:
    provenance['candidateContainerSHA256']=hashlib.sha256(Path(sys.argv[sys.argv.index('--candidate-container')+1]).read_bytes()).hexdigest()
if press_baseline is not None:
    provenance['baselinePressSHA256']=hashlib.sha256(press_baseline).hexdigest()
(output/'provenance.json').write_text(json.dumps(provenance,indent=2)+'\n')
print(json.dumps(provenance,indent=2))
