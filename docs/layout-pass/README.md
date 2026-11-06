# Layout and first-frame audit — 2026-11-02

Startup and resize frames now finish layout before generating view contents.
A focused raster regression reproduces the original first-frame paint mismatch
and stale dimensions across consecutive resize frames. It verifies that the
first two presented framebuffers are identical without an intervening change.

NXKit uses Yoga flex layout; this is not Apple's constraint-based Auto Layout.
Apple's [`layoutIfNeeded`](https://developer.apple.com/documentation/uikit/uiview/layoutifneeded%28%29?language=objc)
requires pending layout to resolve immediately, and
[`layoutSubviews`](https://developer.apple.com/documentation/uikit/uiview/layoutsubviews%28%29?language=objc)
is the view's subview-layout override. Those are the public lifecycle targets
for this change. Yoga's own [pixel-grid implementation](https://github.com/facebook/yoga/blob/main/yoga/algorithm/PixelGrid.cpp)
defines display-scale rounding, including outward rounding for measured nodes.

## Causes and corrections

The old `drawAndLayoutTreeIfNeeded` regenerated each view's contents before
running layout. A layout callback could therefore correct model geometry only
after that frame's bitmap had been generated. Layout and display are now
separate phases, with the complete layout subtree resolved before drawing.

Yoga wrote sizes directly into `CALayer`, bypassing `UIView`'s layout/display
and safe-area invalidation. Already clean manual descendants could retain the
old size. Yoga still assigns placement independently of transforms, but sends
changed bounds through `UIView::setBounds`, refreshes safe areas after movement,
and applies its output only to participating Yoga children. Manual children
keep their own layout.

`layoutIfNeeded` previously handled only the receiver and an ancestor Yoga
root, without resolving its complete subtree. Controller callbacks that changed
styles after calculation could leave another pass pending for the next frame.
The new parent-first walk resolves callbacks, inherited margins and safe areas,
then revisits pending work before drawing. Reentry is guarded, hierarchy
membership is snapshotted, and self-invalidating callbacks have an eight-pass
limit with pending work retained. Automatic preparation defers invisible
branches; explicit `layoutIfNeeded` can lay them out. Visibility follows the
presentation layer, so a view fading toward model alpha zero is still laid out
while visible.

The old Yoga adapter cached the first display scale forever and also left Yoga's
own pixel rounding at its default 1×. Each node now owns a config using its
current display scale, avoiding the frozen scale and a second rounding pass.
Focused tests switch 1×→2×→3×→2× and exercise fractional UI scale changes.

Animation presentation layers were copied before layout. An unrelated opacity
or transform animation could consequently preserve an old size/position even
when the model had the correct new layout. The frame samples animations again
after layout using the **same timestamp**, refreshing unanimated geometry
without advancing animation time. Newly generated bitmap contents and an
unanimated contents density also reach the existing presentation immediately;
an explicitly animated density keeps its interpolated value.

Initial window layout now completes between attaching the root view and
`viewDidAppear`/initial focus. Window layout also tolerates a missing renderer
in geometry-only harnesses. The navigation regression now supplies the font
service required by actual initial chrome measurement.

The audit also found that `NXRect::operator!=` returned equality. It now returns
its complement, with a regression for changed and unchanged rectangles. This
allows changed-bounds detection to work reliably. The existing positive-inset
expansion convention of `NXRect::insetBy` is unchanged and remains different
from UIKit inset semantics.

## Frame order

1. Capture platform geometry, then install window bounds and traits.
2. Sample animations for this frame.
3. Resolve layout, safe areas, inherited margins and callback invalidations.
4. Refresh presentation geometry at the same animation timestamp.
5. Regenerate dirty view/layer contents, then composite and present.

Continuous rendering and the earlier native live-resize/rotation geometry
bridge remain in place. This change fixes layout and content ordering rather
than delaying presentation until a later tick.

## Verification

`LayoutPassTests` exercises actual application-frame rendering and checks:

- first paint uses final nested layout, with identical first/second pixels;
- root appearance sees laid-out descendants;
- 24 consecutive portrait-to-landscape size/safe-area samples;
- UI scale factors .75, 1.25 and 1 in consecutive frames;
- manual descendants under Yoga, including already clean subtrees;
- unrelated animation bounds and regenerated animated bitmap contents;
- visible fade-out layout and preservation of animated contents density;
- fixed-child safe-area changes when only its parent resizes;
- inherited margins, hierarchy mutation, hidden/revealed content and reentry;
- 1×/2×/3× display rounding and rectangle inequality.

Fourteen raster suites pass. After the final Yoga hierarchy-snapshot adjustment,
the focused layout and navigation/tab suites were rebuilt and passed again.
Related coverage includes navigation/tab containers, adaptive presentation,
application scale/traits, glass screens, tab lenses, scroll physics/indicators,
gestures, text rendering, lens refraction and layer sampling. Native Cocoa
resize/expose/wake tests pass. The iOS Debug Demo builds and links.

The 96-card/eight-horizontal-rail stress replay still presents all 64 frames
and delivers its action with no failures. The two-phase traversal has a measured
CPU cost: the local null-recording run's warm median preparation increased
from about 0.60 ms to 0.90 ms across 1,665 views, with total frame time about
4.34→4.99 ms. These are busy-host CPU figures, not GPU or thermal measurements.
Cold-frame timing is not comparable because initial layout now occurs inside
`makeKeyAndVisible` before the first recorded frame.

Raw validation logs and timing traces are retained alongside this report.
The host raster build uses `Demo/GlassComparison/RasterTests`, matching Skia
raster headers/archives and its controller-test option. The focused target is
also registered as `NXKitLayoutPassTests` under `NXKIT_BUILD_TESTS`.

```sh
cmake --build /tmp/nxkit-container/raster-tests --target LayoutPassTests
/tmp/nxkit-container/raster-tests/LayoutPassTests
ctest --test-dir /tmp/nxkit-container/raster-tests -R 'LayoutPassTests|ApplicationScaleTests|TraitCollectionTests|NavigationTests|TabControllerTests|AdaptiveTabTests|GlassScreenTests|LensTabTests|TextRendererTests|ScrollPhysicsTests|ScrollIndicatorTests|GestureTests|LensRefractionTests|LayerSamplingTests' --output-on-failure
```

The build path is the configured local comparison build, not a required
repository location. Physical-device first-frame/rotation video and Switch
execution remain unverified. A full macOS Metal Demo build still requires the
missing `Submodules/skia/out/mac-arm64/libskia.a`; native Cocoa resize testing
here uses the available raster archives. Deliberately non-convergent layout
callbacks can leave work pending after the bounded retry limit. This remains
Partial UIKit lifecycle coverage, not complete constraint-layout compatibility.
