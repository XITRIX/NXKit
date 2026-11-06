"""Generate a standalone native UIKit app + public XCUITest touch runner.
Usage: python3 build-interaction.py OUTPUT_DIRECTORY CASES_JSON
Then: xcodebuild -project OUTPUT/Interaction.xcodeproj -scheme Interaction \
      -destination 'platform=iOS Simulator,id=DEVICE' -derivedDataPath OUTPUT/build test
"""
import json
from pathlib import Path
import plistlib
import shutil
import sys

output=Path(sys.argv[1]).resolve(); output.mkdir(parents=True,exist_ok=True)
source=Path(__file__).resolve().parent
cases=json.loads(Path(sys.argv[2]).read_text())
if not isinstance(cases,list) or not cases:
    raise ValueError('Expected a nonempty list of touch scenarios')
(output/'Cases.json').write_text(json.dumps(cases,indent=2)+'\n')
info=plistlib.loads((source/'Info.plist').read_bytes())
info.update(CFBundleIdentifier='$(PRODUCT_BUNDLE_IDENTIFIER)',CFBundleName='Glass Interaction',CFBundleExecutable='$(EXECUTABLE_NAME)')
(output/'Info.plist').write_bytes(plistlib.dumps(info))
project=output/'Interaction.xcodeproj'; project.mkdir(exist_ok=True)
objects={}
def obj(identifier,**fields):
    objects[identifier]=fields
    return identifier
def ident(n): return f'{n:024X}'
project_id,group,products,app,tests,app_product,test_product=map(ident,range(1,8))
app_file,test_file,case_file,art_file=map(ident,range(10,14))
obj(app_file,isa='PBXFileReference',lastKnownFileType='sourcecode.c.objc',path=str(source/'InteractionReference.m'),sourceTree='<absolute>')
obj(test_file,isa='PBXFileReference',lastKnownFileType='sourcecode.c.objc',path=str(source/'InteractionUITests.m'),sourceTree='<absolute>')
obj(case_file,isa='PBXFileReference',lastKnownFileType='text.json',path='Cases.json',sourceTree='<group>')
obj(art_file,isa='PBXFileReference',lastKnownFileType='image.png',path=str(source.parent/'resources/img/glass_demo_scene.png'),name='scene.png',sourceTree='<absolute>')
# Use the resource's actual scene.png basename in the app bundle.
shutil.copyfile(source.parent/'resources/img/glass_demo_scene.png',output/'scene.png')
objects[art_file].update(path='scene.png',sourceTree='<group>')
obj(app_product,isa='PBXFileReference',explicitFileType='wrapper.application',path='InteractionReference.app',sourceTree='BUILT_PRODUCTS_DIR')
obj(test_product,isa='PBXFileReference',explicitFileType='wrapper.cfbundle',path='InteractionUITests.xctest',sourceTree='BUILT_PRODUCTS_DIR')
obj(products,isa='PBXGroup',children=[app_product,test_product],name='Products',sourceTree='<group>')
obj(group,isa='PBXGroup',children=[app_file,test_file,case_file,art_file,products],sourceTree='<group>')
def phase(n,kind,file):
    build_file=obj(ident(n+1),isa='PBXBuildFile',fileRef=file)
    return obj(ident(n),isa=kind,buildActionMask=2147483647,files=[build_file],runOnlyForDeploymentPostprocessing=0)
app_sources=phase(20,'PBXSourcesBuildPhase',app_file)
app_resources=phase(22,'PBXResourcesBuildPhase',art_file)
test_sources=phase(24,'PBXSourcesBuildPhase',test_file)
test_resources=phase(26,'PBXResourcesBuildPhase',case_file)
proxy=obj(ident(28),isa='PBXContainerItemProxy',containerPortal=project_id,proxyType=1,remoteGlobalIDString=app,remoteInfo='InteractionReference')
dependency=obj(ident(29),isa='PBXTargetDependency',target=app,targetProxy=proxy)
common={'SDKROOT':'iphonesimulator','IPHONEOS_DEPLOYMENT_TARGET':'26.0','TARGETED_DEVICE_FAMILY':'1,2',
        'CLANG_ENABLE_OBJC_ARC':'YES','CODE_SIGNING_ALLOWED':'NO','PRODUCT_NAME':'$(TARGET_NAME)',
        'SUPPORTED_PLATFORMS':'iphonesimulator','DEBUG_INFORMATION_FORMAT':'dwarf-with-dsym',
        'ENABLE_TESTABILITY':'YES','GCC_OPTIMIZATION_LEVEL':'0','ARCHS':'arm64','ONLY_ACTIVE_ARCH':'YES',
        'ALWAYS_SEARCH_USER_PATHS':'NO',
        'OTHER_LDFLAGS':['$(inherited)','-framework','UIKit','-framework','QuartzCore','-framework','Foundation','-framework','CoreGraphics']}
def configurations(n,settings):
    debug=obj(ident(n+1),isa='XCBuildConfiguration',buildSettings={**common,**settings},name='Debug')
    release=obj(ident(n+2),isa='XCBuildConfiguration',buildSettings={**common,**settings},name='Release')
    return obj(ident(n),isa='XCConfigurationList',buildConfigurations=[debug,release],defaultConfigurationIsVisible=0,defaultConfigurationName='Debug')
app_config=configurations(30,{'PRODUCT_BUNDLE_IDENTIFIER':'org.nxkit.GlassInteractionReference','INFOPLIST_FILE':'Info.plist',
    'LD_RUNPATH_SEARCH_PATHS':['$(inherited)','@executable_path/Frameworks']})
test_config=configurations(34,{'PRODUCT_BUNDLE_IDENTIFIER':'org.nxkit.GlassInteractionUITests','GENERATE_INFOPLIST_FILE':'YES',
    'TEST_TARGET_NAME':'InteractionReference','LD_RUNPATH_SEARCH_PATHS':['$(inherited)','@executable_path/Frameworks','@loader_path/Frameworks'],
    'OTHER_LDFLAGS':common['OTHER_LDFLAGS']+['-framework','XCTest','-framework','XCUIAutomation']})
project_config=configurations(38,{})
obj(app,isa='PBXNativeTarget',name='InteractionReference',productName='InteractionReference',productType='com.apple.product-type.application',
    buildConfigurationList=app_config,buildPhases=[app_sources,app_resources],buildRules=[],dependencies=[],productReference=app_product)
obj(tests,isa='PBXNativeTarget',name='InteractionUITests',productName='InteractionUITests',productType='com.apple.product-type.bundle.ui-testing',
    buildConfigurationList=test_config,buildPhases=[test_sources,test_resources],buildRules=[],dependencies=[dependency],productReference=test_product)
obj(project_id,isa='PBXProject',attributes={'LastUpgradeCheck':'2600','TargetAttributes':{tests:{'TestTargetID':app}}},
    buildConfigurationList=project_config,compatibilityVersion='Xcode 14.0',developmentRegion='en',hasScannedForEncodings=0,
    knownRegions=['en','Base'],mainGroup=group,productRefGroup=products,projectDirPath='',projectRoot='',targets=[app,tests])
def encode(value):
    if isinstance(value,dict): return '{ '+''.join(f'{json.dumps(k)} = {encode(v)}; ' for k,v in value.items())+'}'
    if isinstance(value,list): return '('+''.join(encode(v)+',' for v in value)+')'
    if isinstance(value,int): return str(value)
    return json.dumps(value)
(project/'project.pbxproj').write_text('// !$*UTF8*$!\n'+encode({'archiveVersion':1,'classes':{},'objectVersion':56,'objects':objects,'rootObject':project_id})+'\n')
schemes=project/'xcshareddata/xcschemes'; schemes.mkdir(parents=True,exist_ok=True)
ref=lambda target,name:f'<BuildableReference BuildableIdentifier="primary" BlueprintIdentifier="{target}" BuildableName="{name}" BlueprintName="{name.split(".")[0]}" ReferencedContainer="container:Interaction.xcodeproj"/>'
(schemes/'Interaction.xcscheme').write_text(f'''<?xml version="1.0" encoding="UTF-8"?>
<Scheme LastUpgradeVersion="2600" version="1.3">
 <BuildAction parallelizeBuildables="YES" buildImplicitDependencies="YES"><BuildActionEntries>
  <BuildActionEntry buildForTesting="YES" buildForRunning="YES" buildForProfiling="YES" buildForArchiving="YES" buildForAnalyzing="YES">{ref(app,'InteractionReference.app')}</BuildActionEntry>
  <BuildActionEntry buildForTesting="YES" buildForRunning="NO" buildForProfiling="NO" buildForArchiving="NO" buildForAnalyzing="NO">{ref(tests,'InteractionUITests.xctest')}</BuildActionEntry>
 </BuildActionEntries></BuildAction>
 <TestAction buildConfiguration="Debug" selectedDebuggerIdentifier="Xcode.DebuggerFoundation.Debugger.LLDB" selectedLauncherIdentifier="Xcode.IDEFoundation.Launcher.LLDB" shouldUseLaunchSchemeArgsEnv="YES">
  <Testables><TestableReference skipped="NO">{ref(tests,'InteractionUITests.xctest')}</TestableReference></Testables>
 </TestAction>
 <LaunchAction buildConfiguration="Debug" selectedDebuggerIdentifier="Xcode.DebuggerFoundation.Debugger.LLDB" selectedLauncherIdentifier="Xcode.IDEFoundation.Launcher.LLDB" launchStyle="0" useCustomWorkingDirectory="NO" ignoresPersistentStateOnLaunch="NO" debugDocumentVersioning="YES" debugServiceExtension="internal" allowLocationSimulation="YES">
  <BuildableProductRunnable runnableDebuggingMode="0">{ref(app,'InteractionReference.app')}</BuildableProductRunnable>
 </LaunchAction>
</Scheme>''')
print(project)
