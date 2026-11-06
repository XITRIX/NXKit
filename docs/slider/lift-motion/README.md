# Slider lift and resting-material correction — 2026-11-06

The implementation now separates slider value retirement, knob size, resting-fill
opacity, and view-motion flex. Finger-up clears UIControl highlight and starts
lowering while projected value movement continues; terminal tracking delivery
remains deferred until value retirement ends. Stationary tracking ends immediately,
independently from its visual tail. A short tap retains the small lens's minimum
lift interval. See the [moving-release timing correction](release-timing/README.md).

## Evidence and confirmed defects

The native fixture uses actual XCTest-delivered touches and samples public model /
presentation layers. It retains actual timestamps, layer order, opacity, bounds,
transforms and animation metadata. Native recording requested 120 Hz but achieved
about 60 Hz. The [native report](README-native.md) records capture provenance and
limits; [video provenance](video/README.md) maps visible frame markers to samples.

* NXKit's generic UIView spring ignores the supplied damping and uses a fixed
  elastic timing function. The old slider therefore peaked around 65pt wide and
  undershot to about 29.5pt. Native peaks near 59.86pt and undershoots near 36pt.
* Its independent lens-progress spring made the opaque fill fade on a different
  clock from knob geometry. Native press size and white-fill opacity share a
  spring; release opacity follows a separate critical response.
* Disabling the entire visual host caused LensView's ancestor validation to
  cancel its motion response every frame. The host remains enabled; slider hit
  testing still returns the control itself and visual children are disabled.
* Native glass containers remain alpha 1 during the transition. The containing
  control drives resting-fill opacity; it does not fade the entire optical pass.
* Native slider `warpsContentBelow` is false, but its Gaussian remains active.
  A direct per-frame KVC probe established that this flag changes sustained
  displacement; it does not disable the complete backdrop pass. Cold first lift
  also has a transient displacement that fades to zero.

## Native parameters and portable translation

| Transition | Response | Damping ratio | Retarget impulse |
| --- | ---: | ---: | ---: |
| Small-lens lift bounds | .27 | .625 | .032 |
| Small-lens unlift bounds | .5 | .7 | .016 |
| Resting fill on unlift | .4 | 1 | 0 |

The recovered spring parameters are independently corroborated by the stationary
presentation curves: `omega=2*pi/response`, and retargeting adds
`(target-current)*omega*omega*impulse` to velocity. Exact damped integration is
independent of refresh rate. Bounds terminate at fitted .25/.46-second tails;
fill terminates at .33/.48. Optical inputs have their own .5/.3 completion
windows: unlike bounds, the initial warp keeps oscillating after size settles,
and on release the blur finishes before the geometry tail. The minimum lift uses the captured .22 lens-hang
setting; native short taps corroborate a minimum raised interval, but its exact
private clock origin is uncertain by a compositor frame or two.

The C++ regression retains captured values rather than regenerating them from the
implementation. After alignment for the measured commit interval, maximum
stationary light bounds residuals are .01873pt on lift and .05707pt on release;
clamped white-opacity residuals are .000814 and .002744. These numbers establish
curve agreement, **not device input-to-display latency**. Re-grab preserves current
size, opacity and spring velocity as a portable policy. Requested XCTest re-grabs
were coalesced or compressed in the recorded touch streams, so native re-grab
velocity semantics remain unverified.

## Resting blur and material routing

REA archive analysis checked 432 actual-control records across 144 captures:
[settings, exact values and Evidence IDs](../../liquid-glass-lens/native-settings/control-materials/README.md).

* The real tab selection resting surface uses Gaussian input radius 2, capture
  scale .25, then a color matrix. NXKit now uses those actual matrix coefficients
  instead of coefficients fitted from pixels. Radius-to-capture-pixel semantics
  remain an approximation of the native compositor.
* Actual slider filters establish `radius=max(0,6*(1-q))` while the optical
  spring is active. `q` initially follows the geometry spring, including its
  overshoot. The Gaussian reaches 6.21445 before settling at 6 around .30 seconds
  after release; resting-fill opacity follows its separate critical response.
* Cold first lift has displacement `50*(1-q)`, including negative overshoot;
  settled lift and all captured release frames have displacement zero. Generic
  small lenses retain captured 50/6 → 9/0 endpoints. Subsequent-contact transient
  displacement is unverified and currently uses a portable zero-origin policy.
* A real displacement filter now precedes a real Gaussian filter, followed by
  caller-owned resting fill, ClearGlass optics and highlight. The Core rendering
  extension `BackdropEffect::outputBlurRadius()` filters shader output, unlike
  input blur; it works through Core CABackdropLayer and UIKit effect hosts.
* `LensView::warpsContentBelow()` is a portable private-policy counterpart,
  defaults true, invalidates live material and supports XML. UISlider sets false;
  tab selection retains true. This does not suppress transition blur.

These slider radius/displacement relationships are directly measured by known-key
CAFilter KVC sampling, not inferred from screenshots. See the native report's
`direct-filter` evidence. Generic switch interpolation remains inferred from
captured endpoints; this work does not claim a completed UISwitch component.

## Shadow and optical retirement correction

The actual-slider material probe now retains all69 glassBackground inputs over119
presentation samples. Its layer opacity remains1, but refraction, face opacity,
ring radius/offset/opacity, highlight geometry, headroom and shadow inputs animate
back toward neutral settings. Keeping the lifted profile active explains the
returning white fill's internal dark contour. A native red/green/white ablation
reproduces that contour when the resting fill is forced visible while the glass
remains lifted; it does **not** support moving the resting fill above the optics.
See [the retained native evidence](README-native.md).

NXKit keeps the verified native layer order and animates those material inputs.
The slider passes the raw opacity-spring value, including lift overshoot, and a
reference size derived from its thumb geometry. Its ordinary resting shadow
(.14/radius5/offset0,1) now belongs to the resting-fill host, not the lens root;
the host opacity and live capsule geometry carry it through the entire lowering
transition. The white content remains clipped without clipping its exterior
shadow. No additional guessed inner-shadow silhouette was introduced.

REA also disproved the radial interpretation of CASDFOutputEffect.maximum. It
controls SDF support padding, not an alpha cutoff at that distance from the base
capsule. The lens shader no longer cuts the ring/shadow at that contour. Finite
output support follows the shifted ring/shadow kernels, allowing their natural
falloff. [Static evidence and its allocation limits](../../liquid-glass-lens/native-settings/control-materials/shadow-padding.md)
are retained; the exact native offscreen allocator is not reproduced.

Material retirement currently shares the measured white-fill critical clock.
The native inner-refraction/shadow-amount follower has a small independent initial
velocity; the focused input regression permits its measured difference. Exact
private opacityPair grouping, every intermediate material clock, and full native
pixel parity remain Partial. The earlier experimental global optical fade and
resting-fill reordering were removed.

## Verification and comparison

* 35 native geometry cases and six independent native touch replays pass.
* Native bounds/fill curve regressions, refresh-rate independence, continuous
  re-grab state, minimum-size pickup, cancellation and overlapping moving-release lowering
  checks pass on Simulator.
* LensView, LensMotion and tab controller regressions pass.
* Both themes at1×/2×/3× check the late-release surface and exact final resting
  shadow footprint. White-background samples check that the lifted shadow tail
  continues beyond the former cutoff without an abrupt alpha cliff. Independent
  native material-input checks cover retirement, neutral endpoints and overshoot.
* 72 slider Raster/Metal cases pass (mean channel error .05636/255, maximum 92).
* 870 full-motion Metal frames / 150 Raster comparisons pass (mean .03358/255,
  maximum 92), including changing post-displacement Gaussian kernels.
* Core-only rendering verifies that output blur softens a shader-generated edge;
  applying blur only to the shader input cannot produce that result.
* 48 lifted shared-lens Raster/Metal cases pass (mean .06180/255, maximum 24).
* 480 bounded-source cases pass on each renderer (Raster maximum 1, Metal 23;
  mean errors below .000002/255). These tiny averages include empty areas and
  do not establish native optical pixel parity.

[Shadow details](comparison/shadow-detail.png) retain unscaled native / before / after
3× crops for the lifted tail and light/dark retirement.

[Light motion](comparison/light.gif) and [dark motion](comparison/dark.gif) show
native / NXKit crops at matching 3× optical backing at half speed. The
previous visual baseline was accidentally rendered through a window with unset
traits, producing 1× optics enlarged into a 3× image. Those images are excluded
from quality comparisons; scale-independent geometry/opacity CSVs remain for
before/after curve metrics. The capture now asserts inherited display scale and
all lens optical backing scales on every frame, and records them in metadata. [Metrics](comparison/metrics.json)
retain before/after curve errors and timing limitations. Native video is compressed
and marker alignment has up to one compositor frame of phase uncertainty.

The [final verification record](verification/summary.json) retains test counts,
shader ABI, small logs and explicit limits. A strict cold-cache replay uses248
offline MSL libraries and206 pipeline descriptors and reports zero library or
pipeline compilations during rendering.

The final optimized iPhone build was signed and installed on iPhone Pro XITRIX.
These Simulator tests do not assert physical-device optical or frame-rate parity.

Reproduce with the existing Simulator build:

```sh
cmake --build Demo/build/ios-sim-ninja --target NXKitSliderTests NXKitLensViewTests SimulatorGlassBlurTests
xcrun simctl spawn DEVICE /absolute/path/to/NXKitSliderTests.app/NXKitSliderTests
xcrun simctl spawn DEVICE /absolute/path/to/NXKitSliderTests.app/NXKitSliderTests --motion /private/tmp/slider-after
xcrun simctl spawn DEVICE /absolute/path/to/SimulatorGlassBlurTests --slider-only
xcrun simctl spawn DEVICE /absolute/path/to/SimulatorGlassBlurTests --lens-only
/usr/bin/python3 Demo/SliderComparison/compare-motion.py /private/tmp/slider-before /private/tmp/slider-after
```
