# Process-local iOS 27 glass comparison control

Observed on 2026-11-04 with the installed **iOS 27.1 simulator, build 24A94401**, in the disposable comparison simulator `10E1CE1C-81AB-47AA-B1EF-7C72589D5CCB`. This supplements the older Catalyst findings in [native-evidence.md](native-evidence.md); Catalyst's preference key and no-op setter must not be assumed to describe iOS.

## Findings

Static inspection of code bytes read from the loaded simulator UIKitCore established:

```objc
NSNumber * _Nullable UIViewGlassGetTintAmount(void);
void UIViewGlassSetTintAmount(NSNumber * _Nullable amount);
NSUserDefaults *_UIKitUserDefaults(void);
```

The getter calls `_UIKitUserDefaults()` and reads **`UIViewGlassTintAmount`**. It calls `doubleValue`, rounds to the nearest 0.05, constructs an `NSNumber`, and caches the result. The setter likewise takes an object, then calls `setObject:forKey:` or `removeObjectForKey:` and `synchronize`. The setter was **never invoked** during this investigation; its persistence behavior is a static observation of its calls, not an observed settings change.

Runtime probes observed that `_UIKitUserDefaults()` returns an `NSUserDefaults` instance **distinct from** `standardUserDefaults`. Installing a volatile `NSArgumentDomain` on that specific instance, before the getter is first used, produced the following results in three fresh processes:

| Process-only value | Actual native getter |
| --- | --- |
| `0` | `0` |
| `0.5` | `0.5` |
| `1` | `1` |

`NSGlassTintAmount` on `standardUserDefaults` was absent in all three probes. No persistent defaults or system settings were written.

## Fixture helper

Call this at the beginning of `main`, before `UIApplicationMain`, creating materials, or calling the native getter. Launch a fresh process for each amount, because changing a volatile default after the getter has cached a value is insufficient.

```objc
#import <Foundation/Foundation.h>
#import <dlfcn.h>
#import <math.h>

static BOOL SetComparisonGlassAmount(float amount) {
    typedef NSUserDefaults *(*DefaultsGetter)(void);
    typedef NSNumber *(*TintGetter)(void);
    DefaultsGetter defaultsGetter =
        (DefaultsGetter)dlsym(RTLD_DEFAULT, "_UIKitUserDefaults");
    TintGetter tintGetter =
        (TintGetter)dlsym(RTLD_DEFAULT, "UIViewGlassGetTintAmount");
    if (!defaultsGetter || !tintGetter || !isfinite(amount) ||
        amount < 0 || amount > 1) return NO;

    NSUserDefaults *defaults = defaultsGetter();
    NSMutableDictionary *domain =
        [[defaults volatileDomainForName:NSArgumentDomain] mutableCopy];
    if (!domain) domain = [NSMutableDictionary dictionary];
    domain[@"UIViewGlassTintAmount"] = @(amount);
    [defaults setVolatileDomain:domain forName:NSArgumentDomain];
    NSNumber *actual = tintGetter();
    NSLog(@"glass comparison: requested=%g actual=%@", amount, actual);
    return actual && fabsf(actual.floatValue - amount) < 0.0001f;
}
```

This is private API used by the comparison fixture, not an application-facing dependency. The fixture must reject captures when the getter fails to confirm its requested amount. For intermediate values, account for native 0.05 quantization when validating readout.

## Exact probe reproduction

The diagnostic source and its disassembly are retained locally under `/private/tmp/nxkit-native-glass-control/`. The executable is a separate Foundation/UIKit command-line process, with no application UI. The following commands were run successfully:

```sh
xcrun --sdk iphonesimulator clang++ -std=c++20 \
  -target arm64-apple-ios27.1-simulator \
  -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator.sdk \
  -framework Foundation -framework UIKit \
  /private/tmp/nxkit-native-glass-control/probe.mm \
  -o /private/tmp/nxkit-native-glass-control/probe

xcrun simctl spawn 10E1CE1C-81AB-47AA-B1EF-7C72589D5CCB \
  /private/tmp/nxkit-native-glass-control/probe 0
xcrun simctl spawn 10E1CE1C-81AB-47AA-B1EF-7C72589D5CCB \
  /private/tmp/nxkit-native-glass-control/probe 0.5
xcrun simctl spawn 10E1CE1C-81AB-47AA-B1EF-7C72589D5CCB \
  /private/tmp/nxkit-native-glass-control/probe 1
```

The runtime mapped UIKitCore at `0x1c3c00000`; getter `0x1c4434ac4`, setter `0x1c4434c38`. `dladdr` resolved the getter's calls to `_UIKitUserDefaults`, `objc_msgSend$objectForKey:`, `objc_msgSend$doubleValue`, and `objc_msgSend$numberWithFloat:`. The actual getter key was read as an Objective-C string from its ADRP/ADD constant reference, yielding `UIViewGlassTintAmount`. Setter calls resolved to `objc_msgSend$setObject:forKey:`, `objc_msgSend$removeObjectForKey:`, and `objc_msgSend$synchronize`. Wrapper disassembly files preserve the actual loaded instruction bytes; their displayed relative target addresses are wrapper-relative.

The simulator's boot-status command reported a migration failure, but these UIKit processes spawned and ran successfully. These observations establish native preference control and readout on this simulator. They do not establish the physical-device transfer curve, appearance, or live preference invalidation; those require the rendered reference captures.
