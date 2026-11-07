#!/usr/bin/env python3
"""Add final live slider/native-navigation raw states to the retained catalog.

Only content-addressed MSL and pipeline descriptors are reused. GPU libraries
and archives must be rebuilt by bake_metal.py for the final ABI and SDK.
"""
import argparse,hashlib,json,shutil,sys
from pathlib import Path
root=Path(__file__).resolve().parents[2];sys.path.insert(0,str(root/'scripts/apple'))
from bake_metal import inputs
parser=argparse.ArgumentParser();parser.add_argument('live',type=Path)
parser.add_argument('--coverage',type=Path,help='JSON description of the actual fresh replay coverage')
args=parser.parse_args()
catalog=root/'shaders/apple/catalog'
old,oldSources,oldScript=inputs(catalog);live,liveSources,liveScript=inputs(args.live)
header=(root/'Demo/build/ios-sim-ninja/Thirdparty/UIKit/UIKitCore/UIKitCore-metal-generated/NXMetalShaderABI.hpp').read_text()
assert live['abi'] in header,'Live capture does not match the current Simulator build'
hashfile=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
provenance={'abi':old['abi'],'manifestSHA256':hashfile(catalog/'manifest.json'),
    'pipelineDescriptionSHA256':hashfile(catalog/'pipelines.mtlp-json'),
    'reason':'Retain exact content-addressed raw MSL and complete pipeline descriptors as an additive superset; no old GPU binary is reused or relabelled.'}
aliases={};pipelines={}
for script in [oldScript,liveScript]:
    for item in script['libraries']['paths']:aliases[item['label']]={'label':item['label'],'path':'libraries/'+item['label']+'.metallib'}
    for item in script['pipelines']['render_pipelines']:pipelines[json.dumps(item,sort_keys=True,separators=(',',':'))]=item
for path in liveSources:shutil.copy2(path,catalog/'sources'/path.name)
script={'libraries':{'paths':[aliases[key] for key in sorted(aliases)]},'pipelines':{'render_pipelines':[pipelines[key] for key in sorted(pipelines)]},'exportFailed':False}
(catalog/'pipelines.mtlp-json').write_text(json.dumps(script,indent=2)+'\n')
manifest=dict(live);manifest['additionalRawStateCatalogs']=old.get('additionalRawStateCatalogs',[])+[provenance]
manifest['coverage']={'fixture':'Demo/GlassComparison/SimulatorGlassBlurTests.mm --slider-only','cases':360,
    'lens':{'fixture':'SimulatorGlassBlurTests --lens-only','semanticCases':48,'boundedCases':480,'liftedCases':48},
    'lensStateControls':{'fixture':'SimulatorGlassBlurTests --lens-state-output','cases':128},
    'switch':{'fixture':'SimulatorGlassBlurTests --switch-only','cases':144},
    'lensMotion':'MetalNavigationBenchmark --lens-motion --quick --passes 2; 2/3/4/5 items',
    'controlGallery':'MetalNavigationBenchmark --controls-gallery --passes 1',
    'navigation':'Prior raw catalog retained as an additive superset; current capture covers the control gallery and lens motion',
    'freshLibraries':len(liveSources),'freshPipelines':len(liveScript['pipelines']['render_pipelines'])}
if args.coverage:
    manifest['coverage']={**json.loads(args.coverage.read_text()),
        'freshLibraries':len(liveSources),'freshPipelines':len(liveScript['pipelines']['render_pipelines'])}
(catalog/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
_,sources,validated=inputs(catalog)
print(json.dumps({'abi':live['abi'],'libraries':len(sources),'pipelines':len(validated['pipelines']['render_pipelines']),'prior':provenance},indent=2))
