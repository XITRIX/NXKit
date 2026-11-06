"""Build isolated before/after lens adapters without rebuilding shared core.

Usage: build-lens-refraction.py RASTER_BUILD OUTPUT CORE_ARCHIVE [--before=FROZEN_HEADERS] [--tests]
The before branch reproduces the previous selected-content overlay composition.
"""
from pathlib import Path
import hashlib,json,shlex,shutil,subprocess,sys
build,out,archive=(Path(s).resolve() for s in sys.argv[1:4]);out.mkdir(parents=True,exist_ok=True)
source=Path(__file__).resolve().parent/'LensRefractionRenderer.cpp'
if '--tests' in sys.argv:source=source.parent.parent.parent/'UIKit/tests/LensRefractionTests.cpp'
commands=subprocess.check_output(['ninja','-C',str(build),'-t','commands','ContainerTests'],text=True).splitlines()
args=shlex.split(next(s for s in commands if 'GlassContainerTests.cpp.o -c ' in s))
for flag in ['-MF','-MT']:
    if flag in args:i=args.index(flag);del args[i:i+2]
args=[s for s in args if s not in ['-MD','-MMD']]
before=next((Path(s.split('=',1)[1]).resolve() for s in sys.argv if s.startswith('--before=')),None)
if before:args[1:1]=['-DNXKIT_LENS_REFRACTION_BEFORE=1','-I'+str(before),'-I'+str(before/'headers')]
executable=out/source.stem
obj=executable.with_suffix('.o');args[args.index('-o')+1]=str(obj);args[-1]=str(source)
subprocess.run(args,cwd=build,check=True)
shutil.copy2(archive,out/'libLensCore.a')
link=shlex.split(commands[-1].split(' && ')[1]);args=[]
for s in link:
    if s.endswith('GlassContainerTests.cpp.o'):args.append(str(obj))
    elif s=='UIKit/libXITRIXUIKit.a':args.append(str(out/'libLensCore.a'))
    else:args.append(s)
args[args.index('-o')+1]=str(executable);subprocess.run(args,cwd=build,check=True)
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
(out/'build-provenance.json').write_text(json.dumps(dict(branch='before' if before else 'after',rendererSHA256=sha(executable),coreArchiveSHA256=sha(out/'libLensCore.a'),adapterSourceSHA256=sha(source)),indent=2)+'\n')
