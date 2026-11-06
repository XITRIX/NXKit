# Interface size classes

`UIUserInterfaceSizeClass::{unspecified, compact, regular}` describes the coarse
space available along an axis. Read both values from a view or controller's
inherited `UITraitCollection`:

```cpp
void MyViewController::viewDidLayoutSubviews() {
    UIViewController::viewDidLayoutSubviews();
    const auto traits = traitCollection();
    if (!traits) return; // A detached environment may not have inherited traits yet.

    const bool wide = traits->horizontalSizeClass() == UIUserInterfaceSizeClass::regular;
    const bool shortHeight = traits->verticalSizeClass() == UIUserInterfaceSizeClass::compact;
    // Choose your layout using wide and shortHeight.
}
```

Standalone `UITraitCollection` objects default to `unspecified` on both axes.
`copy()` preserves both axes together with interface style and display scale.
The values are read-only; this change does not introduce trait overrides or
UIKit's trait collection composition/registration APIs.

## Source and updates

On **iOS**, NXKit reads the actual UIKit trait collection of the SDL window's
root controller, falling back to the native window's traits if no root exists.
UIKit supplies the device-, orientation- and multitasking-specific classification.
NXKit does not infer iPhone/iPad behavior from dimensions or apply its extra UI
scale factor to the native class values. Native `unspecified` stays unspecified.
If the native window is unavailable during startup, the portable fallback applies.

On **macOS and Switch**, the portable backend uses logical window coordinates:

| Axis | Compact | Regular |
| --- | --- | --- |
| Horizontal | greater than 0 and less than 600 points | at least 600 points |
| Vertical | greater than 0 and less than 480 points | at least 480 points |

Empty, negative or nonfinite dimensions yield `unspecified` for that axis.
The axes are independent. Pixel density does not affect classification. These
cutoffs are an intentional NXKit policy for platforms without native UIKit,
not universal Apple thresholds. Backend implementations can override
`SkiaCtx::getSizeClasses()` to supply native classifications.

The key window obtains its initial traits before root `loadView`, `viewDidLoad`
and appearance callbacks. The application loop polls traits after processing
events and before animation/layout, including frames with unchanged bounds.
This covers SDL resizes/rotation as well as native class changes that do not
produce a geometry event. A new collection is installed only when a class,
interface style, or display scale changes. `traitCollectionDidChange(previous)`
receives the stable old collection. Layout is invalidated for affected views;
there is no automatic trait dependency tracking or transition coordinator yet.

Traits propagate through ordinary views and masks, controller containment and
active presentations. Newly attached/reparented views and child controllers
inherit their current parent environment. A tiny subview retains its inherited
window classes; its own bounds do not independently classify it. Glass's copied
content appearance traits preserve the two size classes. Traversal snapshots
allow callbacks to alter the hierarchy without invalidating the iteration.
NXKit currently has one SDL-backed key window; per-scene/multiwindow native
routing and explicit subtree overrides are outside this implementation.

## Verification

`NXKit/tests/UITraitCollectionTests.cpp` covers defaults/copies, threshold and
invalid-dimension cases, initial traits before `viewDidLoad`, both rotation axes,
size changes within one class, scale/theme changes, native classifications that
contradict geometry or change at fixed bounds, views/masks, late containment,
reparenting, removal during callbacks, active presentations, layout delivery,
and Glass content appearance copies. Its headless target also executes the
production `applicationRunLoop()` with a resize event and fixed-bounds updates.

```sh
# With the Demo configured using NXKIT_BUILD_TESTS=ON:
cmake --build YOUR_BUILD --target NXKitTraitCollectionTests

# Existing headless Skia raster harness (matching headers/archive ABI required):
cmake --build YOUR_RASTER_BUILD --target TraitCollectionTests
ctest --test-dir YOUR_RASTER_BUILD -R TraitCollectionTests --output-on-failure
```

Verified 2026-10-27, working tree after `7a1273a`: the focused headless trait tests
pass, and the iOS Demo/trait test compile and link using the installed SDK.
The full headless glass/corner/gallery suite also passes. The existing navigation
regression passes with a headless CoreText/font context (its standalone executable
does not initialize a renderer context). The native iOS trait bridge is build-checked; physical rotation and
iPad multitasking were not executed on an NXKit device build. Native source
reads UIKit's values directly, while tests simulate those values at the backend
boundary. macOS Graphite and Switch builds remain blocked by absent Skia archives.

References: Apple's [size-class enum](https://developer.apple.com/documentation/uikit/uiuserinterfacesizeclass),
[trait collection](https://developer.apple.com/documentation/uikit/uitraitcollection),
and [trait-change behavior](https://developer.apple.com/documentation/uikit/adapting-your-app-when-traits-change).
