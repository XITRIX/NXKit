#!/usr/bin/env python3
"""Generate a disposable, unsigned iOS simulator probe and public XCTest target."""
import plistlib
import sys
from pathlib import Path

root = Path(__file__).resolve().parents[2]
output = Path(sys.argv[1] if len(sys.argv) > 1 else "/tmp/nxkit-rotation-probe").resolve()
project = output / "Rotation.xcodeproj"
project.mkdir(parents=True, exist_ok=True)
objects = {}


def node(kind, **values):
    key = f"{len(objects)+1:024X}"
    objects[key] = {"isa": kind, **values}
    return key


def sources(paths):
    files = []
    for path, kind in paths:
        ref = node("PBXFileReference", path=str(path), sourceTree="<absolute>", lastKnownFileType=kind)
        files.append(node("PBXBuildFile", fileRef=ref))
    return node("PBXSourcesBuildPhase", buildActionMask=2147483647, files=files, runOnlyForDeploymentPostprocessing=0)


def settings(extra):
    common = {"SDKROOT": "iphonesimulator", "SUPPORTED_PLATFORMS": "iphonesimulator",
              "IPHONEOS_DEPLOYMENT_TARGET": "15.0", "TARGETED_DEVICE_FAMILY": "1,2",
              "CODE_SIGNING_ALLOWED": "NO", "CLANG_ENABLE_OBJC_ARC": "YES",
              "CLANG_CXX_LANGUAGE_STANDARD": "c++20", "ARCHS": "arm64", "ONLY_ACTIVE_ARCH": "YES", "PRODUCT_NAME": "$(TARGET_NAME)",
              "HEADER_SEARCH_PATHS": [str(root / "UIKitCore/include")],
              "OTHER_LDFLAGS": ["-framework", "UIKit", "-framework", "QuartzCore", "-framework", "Foundation", "-framework", "CoreGraphics", "-framework", "Metal"],
              "LD_RUNPATH_SEARCH_PATHS": ["$(inherited)", "@executable_path/Frameworks", "@loader_path/Frameworks"]}
    common.update(extra)
    config = node("XCBuildConfiguration", name="Debug", buildSettings=common)
    return node("XCConfigurationList", buildConfigurations=[config], defaultConfigurationName="Debug", defaultConfigurationIsVisible=0)


app_product = node("PBXFileReference", path="RotationProbe.app", explicitFileType="wrapper.application", sourceTree="BUILT_PRODUCTS_DIR")
test_product = node("PBXFileReference", path="RotationUITests.xctest", explicitFileType="wrapper.cfbundle", sourceTree="BUILT_PRODUCTS_DIR")
app_sources = sources([(root / "Demo/ResizeComparison/RotationProbe.mm", "sourcecode.cpp.objcpp"),
                       (root / "UIKitCore/lib/platforms/apple/ios/IOSWindowGeometry.mm", "sourcecode.cpp.objcpp")])
test_sources = sources([(root / "Demo/ResizeComparison/RotationUITests.m", "sourcecode.c.objc")])
app = node("PBXNativeTarget", name="RotationProbe", productName="RotationProbe", productReference=app_product,
           productType="com.apple.product-type.application", buildPhases=[app_sources], buildRules=[], dependencies=[],
           buildConfigurationList=settings({"PRODUCT_BUNDLE_IDENTIFIER": "org.nxkit.RotationProbe", "INFOPLIST_FILE": str(output / "Info.plist")}))
dependency = node("PBXTargetDependency", target=app)
tests = node("PBXNativeTarget", name="RotationUITests", productName="RotationUITests", productReference=test_product,
             productType="com.apple.product-type.bundle.ui-testing", buildPhases=[test_sources], buildRules=[], dependencies=[dependency],
             buildConfigurationList=settings({"PRODUCT_BUNDLE_IDENTIFIER": "org.nxkit.RotationUITests", "GENERATE_INFOPLIST_FILE": "YES",
                                             "TEST_TARGET_NAME": "RotationProbe"}))
group = node("PBXGroup", children=[app_product, test_product], sourceTree="<group>")
proj = node("PBXProject", mainGroup=group, productRefGroup=group, targets=[app, tests],
            buildConfigurationList=settings({}), compatibilityVersion="Xcode 14.0", developmentRegion="en",
            attributes={"TargetAttributes": {tests: {"TestTargetID": app}}}, projectDirPath="", projectRoot="")
with (project / "project.pbxproj").open("wb") as file:
    plistlib.dump({"archiveVersion": "1", "objectVersion": "56", "classes": {}, "objects": objects, "rootObject": proj}, file)
with (output / "Info.plist").open("wb") as file:
    plistlib.dump({"CFBundleExecutable": "$(EXECUTABLE_NAME)", "CFBundleIdentifier": "$(PRODUCT_BUNDLE_IDENTIFIER)",
                  "CFBundleName": "Rotation Probe", "CFBundleVersion": "1", "CFBundleShortVersionString": "1.0",
                  "CFBundlePackageType": "APPL", "UILaunchScreen": {},
                  "UIApplicationSceneManifest": {"UIApplicationSupportsMultipleScenes": False, "UISceneConfigurations": {"UIWindowSceneSessionRoleApplication": [{"UISceneConfigurationName": "Rotation", "UISceneDelegateClassName": "RotationProbeScene"}]}},
                  "UISupportedInterfaceOrientations": ["UIInterfaceOrientationPortrait", "UIInterfaceOrientationLandscapeLeft", "UIInterfaceOrientationLandscapeRight"]}, file)
scheme = project / "xcshareddata/xcschemes"
scheme.mkdir(parents=True, exist_ok=True)


def reference(key, name, product):
    return f'<BuildableReference BuildableIdentifier="primary" BlueprintIdentifier="{key}" BuildableName="{product}" BlueprintName="{name}" ReferencedContainer="container:Rotation.xcodeproj"/>'


app_ref = reference(app, "RotationProbe", "RotationProbe.app")
test_ref = reference(tests, "RotationUITests", "RotationUITests.xctest")
(scheme / "Rotation.xcscheme").write_text(f'''<?xml version="1.0" encoding="UTF-8"?>
<Scheme version="1.3"><BuildAction buildImplicitDependencies="YES"><BuildActionEntries>
<BuildActionEntry buildForTesting="YES" buildForRunning="YES">{app_ref}</BuildActionEntry>
<BuildActionEntry buildForTesting="YES" buildForRunning="NO">{test_ref}</BuildActionEntry>
</BuildActionEntries></BuildAction><TestAction buildConfiguration="Debug"><Testables>
<TestableReference skipped="NO">{test_ref}</TestableReference></Testables></TestAction>
<LaunchAction buildConfiguration="Debug"><BuildableProductRunnable>{app_ref}</BuildableProductRunnable></LaunchAction></Scheme>
''')
print(project)
