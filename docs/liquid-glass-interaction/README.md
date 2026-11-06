# UIKit glass merging and interaction comparison

Measured 2026-10-27 against **iPadOS 27.0**, iPad Pro 11-inch (M5) Simulator,
2× scale, iPhoneSimulator 27.1 SDK. The supplied phone screenshots identify
iOS 27.2; that runtime/device is **not** covered by this measurement. The paired
27.2 devices were unavailable through the development connection when checked.

The patch fixes mixed-material bands at joins, transforms each material in its
own coordinate system, corrects optical distance at thin bridges, and replaces
the previous one-size motion with measured UIKit response. Pressed regular
neutral/tinted lighting is calibrated independently in light and dark modes.
See the [material, seam and lighting comparison](material-and-press.md).

## Observations and fitted behavior

Real XCTest long-press/drag/release gestures drive independent native
`UIVisualEffectView`/`UIGlassEffect` instances, including a
`UIGlassContainerEffect` pair. Nothing in the reference app overrides native
feedback or injects fabricated touches. A passive `UIWindow.sendEvent` hook
records input; a display link records live presentation rectangles and layer
transforms. The public glass frame/transform remain unchanged: native feedback
is applied through a presentation wrapper.

On this runtime, **119-point squares are soft and 120-point squares are stiff**.
52, 79, 80, 81, 90, 99, 100, 101, 104, 108, 112, 116, 117, 118, 119, 120, 124,
128, 160 and 240-point squares, and tall/wide rectangles, were measured. This
establishes the sampled boundary; it does not establish an Apple API contract
for fractional sizes, other OS versions, or every shape/radius.

The inferred policy uses the longest side to choose the mode and the shortest
side to resolve large-mode inflation/resistance. A shortest-side weight is
`clamp((shortest - 44) / 116, 0, 1)`.

| Geometry | Measured steady longest-side growth | Mode |
|---|---:|---|
| 52×52, 80×80, 119×119 | 16 points | Soft |
| 120×120 | 8.13793 points | Stiff |
| 160×160, 240×240 | 4 points | Stiff |
| 120×52, 160×52, 240×52 | 15.17241 points | Stiff |
| 160×100 | 10.20690 points | Stiff |

Large-mode growth interpolates 16→4 points with the shortest-side weight.
Small press spring frequency is about 24 rad/s with damping ratio .625;
release is about 15.708 rad/s with damping ratio .375. The large response
transitions to a slower press and a stiffer, more damped release. Drag follow
is fitted separately; release shares the inflation spring. These constants are
empirical fits, not recovered UIKit source or private API declarations.

Each pull axis is independent. For a dimension `s` and absolute finger delta
`d`, the fitted soft target displacement is:

```
s * (d + 3 * max(d - s/2, 0)) / 8000
```

Large mode divides that by `3 + 9 * shortestSideWeight`. The original delta
supplies its sign. The resistance changes continuously in displacement at the
edge; diagonal input does not divide either axis by radial magnitude. Stretch
is centered, with opposite relative changes on the two axes. Translation is
applied before scale. This replaces the former opposite-edge anchor.

## Retained regression and acquisition limits

[`uikit-reference.txt`](uikit-reference.txt) retains **53 native time series**:
actual touch events and target rectangles, not expected values generated from
NXKit. [`native-fits.json`](native-fits.json) retains independently reported
press/release fits. [`cases.json`](cases.json) recreates every native case.
The production `GlassInteraction` replay checks all recorded frames, plus
60/120 Hz equivalence, interrupted presses, cancellation, resizing, diagonal
input and exact restoration of app geometry.

Event dispatch and compositor observation are delayed by variable amounts.
Each fixture explicitly retains the fitted acquisition offsets: press and
release are aligned separately, while move observation uses a fixed 22 ms
hardware-time / 6 ms dispatch-time offset. **These delays are not added to
production motion.** This is a geometry/curve comparison, not an end-to-end
latency benchmark. Several early traces lack dispatch timestamps; newer traces
record both clocks. A stalled 160×160 press is too sparsely sampled to identify
all spring parameters, so its alignment uses independently measured large-mode
parameters and is marked `undersampled` in the fit report.

An older logger keyed layers by tree path. UIKit temporarily changes wrapper
hierarchy, producing one invalid local-space rectangle in each of four paired
traces. Extraction omits those four samples explicitly, records the omissions
in the fixture, and never substitutes fabricated geometry. The current logger
uses object identity. Remaining frames are retained without resampling or
smoothing. Hierarchy-rendered optical snapshots capture the visible materials
in this fixture, but are not a substitute for Metal framebuffer validation.

Current replay: worst per-case four-component RMS error **0.08276 points**,
largest individual component error **1.14422 points**. The regression bounds
are .12 RMS and 1.25 maximum points, including busy-simulator acquisition
outliers. See [`replay-metrics.json`](replay-metrics.json). REA controlled-replay
evidence `ev_c879516d58f3db051eb708dc1502a1d7cefceaa11d05adc6bd9f83cde85813cb`
observed the production replay exit successfully; it does not itself observe
the earlier native test execution. Native XCTest runs completed with zero
failures for the 26-case sweep, 15 boundary/pull cases, 8 final boundary/lighting
cases and 4 fixed-input lighting cases.

Native private join absorption, resting tint color, exact fractional-boundary
policy, arbitrary shear/rotated blur covariance, three-plus-item merge ordering,
and end-to-end Metal/Dawn performance remain partial. These results do not
claim pixel-perfect parity with the supplied iOS 27.2 device screenshots.

## Verification

All six production-core raster targets pass: `LayerTests`, `CornerTests`,
`MaterialTests`, `ContainerTests`, `GalleryTests` and `InteractionTests`. The
iOS Demo plus material/interaction/gesture/container targets build and link;
the interaction fixture is present in its iOS test bundle. The finalized native
UIKit app/XCTest fixture also builds for testing. NXKit Metal execution and
Switch/macOS Graphite GPU builds are not verified by these raster results.

## Reproduction

Requires an iPad Simulator with iOS 26+ (720-point-wide stage), Xcode, Python,
and NumPy for fitting / Pillow for optical comparisons. Run from the repo root:

```sh
python3 Demo/GlassComparison/build-interaction.py /tmp/nxkit-interactions docs/liquid-glass-interaction/cases.json
xcodebuild -project /tmp/nxkit-interactions/Interaction.xcodeproj -scheme Interaction -destination 'platform=iOS Simulator,id=DEVICE_ID' -derivedDataPath /tmp/nxkit-interactions/build -parallel-testing-enabled NO test
xcrun simctl get_app_container DEVICE_ID org.nxkit.GlassInteractionReference data
```

Copy that app's `Documents` directory to `/tmp/native-interaction-captures`.
Keep the raw trace/snapshot directory if investigating acquisition artifacts.
Compact fixtures can then be regenerated without changing native images:

```sh
python3 Demo/GlassComparison/analyze-interactions.py /tmp/native-interaction-captures --output /tmp/native-fits.json
python3 Demo/GlassComparison/extract-interactions.py /tmp/uikit-reference.txt /tmp/native-interaction-captures --fits /tmp/native-fits.json
cmake --build RASTER_BUILD --target InteractionTests MaterialTests ContainerTests GalleryTests
ctest --test-dir RASTER_BUILD -R '^(InteractionTests|MaterialTests|ContainerTests|GalleryTests)$' --output-on-failure
RASTER_BUILD/InteractionTests /tmp/uikit-reference.txt /tmp/replay.csv
python3 Demo/GlassComparison/summarize-interaction-replay.py /tmp/replay.csv /tmp/replay-metrics.json
```

`RASTER_BUILD` is the matching-Skia build described in the
[comparison guide](../../Demo/GlassComparison/README.md). The normal NXKit
test option also registers `NXKitGlassInteractionTests`; iOS bundles its native
fixture as a resource. Native optical replay tools and measured geometry
injection are described in the [material report](material-and-press.md).

Apple documents scaling and bouncing for
[`isInteractive`](https://developer.apple.com/documentation/uikit/uiglasseffect/isinteractive),
but does not document the size threshold or physical spring constants.
