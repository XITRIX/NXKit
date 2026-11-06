# Lens liquid motion comparison

[Interactive geometry replay](comparison.html) · [Native touch evidence](../native-motion.md) · [Chromatic pixel overlays](../chromatic/comparison.html)

The reference is UIKit's real tab selection, slider thumb and switch lens on
**iPadOS 27.0 build 24A434 at 2×**. XCTest supplies actual OS contacts; native
presentation geometry is observed independently. The original traces retain
contact and dispatch times, complete layer trees, source hashes and diagnostics.
This comparison isolates the lens's additional elastic affine. Native
control-owned placement and lift bounds are shared inputs; it does not claim
that NXKit reproduces a complete native tab bar, slider or switch.

## Input and reconstruction

`LensView` defaults to noninteractive. Opting in enables a visual-only response
without changing its model frame, application transform, selection or hit tests.
`InteractionSource::viewMotion` consumes global presented placement, including
ancestor movement and excluding elastic feedback. `InteractionSource::contact`
is a separate portable extension for fixed knobs; contact velocity drives it
without also adding the view's motion. Its stale input expires after 50ms.
Changing sources, disabling, hiding, removing or reparenting resets history.

Native read-only diagnostics observe a 50ms hysteresis setting, at most five
samples and directionless acceleration. Separate native debounced values exist
for horizontal/vertical scale and drift. These API observations do not recover
the full private force equation. The portable response is independently fitted
to measured output: a finite-window damped response retains unsigned strain
and signed drift separately. Reversals preserve their histories rather than
flipping a transform immediately. The affine composes scale and drift in the
observed order, while explicit bounds on scale and velocity keep input finite.
Native lift sizing and control tracking remain caller-owned. The combined response
fit uses a 0.167s response window, angular frequency 11.11 rad/s, damping ratio
0.596 and an approximately 30ms response phase. Those are fitted transfer-model
parameters; the 0.167s window is not the directly observed 50ms native getter.
There is no per-case image or time registration applied to the traces.

The optical surface, selected-source sampling, resting fill, shading and capsule
clip share the extra root transform. The tab bar applies the same geometry to
its ordinary-content punchout and leaves the full selected row stationary
underneath. Its base regular-glass effect is interactive independently, and both
observers preserve ordinary taps and selection actions.

## Results

Eight non-snapshot traces contribute **1,394 frames**. The four slider movement
paths constrain the common response; switch and tab geometries test its transfer
to different controls. The control-disable case is a diagnostic. Additional
animation/getter diagnostics and the timing-perturbed screenshot replay are
retained with the native evidence and excluded from these eight scored paths.

Position/size RMSE combines stage x, y, width and height in points. Both columns
share local lift bounds and the native ancestor presentation scale, which
includes the platter's separate response. This prevents its inflation being
incorrectly charged to lens elasticity. The reported scale/drift curves isolate
the local lens affine; its bounds are then projected into stage coordinates.

| Actual input | Rigid baseline RMSE | Elastic RMSE |
| --- | ---: | ---: |
| Slider, slow right | 0.707 | 0.083 |
| Slider, fast right | 1.757 | 0.265 |
| Slider, left, dark | 0.546 | 0.067 |
| Slider, reversal | 1.221 | 0.142 |
| Switch, slow | 0.135 | 0.042 |
| Tabs, slow | 1.734 | 0.437 |
| Tabs, reversal | 3.244 | 0.872 |
| Slider, control interruption | 0.949 | 0.523 |

All eight paths improve over an undeformed lens. Tab reversal retains a 3.793pt
maximum coordinate residual; the control-interruption diagnostic reaches
3.680pt. The transfer model does not reproduce all private debouncing and
control-specific interactions. The same constants apply across these cases;
the fixed-knob contact mode is not measured native equivalence.

## Reading the viewer

UIKit is blue, NXKit coral, and the shared placement/lift bounds gray. Recorded
bounds are drawn as capsule guides; this is a geometry overlay, not an optical
pixel comparison. The time cursor also shows horizontal/vertical stretch and
lens-local drift. “Before” removes only NXKit's extra elastic affine while retaining the
same input geometry. Absolute stage coordinates and unmodified source clocks
are retained in the downloadable CSV and metadata. The separate chromatic viewer
compares actual rendered static pixels and removes matching pixels.

## Reproduction and verification

The [retained reference](../interaction/native-reference.csv) is extracted by
`Demo/GlassComparison/export-lens-motion.py NATIVE_MOTION_DIRECTORY OUTPUT`.
It preserves local bounds, native ancestor scale, source coordinates and clocks.
Build the documented retained raster target, then run `LensMotionTests
native-reference.csv replay.csv` to exercise the production helper and export
the comparison. `build-lens-motion-report.py replay.csv OUTPUT --metadata
replay-metadata.json` rebuilds this offline viewer. The
[final provenance](../interaction/replay-metadata.json) records exact source,
reference, executable and combined UIKitCore archive hashes.

The eleven retained glass/lens suites pass against that combined archive. New
checks cover native trajectory improvements, analytic display subdivision,
reversal/release, finite extreme input, exclusive contact/view sources,
ancestor motion, source alignment beneath shear/stretch, lifecycle termination,
and real tab tap/drag/cancel routing with both glass observers. The iOS Demo and
LensView/LensRefraction/LensMotion executables build and link with SDK 27.1;
the native XCTest fixtures run on the separately identified 27.0 simulator.

## Limits

The native captures cover horizontal drags, both directions, different speeds,
reversals, holds, release and a declared control interruption. The left slider
provides dark-theme evidence; the other motion geometries are primarily light theme. No native
OS cancellation trajectory was measured: the interruption trace ends with
`UITouchPhaseEnded`. Portable cancellation has independent lifecycle tests.
Diagonal/vertical physics, physical iOS 27.2, complete native control tracking,
end-to-end touch latency and Metal/Dawn rendering/performance remain unverified.
Fine spectral footprints still differ in the static chromatic comparison.
