# UISlider native reference

The native-only fixture in `Demo/SliderComparison` uses Apple's public slider,
control, action, and XCTest APIs. It does not replace UIKit methods or supply
synthetic UITouch objects. The implementation target is Apple's public
[UISlider API](https://developer.apple.com/documentation/uikit/uislider), with
[UIControl events](https://developer.apple.com/documentation/uikit/uicontrol).
Private class names in captured view trees identify composition, not an
implementation to link into NXKit.

## Observed defaults and geometry

Fresh captures run iOS 27.0 (24A434), with the iPhone Simulator at 3× in light
and dark appearance. `native/{light,dark}/geometry.json` retains the complete
public geometry outputs and native view trees; corresponding PNGs are UIKit's
rendered output. The supplied bounds cover four widths and four heights.

| Property | Native observation |
| --- | --- |
| Initial value / range | 0 / 0...1 |
| Continuous | true |
| Intrinsic size | no intrinsic width, 34pt height |
| sizeThatFits(300,100) | (300,34), including endpoint/custom-thumb images |
| Default track | full available width, vertically centered, 6pt high |
| Thumbless track | vertically centered, 7pt high |
| Default thumb | 37×24pt, opaque white in light and dark |
| Disabled | visual subtree alpha 0.5; geometry unchanged |
| Selected / programmatically highlighted | no geometry change or contact lift |
| Endpoint images | natural size, vertically centered; 11pt spacing to track |
| Custom thumb | natural image size controls resting thumb geometry |
| Resting shadow | black, opacity .14, radius 5pt, offset (0,1), no shadow path |
| Maximum-track background | black at alpha .1 in light, white at alpha .1 in dark |

For a nondegenerate range the public thumb-rect position is rounded to integral
points after interpolation through `trackWidth - thumbWidth`. Minimum and
maximum range crossings collapse both endpoints and the current value to the
newly assigned endpoint. A zero-length range places the thumb at the leading
end. See the raw bounds and image rectangles rather than assuming a reference
device width or safe area.

The latest `native/layer-geometry` sweep retains public layer shadow and color
properties. None of the native lens or resting-view ancestors clips the resting
shadow. The native white resting child reports a continuous corner curve and
zero public corner radius; this does not identify the private shape operation.
Explicitly changing value .25 to 0 or 1 produces zero or full-width minimum
track fill, respectively. Intermediate fill uses the unrounded thumb center.

The normal custom thumb image is returned by `thumbImageForState` for normal,
highlighted, and disabled states, and remains visible in those states. This
runtime's new glass visual element returns nil from `currentThumbImage` even
with a configured, visible normal image; that getter gap is recorded, rather
than proposed as a useful portable API behavior.

The public five-tick configuration defaults to positions
`[0, .25, .5, .75, 1]`, `allowsTickValuesOnly=true`, `neutralValue=0`, and enabled
range `0...1`. These observations corroborate the
[public configuration initializer defaults](https://developer.apple.com/documentation/uikit/uislider/trackconfiguration-swift.struct/init%28allowstickvaluesonly%3Aneutralvalue%3Aenabledrange%3Aticks%3A%29).

## Control dispatch

The native `control.json` records establish these public action behaviors:

* A single UIAction registered for two event bits is invoked once when both
  bits are passed to one `sendActions` call.
* Explicit `sendActions` still dispatches when `enabled` is false or when user
  interaction is disabled.
* Adding a new UIAction with the same identifier replaces the handler while
  preserving its previously registered event bits and adding the new bits.
* Removing only one event bit preserves the other registrations for that action.
* Setting enabled=false preserves explicitly set selected/highlighted flags:
  selected=true, highlighted=true, disabled=true produces state 7.
* Explicit `performPrimaryAction` dispatches while disabled.
* Disabled UIControl and UISlider return nil from hit testing; the slider result
  is also confirmed in an attached native window.

These are explicit action-dispatch probes. They do not establish raw touch
delivery for disabled controls or assert arbitrary callback ordering under
reentrant mutation.

## LiquidLens motion

The [lift / resting-material follow-up](lift-motion/README.md) supersedes the
initial generic lift timing and missing control-specific blur policy. It retains
frame-by-frame UIKit traces, REA settings evidence and before/after motion.


Existing independent real-touch
[native lens traces](../liquid-glass-lens/native-motion.md) measure a 37×24pt
resting slider thumb that expands to 57.35×37.20pt during contact (1.55×).
Its presentation transform additionally stretches, compresses, and rings with
motion, including after release. Fast motion produces a larger response than
slow motion. Lift geometry and dynamic deformation are separate effects.

Fresh OS-touch tests observe that a touch well outside the thumb is rejected:
pressing or dragging from track position x=240 on a 300pt slider at value .25
returns false from `beginTracking`, without moving the value. Center and
offset contacts on the thumb are accepted. This is not jump-to-track behavior.
The fixture records native method results; it never calls tracking methods
to emulate touches.

`native/calibration`, `native/velocity`, and `native/low-velocity` retain fresh
actual OS touches, per-event actions, display-link values/lens geometry, and
public native `UIPanGestureRecognizer.velocity(in:)` readings. In the tested
positive 80pt drags, preserving a 9pt off-center grab produces the same value
as a centered grab. The first 10pt of movement is consumed as a deadband, then
the value advances by the remaining displacement divided by the full 300pt
control width: `.25 + (80 - 10) / 300 = .4833333`. This establishes those paths,
not an arbitrary transformed or bidirectional tracking proof.

The native pan velocity remains nonzero during a stationary hold. At release,
UIKit projects that retained velocity, then continues emitting valueChanged
while tracking=true and highlighted=false. Native endTracking runs after that
retirement; touchUpInside observes tracking=false, followed by a final
valueChanged. Noncontinuous mode emits only that final valueChanged.

The retained noncontinuous case is an event-order reference only: coarse native
input delivery made its terminal moved and ended samples share a timestamp,
and some public value/display-link updates ran after end dispatch. Its summary's
last pre-end display value must not be interpreted as a fixed value during a
noncontinuous drag, or used to fit release physics. The trace itself shows
updates without intermediate valueChanged callbacks.

| Measured native pan velocity (pt/s) | Measured release displacement (pt) | `.099 × velocity` (pt) |
| --- | --- | --- |
| 32.548734 | 0 | 3.222325 |
| 60.000000 | 0 | 5.940000 |
| 122.000000 | 12.077996 | 12.078000 |
| 144.869455 | 14.342085 | 14.342076 |
| 311.036518 | 30.792627 | 30.792615 |

**Observed:** the nonzero projections match the `.99` deceleration projection
`v × .99 / (1000 × (1 - .99))`, to less than .00002pt in these captures.
**Inferred:** the exact activation cutoff is somewhere above 60 through 122pt/s;
100pt/s is a portable reconstruction choice, not an extracted constant. The
native pan velocity estimator itself is not identified by this test. Authored
XCTest movement speed must not be substituted for the measured native velocity.

A 30×40 custom-image thumb keeps exactly that model size throughout contact
and release; it does not get the default thumb's 1.55× lift inflation. It still
uses native tracking and release projection. No-motion contact returns the
same value after release.

The captured PNGs have Display P3 profiles. Nine clean native resting, contact,
and released screenshots are color-converted to sRGB under `native/srgb`, with
source/output hashes and profile metadata in its manifest. Screenshot runs
explicitly record capture start/end times and are excluded from motion-law
estimation because drawing perturbs timing.

The public tick-configuration default values are captured, but this runtime's
glass visual element shows no tick marks or changed neutral fill in the bounded
configuration cases. These observations do not establish all tick interaction
semantics or justify claiming pixel parity for a portable tick implementation.

## Evidence and reproduction

`native/rea-geometry-evidence.json` retains REA evidence
`ev_3b3ea3a67c7b0559aabc2fc27e6526db60914d5f1b3f40c557fd1de32dacd917`.
Its process driver exited successfully and retained geometry, screenshots,
source hashes, and executable hash. The REA record is marked truncated overall;
filesystem checkpoints are before/after snapshots, UIKit is launched separately
by CoreSimulator, process sampling can omit short-lived children, and inherited
environment/network activity is not fully observed. This is runtime output,
not static binary disassembly.

`native/rea-control-api-evidence.json` records successful, untruncated native
public-control execution as
`ev_fc1c813d7f73356c07e1ef2adddf5e1341028f2ba64e3c8042eec060ebc707cf`.
This command-line query makes no window or pixel claims; attached-window
geometry independently confirms disabled slider hit testing.

The XCTest capture driver and raw test logs report successful native runs.
REA's wrapper around the first long XCTest run returned `cleanup_incomplete`,
so no successful REA execution Evidence ID is claimed for that run. Subsequent
bounded artifact verification is retained in `native/verification.json` and
`native/rea-artifact-verification.json`, Evidence
`ev_b33542941ec67c3dd3edb62e718cd001e88a0c261829a8a332fb3080fe9d1af3`.
It exited successfully and verifies retained values, raw hashes, projection
math, and color conversions; it does not retroactively observe UIKit execution.
That verifier record is marked truncated overall. A read-only REA doctor check
found the Codex registration and skill aligned; no setup/configuration changes
were made.

```sh
/usr/bin/python3 Demo/SliderComparison/capture.py DEVICE /private/tmp/slider-geometry
/usr/bin/python3 Demo/SliderComparison/capture-touch.py DEVICE /private/tmp/slider-touch
/usr/bin/python3 Demo/SliderComparison/capture-touch.py DEVICE /private/tmp/slider-calibration --calibration
/usr/bin/python3 Demo/SliderComparison/capture-touch.py DEVICE /private/tmp/slider-velocity --velocity
/usr/bin/python3 Demo/SliderComparison/capture-touch.py DEVICE /private/tmp/slider-low-velocity --low-velocity
/usr/bin/python3 Demo/SliderComparison/verify.py
```

These scripts require an already booted dedicated Simulator and an installed
Xcode SDK. They do not reset Simulator services, change persistent system
settings, or use a physical device. Early scene-less and overlapping-app pilot
runs were discarded. The earlier fixed-time event pilot cannot establish
settled release values and is not part of the retained final touch reference.

The exact private flex solver, force coefficients, device input latency,
arbitrary transformed/vertical tracking, HDR composition, accessibility haptics,
and genuine OS cancellation are not established by these measurements.

## Portable implementation verification

The completed control event/state implementation and UIKit slider use these
observations without linking Apple private APIs. See [semantics and C++
differences](../controls/README.md) and [build/test results](verification/README.md).
The modern track configuration surface remains partially calibrated; exact
private animation physics is not claimed.

[Resting comparisons](comparison/resting.png) put native output above NXKit for
each theme/style. [The filled-track close-up](comparison/filled-cap.png) verifies
the capsule end. Native P3 images are converted to sRGB before comparison.

Track endpoint follow-up: [native comparisons, continuity regressions and Metal verification](endpoints/README.md).
