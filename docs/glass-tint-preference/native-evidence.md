# Apple system Glass tint preference evidence

Audited 2026-11-04. Environment: macOS 27.0.1 (26A434), installed macOS 27.0 SDK and iOS 27.1 SDK. All runtime probes used separate local diagnostic processes. No settings setters were called and no system preferences were changed. The iOS 27.1 simulator boot failed data migration and was shut down; no simulator or physical-iPhone runtime result is claimed here.

## Observed interface and default

The iOS 27.1 UIKit SDK stub exports these private symbols:

```
UIViewGlassGetTintAmount
UIViewGlassSetTintAmount
UIViewGlassTintAmountDidChangeNotification
$s5UIKit6_GlassV16systemTintAmountSfSgvgZ
```

The Swift symbol demangles to `UIKit._Glass.systemTintAmount: Float?`. These symbols were absent from the public UIKit headers searched. Stub location: `/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS27.1.sdk/System/Library/Frameworks/UIKit.framework/UIKit.tbd` (Swift export near line 15693; C exports near line 20300).

Disassembly of the installed MacCatalyst UIKitCore establishes the C getter's effective signature:

```objc
NSNumber * _Nullable UIViewGlassGetTintAmount(void);
```

It returns an Objective-C object, not a float. The private Swift getter checks for nil and calls the object's `floatValue`. The [retained getter disassembly](uikit-getter-disassembly.txt) shows the NSNumber cache and the Swift caller. A typed, read-only probe returned `0.5`; `NSUserDefaults` returned `0.5` for the same key.

In this Catalyst implementation, the getter reads `NSGlassTintAmount`, rounds the value to the nearest 0.05 (`round(value * 20) / 20`), and caches the NSNumber. The rounding is observed on Catalyst and has not been independently checked on iPhone. The exported NSString notification's actual value is `UIViewGlassTintAmountDidChangeNotification`.

The installed AppKit implementation of `NSGlassEffectView.Legibility.systemLegibility()` also reads `NSGlassTintAmount`. Its absent-value path loads float bits `0x3f000000`, or **0.5**. See the [AppKit getter disassembly](appkit-getter-disassembly.txt). AppKit exports private Swift cases `standard`, `increased`, and `tintAmount(Float)`; a matching public C getter was not found in the bounded SDK export search.

## Change notification path

AppKit's setup calls private `SLSCoordinatedLocalNotificationCenter()`, whose effective return type is an Objective-C notification center. It registers `addObserverForName:object:queue:usingBlock:` with the actual notification name:

```
kSLSCoordinatedGlassAppearanceSettingsChangeNotificationName
```

Its callback posts the local AppKit notification:

```
NSGlassEffectDiffusionDidChangeNotification
```

The latter is an exported AppKit NSString constant. The coordinated center is **not** `NSDistributedNotificationCenter`. Its identity was resolved with `dladdr` on the actual imported function pointer, and the name was read in the diagnostic process. See [focused notification observation](appkit-notification-observation.txt).

The implementation can either cache values with notification/activation invalidation, or poll the native cached NSNumber getter / cached `NSUserDefaults objectForKey:`. Explicit preferences synchronization and disk reads on every frame are unnecessary. Notification delivery and live changes were not exercised by modifying the user's system setting.

## Evidence and reproduction

REA inspected these shipped artifacts:

- `AppearanceIntentsExtension`: SHA-256 `71a959affaa83044752a5e63d5ace53598514379b45709768f84c9beedf12bdf`, path `/System/Library/ExtensionKit/Extensions/AppearanceIntentsExtension.appex/Contents/MacOS/AppearanceIntentsExtension`. Evidence `ev_2fbe29d8995312ae83fcce8330289b8e09fd12fd1a39bdeeb30c2d39408cfd31` observes the string “Use the Liquid Glass setting to adjust the tint level of translucent surfaces across macOS.” This establishes the user-facing tint concept, not rendering math.
- `Appearance`: SHA-256 `a1f66c2b340d98a7bd2e688003b64d3b35c84f8e1fec2dfceaffc46113649275`, path `/System/Library/ExtensionKit/Extensions/Appearance.appex/Contents/MacOS/Appearance`. Evidence `ev_5cf58356f9609e5a297a9ec7a2772325fb3faa60d2575a912433f5a9970ec4f4` finds `GlassStyleRow`; procedure-name search evidence `ev_7e23a7108957b4f749f13ae120f714a5192f6dee95659858dfb063585c7e72ff` had no Glass-named procedure matches. Neither search alone establishes a preference key or getter ABI.

REA sessions opened for these artifacts were closed. The decisive getter and notification observations came from ordinary LLDB and `dladdr` inspection of separate diagnostic processes, not REA decompilation.

Read-only SDK/export checks:

```sh
xcrun --sdk iphoneos --show-sdk-path
xcrun --sdk macosx --show-sdk-path
sw_vers
xcrun dyld_info -exports /System/Library/Frameworks/AppKit.framework/AppKit
xcrun dyld_info -section __TEXT __cstring /System/Library/Frameworks/AppKit.framework/AppKit
defaults read -g NSGlassTintAmount
```

For the native probe, a Foundation command-line executable called `dlopen` for AppKit and UIKit, resolved symbols with `dlsym`, and stopped in its own `ready()` function. The UIKit-loaded probe was built as `arm64-apple-ios27.0-macabi` against the macOS 27.0 SDK and its `System/iOSSupport/System/Library/Frameworks`, linking UIKit and Foundation. LLDB commands `disassemble -n UIViewGlassGetTintAmount` and `disassemble -n '$s5UIKit6_GlassV16systemTintAmountSfSgvgZ'` produced the retained output. A plain macOS variant provided the AppKit getter observation.

## Inferences and limits

The portable trait's 0–1 bounds and 0.5 neutral value are design choices consistent with the observed system preference and its default. We have not proved physical-iOS setter validation, all OS-version behavior, or the exact native transfer between slider position and blur radius, opacity, saturation, and tint. Renderer interpolation must be described as an approximation until separately calibrated. The runtime exports/preferences are private Apple implementation details, so a version guard, dynamic lookup, numeric validation, and a neutral fallback are required.
