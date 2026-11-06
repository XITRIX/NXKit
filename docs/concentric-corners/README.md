# Concentric corners — 2026-11-03

NXKit implements the iOS 26 `UICornerRadius` and `UICornerConfiguration` value
APIs in Core, together with `UIView::cornerConfiguration()`,
`setCornerConfiguration()` and `effectiveRadius(UIRectCorner)`.

```cpp
view->setCornerConfiguration(UICornerConfiguration::corners(
    UICornerRadius::containerConcentric(12)));
view->setClipsToBounds(true); // also clips descendants
const auto radius = view->effectiveRadius(UIRectCorner::topLeft);
```

Fixed radii, independent corners, capsules with optional maximum radii, uniform
corners, and the four uniform-side factories are available. A combined
`UIRectCorner` mask queries the maximum selected radius. Missing optional corner
values mean square corners. Uniform groups use the largest resolved member.
Fixed and minimum radii are finite, nonnegative logical points; invalid values
throw `std::invalid_argument`.

The C++ API uses Swift's short factory names where they are unambiguous. It also
provides the Objective-C factory selector stems. C++ cannot distinguish Swift's
two `uniformEdges` overloads by argument labels; use
`configurationWithUniformTopRadius(top, bottom)` or
`configurationWithUniformLeftRadius(left, right)` for those two factories.
`effectiveRadiusForCorner()` is an alias for `effectiveRadius()`.

An explicitly configured ancestor establishes a corner container, including an
explicit square configuration. Unconfigured wrappers are transparent to corner
resolution. Legacy `CALayer::cornerRadius` neither establishes a configuration
container nor changes `effectiveRadius`, matching the native oracle. Radius
resolution accounts for bounds origins and model affine placement, subtracts
the larger corner-edge inset, applies a requested minimum, then clamps to half
the shorter local dimension. Geometry, configuration, reparenting and layout
refresh the retained result. Detached views immediately resolve to their minimum;
native UIKit may retain its last attached cached result until another layout.

Backgrounds, borders, shadows, own layer output and optional descendant clipping
use the same independent continuous/circular geometry. Presentation copies and
four private scalar animation channels support implicit configuration animation.
XML accepts `cornerConfiguration="containerConcentric"`, `"capsule"`, or a
nonnegative numeric fixed radius; unsupported values fail attribute parsing.

## iOS window bridge

The native bridge samples a transparent, noninteractive view attached to the
SDL window's root controller. On iOS 26+, its public
`UICornerConfiguration` / `UICornerRadius` / `effectiveRadiusForCorner:` calls
resolve each window corner independently. On older iOS, the explicitly requested
private `_displayCornerRadius` selector is queried on that window's `UIScreen`,
with availability checking and a square fallback. No private selector is used
in the iOS 26+ branch. The bridge uses points, and the renderer applies NXKit's
extra UI scale once; display pixel density does not multiply the radius.
macOS and Switch return square platform corners by default. Embedding contexts
may supply their own four values through `SkiaCtx::windowCornerRadii()`.

Push/pop animations and interactive pops clip controller wrapper views as cards.
They resolve the stationary container's corners once and retain those radii
through translation, reversal and settlement. Completion/cancellation restores
unconfigured wrapper corners, allowing descendant concentric views to find the
original ancestor again. Controller-owned transforms, layers and clipping remain
application-owned. Nonanimated stack changes do not apply card styling.

## Verification and remaining work

`UIKitCore/tests/CornerConfigurationTests.cpp` covers native-measured asymmetric
insets, square/legacy/configured wrappers, translations, uniform grouping,
capsules, empty/invalid inputs, reparenting, platform changes, XML, presentation
animation, and independent outlines/clipping at 1×/2×/3×. Navigation and interactive
navigation tests check independent screen radii on moving cards and cleanup after
push, pop, commit and cancellation. Existing corner raster tests cover the legacy
geometry path. The headless Core-only project includes the new test too.

`Demo/GlassComparison/ConcentricReference.mm` is a separate native UIKit oracle
that also compiles the actual production `IOSWindowGeometry.mm` bridge. Retained
JSON records simulator executions on iOS 18.5/iPhone 16 Pro and iOS 27.0/iPhone 17
Pro. Both production branches return 62-point corners; the modern branch equals
the independent public concentric probe. The modern measurements establish the
asymmetric-inset, uniform, nil-corner, fixed-clamp and wrapper rules used above.
Build the fixture with `build-concentric.py`, install/launch its app with simctl,
then read `Documents/concentric.json` from its data container.

Exact rotated/nonuniformly scaled/3D container semantics and native dependency
tracking from arbitrary layout callbacks remain unverified. Explicit layer-only
bounds animations do not establish new view configuration endpoints. Uneven
standalone backdrop outlines are clipped correctly, but material optical normals
still use a uniform minimum radius; merged glass materials do not yet consume
independent corner configurations. Navigation radii are captured at transaction
start; mid-transition screen-shape changes need native visual comparison.
The Core-only build passes both its module boundary test and the new configuration
test. Four focused full-composition suites pass. The iOS Demo compiles/links with the standard offline Metal bake enabled; its
embedded catalog ABI and 68 libraries match the new source catalog.
The refreshed Metal source catalog contains 68 libraries and 55 native pipeline
descriptions, retaining the earlier content-addressed raw-state superset. Actual
Graphite/Metal runs render 17,497 full-window frames per profile (dark/3× and
light/2×), exercising rounded automatic push/pop and interactive commit/cancel.
Both profiles complete their last two passes with zero new pipeline compilations.
Raw timing traces and source hashes are in `verification.json` and
`metal-{dark,light}.csv.gz`; concurrent compilation makes these diagnostics,
not isolated performance measurements. GPU binaries are rebuilt for the new ABI.

Physical-device visual parity and floating iPad/external-display behavior remain
to be checked.

Sources: [UICornerConfiguration](https://developer.apple.com/documentation/uikit/uicornerconfiguration-swift.struct),
[UICornerRadius](https://developer.apple.com/documentation/uikit/uicornerradius-swift.struct),
[effectiveRadius(corner:)](https://developer.apple.com/documentation/uikit/uiview/effectiveradius(corner:)),
and the installed UIKit iOS 27.1 SDK public headers (APIs available since iOS 26).
