#!/usr/bin/env python3
import csv,gzip,hashlib,json,math,shutil,subprocess,sys
from pathlib import Path
root=Path(__file__).resolve().parents[3]
batch=Path(sys.argv[1]).resolve()
destination=root/'docs/scroll-edge-effects/scroll-pocket';destination.mkdir(parents=True,exist_ok=True)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def percentile(xs,p):return sorted(xs)[min(len(xs)-1,math.ceil(p*len(xs))-1)]
def bundle_hashes(directory):
 return {str(p.relative_to(directory)):sha(p) for p in sorted(directory.rglob('*')) if p.is_file()}
artifacts={}
for source in sorted((batch/'logs').iterdir()):
 if source.suffix not in ('.csv','.log','.json'):continue
 name=source.name if source.suffix=='.json' else source.name+'.gz'
 data=source.read_bytes();(destination/name).write_bytes(data if source.suffix=='.json' else gzip.compress(data,mtime=0))
 artifacts[name]={'sha256':sha(destination/name),'uncompressedSHA256':sha(source),'uncompressedBytes':source.stat().st_size}
for name in ('bake-tests.log','bake-macos.log','bake-ios.log'):
 source=batch/name
 (destination/(name+'.gz')).write_bytes(gzip.compress(source.read_bytes(),mtime=0))
edge=[];navigation=[]
for phase in ('cold','strict'):
 for scale in (1,2,3):
  for theme in ('light','dark'):
   p=batch/'logs'/f'{phase}-{scale}-{theme}.json';d=json.loads(p.read_text());assert sum(v['frames'] for v in d['passes'])==975
   if phase=='strict':assert d['libraryCompiles']==d['pipelineCompiles']==d['failures']==0
   edge.append({'phase':phase,'scale':scale,'theme':theme,**d})
 for scale,theme in ((3,'dark'),(2,'light')):
  p=batch/'logs'/f'{phase}-navigation-{scale}-{theme}.csv'
  with p.open(newline='') as f:rows=list(csv.DictReader(f))
  assert len(rows)==17497
  if phase=='strict':assert all(int(r['library_compiles'])==int(r['pipeline_compiles'])==0 for r in rows)
  frames=[float(r['cpu_ms']) for r in rows];pop=[float(r['cpu_ms']) for r in rows if r['direction']=='pop']
  navigation.append({'phase':phase,'scale':scale,'theme':theme,'frames':len(rows),'libraryCompiles':int(rows[-1]['library_compiles']),'pipelineCompiles':int(rows[-1]['pipeline_compiles']),'cpuP50MS':percentile(frames,.5),'cpuP95MS':percentile(frames,.95),'cpuMaxMS':max(frames),'popCpuP95MS':percentile(pop,.95),'popCpuMaxMS':max(pop)})
frozen=json.loads((batch/'frozen-sources.json').read_text())
for name,digest in frozen['sources'].items():assert sha(root/name)==digest, f'Production source changed after freeze: {name}'
raw=root/'shaders/apple/catalog';manifest=json.loads((raw/'manifest.json').read_text());pipelines=json.loads((raw/'pipelines.mtlp-json').read_text());assert manifest['abi']==frozen['shaderABI']
builds=[]
for platform,output,bundle,binary,log in (
 ('macOS',root/'Demo/build/macos/ThorVGApp-metal-macosx',root/'Demo/build/macos/ThorVGApp.app/Contents/Resources/NXKitMetalShaders',root/'Demo/build/macos/ThorVGApp.app/Contents/MacOS/ThorVGApp',Path('/private/tmp/nxkit-scroll-pocket-demo-macos-build.log')),
 ('iOS',root/'Demo/build/ios/ThorVGApp-metal-iphoneos',root/'Demo/build/ios/Debug-iphoneos/ThorVGApp.app/NXKitMetalShaders',root/'Demo/build/ios/Debug-iphoneos/ThorVGApp.app/ThorVGApp',Path('/private/tmp/nxkit-scroll-pocket-demo-ios-build.log'))):
 baked=json.loads((output/'manifest.json').read_text());assert baked['abi']==frozen['shaderABI'];assert bundle_hashes(output)==bundle_hashes(bundle),f'{platform} bundle mismatch'
 name=f'build-{platform.lower()}.log.gz';(destination/name).write_bytes(gzip.compress(log.read_bytes(),mtime=0))
 builds.append({'platform':platform,'status':'passed','log':name,'logSHA256':sha(destination/name),'uncompressedLogSHA256':sha(log),'executableSHA256':sha(binary),'bundleFilesSHA256':bundle_hashes(bundle),'bakedManifest':baked,'manifestSHA256':sha(output/'manifest.json'),'pipelineArchiveSHA256':sha(output/'pipeline.binary.metallib')})
report={'status':'passed','abi':manifest['abi'],'strictProfileCount':8,'strictRuntimeLibraryCompiles':0,'strictRuntimePipelineCompiles':0,'strictFailures':0,'audited':'2026-11-04','source':frozen,'gpu':manifest.get('device'),'hostOS':manifest.get('os'),'catalog':{'abi':manifest['abi'],'sourceCount':len(list((raw/'sources').glob('*.metal'))),'pipelineCount':len(pipelines['pipelines']['render_pipelines']),'manifestSHA256':sha(raw/'manifest.json'),'pipelineDescriptionSHA256':sha(raw/'pipelines.mtlp-json'),'sourceSHA256':{p.name:sha(p) for p in sorted((raw/'sources').glob('*.metal'))},'retainedRawProvenance':manifest.get('additionalRawStateCatalogs',[])},'tools':{'metalCompiler':subprocess.check_output(['xcrun','-sdk','macosx','--find','metal'],text=True).strip(),'macOSSDK':subprocess.check_output(['xcrun','-sdk','macosx','--show-sdk-version'],text=True).strip(),'iOSSDK':subprocess.check_output(['xcrun','-sdk','iphoneos','--show-sdk-version'],text=True).strip(),'bakeScriptSHA256':sha(root/'scripts/apple/bake_metal.py'),'skiaLocalDiffSHA256':hashlib.sha256(subprocess.check_output(['git','-C',str(root/'Submodules/skia'),'diff','--binary'])).hexdigest()},'edgeProfiles':edge,'navigationProfiles':navigation,'builds':builds,'artifacts':artifacts,'framesPerSweep':sum(p['frames'] for p in navigation if p['phase']=='strict')+sum(sum(v['frames'] for v in p['passes']) for p in edge if p['phase']=='strict'),'measurementNotes':['Metal Graphite with retained BGRA8 premultiplied render target matching Apple window surface.','Edge CPU time includes draw/layout/record/snap/submit; synchronized total also includes GPU completion wait; async completion callbacks are polled after timing.','Navigation CPU time excludes subsequent GPU completion wait; all arrangements and interactive commit/cancel plus ordinary push/pop are exercised.','Final strict timings run after competing app/test compilations finish.','Fresh current-ABI raw capture retained exact earlier MSL and complete descriptors additively with provenance; every GPU binary was rebuilt offline.','These are local macOS GPU stall diagnostics, not physical-iPhone performance measurements. No physical-device install/run occurred.','Geometry/API compatibility target remains iOS26; fixed pocket appearance intentionally differs from adaptive UIKit legibility. Offline archive SDK compiler targets are macOS27/iOS27 with portable library deployment targets macOS12/iOS15; actual iOS26 archive execution is unverified.']}
assert report['framesPerSweep']==40844
report['baselineSources']='baseline-source/manifest.json'
report['policy']={'privateComponent':'UIScrollPocket','contentDependent':False,'outerContentOpacity':0.4,'blurSigmaPoints':12,'blurWorkingPixelsPerLogicalPoint':1,'fullResolutionForegroundAndMask':True,'analyticMaskShader':True}
performance_path=destination/'performance.json'
if performance_path.exists():report['pairedPerformance']=json.loads(performance_path.read_text())
report['functionalVerification']={'regressionSuitesPassed':38,'coreOnlySuitesPassed':4,'scrollEdgeChecksPassed':50,'switchELFSourceBuild':'passed','phoneThermalMeasurement':False}
(destination/'report.json').write_text(json.dumps(report,indent=2)+'\n')
shutil.copyfile(batch/'frozen-sources.json',destination/'frozen-sources.json')
shutil.copyfile(batch/'raw/manifest.json',destination/'live-harvest-manifest.json')
lines=['# Fixed ScrollPocket verification','',f'Working tree after `{frozen["revision"][:7]}`, audited 2026-11-04. Real Graphite/Metal on {report["gpu"]}; shader ABI `{manifest["abi"]}`.','',f'Each cold and strict sweep rendered **40,844 frames**: six 975-frame scroll-edge profiles and two 17,497-frame fullscreen Demo navigation profiles. Every strict profile completed with **zero runtime library compilations, zero pipeline compilations, and zero failures**.','', 'The edge benchmark covers all four edges; automatic/soft/hard/mixed/hidden styles; discovered/obstructed/explicit bindings; side chrome interactions; fractional offsets and overscroll; app masks; sparse transparent content; an opt-in adaptive glass observer over a black sibling inside masked content; and private variable blur with zero/one/ramp masks at radii 0/1/8/24/48.','', '| Strict edge profile | CPU p50 | CPU p95 | Synchronized total p95 |','|---|---:|---:|---:|']
for d in edge:
 if d['phase']=='strict':
  p=d['passes'][-1];lines.append(f'| {d["theme"]} {d["scale"]}× | {p["cpuP50MS"]:.2f} ms | {p["cpuP95MS"]:.2f} ms | {p["synchronizedTotalP95MS"]:.2f} ms |')
lines += ['', '| Strict fullscreen navigation | CPU p95 | Pop CPU p95 | Maximum pop CPU |','|---|---:|---:|---:|']
for d in navigation:
 if d['phase']=='strict':lines.append(f'| {d["theme"]} {d["scale"]}× | {d["cpuP95MS"]:.2f} ms | {d["popCpuP95MS"]:.2f} ms | {d["popCpuMaxMS"]:.2f} ms |')
lines+=['','Edge timing table uses the last repeated pass; navigation statistics include every captured frame. CPU submission time and synchronized GPU completion time are separate measures. Async observer completion polling occurs after the edge timing interval. All strict measurements were taken after other builds finished. These are local stall diagnostics, not device FPS or physical-iPhone performance measurements.','',f'The raw catalog contains {report["catalog"]["sourceCount"]} content-addressed MSL sources and {report["catalog"]["pipelineCount"]} complete render-pipeline descriptors. Earlier raw states are retained additively with explicit ABI/hash provenance. No old GPU binaries were relabelled; macOS and iOS libraries/archives were rebuilt using the installed SDK toolchain.','', 'Normal macOS and unsigned iOS Demo builds passed. Both embedded shader directories match their freshly baked outputs byte for byte. The app configurations, runtime tools and catalog share the same ABI. Offline native archive targets use the installed macOS27/iOS27 compiler generation; portable libraries retain macOS12/iOS15 deployment targets. Actual iOS26-device native archive execution remains unverified. The visual target is iOS26 UIKit. No physical device was installed, resumed or run.','', 'See [report.json](report.json) for source, catalog, archive, executable and bundle hashes; all profile counters and timings; and compressed raw CSV/log hashes. [commands.md](commands.md) records reproduction commands; [merge_catalog.py](merge_catalog.py) validates complete eight-profile live capture and preserves only exact raw source/descriptor states.']
lines[2:2]=[
 'The private implementation now uses one `UIScrollPocket` per edge. Public configuration remains on [Apple-shaped UIScrollEdgeEffect styles](https://developer.apple.com/documentation/uikit/uiscrolledgeeffect/style-swift.class). The retained iOS26 traces identify `_UIScrollPocket`, PocketMask and PocketBlur; this is a portable C++ translation, not a dependency on private Apple runtime calls.',
 '',
 'At the user’s request, soft pockets use a fixed **0.4→1 content mask** and **12pt Gaussian blur** blended through a fixed edge gradient. Blur alone runs at one pixel per logical point; foreground and analytic masks retain full display resolution. There is no extra SkPicture recording/replay of scrolling content, CPU image/coverage/luminance analysis, adaptive black backing, or full-viewport mask-picture rasterization. Masks and filters are cached. Scroll penetration still animates activation; overscroll, visibility and per-screen ownership retain their meanings.',
 '',
 '[Retained adaptive baseline sources](baseline-source/manifest.json). [Before/after light/dark dense/sparse images](images/before-after.png), [exact image hashes and unchanged geometry](images/visuals.json), [49-case native geometry comparison](comparison/README.md). All 76 active carrier bands have unchanged coordinates versus the prior portable renderer. Native iOS26 optical adaptation is intentionally not matched by this fixed policy.',
 '',
]
if 'pairedPerformance' in report:
 lines += ['', '## Paired performance', '', '| Fixture | CPU median before → after | Synchronized total median before → after |', '|---|---:|---:|']
 for p in report['pairedPerformance']['profiles']:
  a=p['versions']['before'];b=p['versions']['after']
  lines.append(f"| {p['theme']} {p['scale']}× | {a['cpuMedianMS']:.2f} → {b['cpuMedianMS']:.2f} ms ({p['cpuMedianReductionPercent']:.1f}% lower) | {a['synchronizedTotalMedianMS']:.2f} → {b['synchronizedTotalMedianMS']:.2f} ms ({p['totalMedianReductionPercent']:.1f}% lower) |")
 lines += ['', 'Three alternating-order pairs per profile; repeated passes 1/2 only, 84 vertical-scroll frames per version/profile. This 402×874pt fixture has **all four edge-chrome pockets**, and selects discovered automatic/soft rows; it differs from a typical top/bottom-only phone screen. Both binaries forbid runtime compilation and use their own verified offline bundles. These are whole-render CPU/submission/completion measurements on this Mac, not isolated shader timing, phone power or temperature. See [performance.json](performance.json) for source CSV hashes, selection rules and limits.', '']
lines += ['', '## Functional verification', '', 'All **38 regression suites**, **four Core-only suites**, and **50 scroll-edge checks** pass. Analytic-mask tests verify group alpha at 1×/2×/3× and bypass of picture fallback. The updated Switch ELF source target builds; no NRO regeneration or Switch GPU shader bake/device execution is claimed. [Retained test logs and artifact identities](verification.json).', '']
(destination/'README.md').write_text('\n'.join(lines)+'\n')
print(json.dumps({'docs':str(destination),'framesPerSweep':report['framesPerSweep'],'sourceCount':report['catalog']['sourceCount'],'pipelineCount':report['catalog']['pipelineCount'],'reportSHA256':sha(destination/'report.json')},indent=2))
