# Disabled scroll edges and UIKit bottom default

Audited 2026-11-04, working tree after `af6b8fc`.

```cpp
scroll->bottomEdgeEffect()->setStyle(UIScrollEdgeEffectStyle::none);
// Opt back into an explicit bottom material:
scroll->bottomEdgeEffect()->setStyle(UIScrollEdgeEffectStyle::soft);
```

`none` disables blur, tint, separators and content attenuation without changing
the independent `isHidden` property. It works on every edge in both component
kits. Changing from an active effect to none removes its existing visual result
on the next render; setting it repeatedly does not invalidate again.

Core edge metadata remains automatic/non-hidden by default. UIKit resolves
**automatic bottom to none**, matching the requested iOS 27 presentation; top
and other attached automatic edges resolve to soft. Explicit bottom soft/hard
remain supported. NXKit automatic still uses its shared hard material; explicit
none disables it. The choice belongs to the component kit, independently of OS.
No defaults are installed into Core or rewritten as controllers are reparented.

The full-screen iOS Demo's Appearance → Scroll edge row now cycles Automatic,
Soft, Hard, None. Reset returns to Automatic (top soft, bottom none in UIKit).
The portable scroll comparison CLI also accepts `none`.

The iOS 27.1 SDK's public `UIScrollView.h` declares only automaticStyle, softStyle
and hardStyle on UIScrollEdgeEffectStyle, plus the separate hidden flag.
Consequently none is documented as a **portable extension**, rather than an
Apple UIKit API that exists in those headers. Public reference:
[UIScrollEdgeEffect.Style](https://developer.apple.com/documentation/uikit/uiscrolledgeeffect/style-swift.class).
The bottom policy follows the user's requested iOS 27 behavior; the existing
soft-kernel calibration remains the earlier iOS 26 target.

Focused regressions cover stable Core metadata/validation/no-op invalidation,
UIKit automatic resolution, explicit bottom styles, all-edge none with attached
chrome, exact none/hidden raster equivalence in both themes, immediate removal
of cached blur/fading, independent visibility, NX automatic hard restoration,
and all four Demo setting choices. Existing live-safe-area and mask/kernel tests
explicitly request soft when testing a bottom effect.

## Verification

Core, NX navigation, UIKit edge and full-screen Demo regression executables pass;
UIKit's edge suite runs 213 checks. Normal macOS and signed iOS builds and the
Switch NRO pass. The iPhone app is installed; launch was blocked by the locked
phone, so no new physical-device runtime result is claimed.

A fresh 7,310-frame capture covers six 1×/2×/3× light/dark edge profiles and two
leading-item push/pop navigation probes. Its 90 observed MSL sources are an
exact subset of the prior calibrated catalog: this changes CPU style resolution,
not shader source. All 137 libraries and 110 pipelines are freshly rebuilt for
ABI `087b3dee045c889ddb4ab269017f093dd2e82ed16fc5d84aa3abefc0f518e9e9`.
The matching archive-only replay passes all 7,310 frames with zero runtime library
or pipeline compilations. Prior broader tint coverage retains its original ABI
provenance. See [verification summary](verification/summary.json) and compressed
frame/build logs in `verification/`.
