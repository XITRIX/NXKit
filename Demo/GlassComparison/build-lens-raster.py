"""Build the production lens/tab comparison adapter from an existing raster build.
Usage: build-lens-raster.py RASTER_BUILD OUTPUT [--core-archive=PATH]
"""
from pathlib import Path
import shlex, shutil, subprocess, sys
build,out=map(lambda x:Path(x).resolve(),sys.argv[1:3]);out.mkdir(parents=True,exist_ok=True)
source=Path(__file__).resolve().parent
commands=subprocess.check_output(['ninja','-C',str(build),'-t','commands','ContainerTests'],text=True).splitlines()
compile_args=shlex.split(next(x for x in commands if 'GlassContainerTests.cpp.o -c ' in x))
for flag in ['-MF','-MT']:
 if flag in compile_args: i=compile_args.index(flag);del compile_args[i:i+2]
compile_args=[a for a in compile_args if a not in ['-MD','-MMD']]
objects=[]
for path in [source/'LensRenderer.cpp',source/'../app/Screens/GlassDemoViewController/LensTabBarView.cpp']:
 obj=out/(path.stem+'.o');args=compile_args.copy();args[args.index('-o')+1]=str(obj);args[-1]=str(path);subprocess.run(args,cwd=build,check=True);objects.append(str(obj))
archive=next((Path(x.split('=',1)[1]).resolve() for x in sys.argv if x.startswith('--core-archive=')),build/'UIKit/libXITRIXUIKit.a')
shutil.copy2(archive,out/'libLensCore.a')
link=shlex.split(commands[-1].split(' && ')[1]);args=[]
for a in link:
 if a.endswith('GlassContainerTests.cpp.o'):args.extend(objects)
 elif a=='UIKit/libXITRIXUIKit.a':args.append(str(out/'libLensCore.a'))
 else:args.append(a)
args[args.index('-o')+1]=str(out/'LensRenderer');subprocess.run(args,cwd=build,check=True)
