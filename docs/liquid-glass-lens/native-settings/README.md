# Native liquid-lens settings

Investigation: 2026-11-06, iOS/iPadOS 27.0 Simulator, build 24A434.
Production rendering is independently implemented C++/SkSL. Private UIKit
selectors and filter inspection remain confined to the research executable.
No Apple executable or shader library is linked or redistributed.

## Observed configuration

The [144 fresh captures](captures/index.json) cover two explicit styles,
light/dark, resting/lifted, six sizes (37×24 through160×120), and effective
glass preferences0/.5/1. Every response acknowledges its request UUID and
records fixture/executable hashes. Each contains the manual lens and six
actual/control lenses. Lifted filter settings are identical at the three
measured preferences: lens optics must not fade with the glass tint slider.

| Input | Observed value/rule |
| --- | --- |
| Style0 / style1 | SmallVariantSpec / LargeVariantSpec, independent of tested size |
| Backdrop displacement | +9; SDF height36, curvature1, ovalization.5 |
| Selected-content wrapper | −17.5; SDF height11.2, curvature1, ovalization.5 |
| Selected-source displacement | Small0; large−.2×short side, height.16×short side |
| Foreground aberration | Small0; large short side/19, angle−15° |
| Foreground shoulder | Height0, offset short side/1.8 |
| Foreground edge fade | Linear from distance−.2×short side to0, opacities1→0 |
| Foreground capture margin | 100, independent of measured style/theme/tint/size |
| Inner refraction | Small−.3×short side, height min(20,.18×short side); large−.15, min(20,.1×short side) |
| Ring shadow | Blur3, width4, offset8; opacity.15/.4 small light/dark, .1 large |

The native glass-background archive contains **69 explicit inputs**. Absent
clamp keys remain distinguishable from captured values. Full color transfers,
ring/shadow, filter highlight, external key/fill highlight and vibrant matrix
are retained in [normalized fixtures](fixtures/native-profile-fixtures.json)
and `UIKit/lib/NativeLensProfileData.hpp`. The generator checks all144 inputs
and the C++ test compares72 independent native expected samples. It does not
derive expected values from the resolver under test.

[Live portal identities](portal-topology.md) establish the pass order:
backdrop warp → glass background → separately warped selected source →
foreground spectral filtering → external highlight. The source displacement
samples the lifted-content branch, not the finished glass background.
Source/foreground/highlight share a geometry element with ovalization0;
backdrop/wrapper elements use.5. A configured inner-shadow layer has no
drawable silhouette in these controls; no extra opaque border is invented.

## Shader translation and implementation

[Static AIR findings](air-findings.md) and [artifact hashes](air-manifest.json)
establish the curved shoulder `1−sqrt(t×(2−t))`, signed displacement-map RG,
blue-channel mask coverage, aspect-adjusted normal ovalization, seven spectral
samples and linear edge fade. Ovalization changes direction, not amplitude.
Zero refraction opacity does not disable the independent inner refraction.
Static analysis is not evidence that a particular GPU shader executed.

[Foreground matrix analysis](foreground-aspect.md) establishes that the native
shader swaps axes **after** normalizing by source-texture allocation width and
height. This does not commute with a point-space axis swap. Six independently
varied [capture-margin controls](margin-reference/metrics.json) demonstrate the
visual consequence while keeping the filter amounts unchanged: top G/B gap is
2.441,1.737,1.549,1.395pt at margins0,50,100,200. The portable context uses a
virtual source extent `size + sqrt(2)×margin` on each axis,
then the literal native matrix ordering. This is an **inferred capture-context
policy**, checked against the margin controls, not a recovered allocator
implementation. Actual native texture allocation/tile dimensions remain
unmeasured. Independent2×/3× controls retain approximately the same dispersion;
screen backing scale must not change this capture aspect. Virtual padding
requires no additional transparent texture.

Independent held-out [200×56-point, 2× geometry](wide-reference/README.md)
and [116×56-point, 3× iPhone controls](phone-reference/README.md) validate this
policy without fitting gains or registering pixels. Their largest axial G/B
gap errors are .051/.047 pt, and largest side-normal gap errors .044/.025 pt.
The phone stage is clipped outside the viewport, but all measured ROIs are
fully visible. These checks support the inferred context policy; they do not
identify native allocation dimensions or establish complete optical parity.

`NativeLensProfile` resolves captured settings by explicit style, appearance
and current geometry. `LensMaterial` composes these passes with the shared
native-glass translation. Selected RGBA artwork retains independent channel
coverage; transparent black is not substituted for transmitted backdrop.
There is no additional five-tap empirical source blur. Zero-radius lenses
sample the warped sharp source directly and skip a redundant LOD0 atlas.
Bounds changes refresh material parameters; size-dependent shader distances
also follow live presentation resolution during bounds animation.

Resting fill/ownership and control-driven size/magnification remain caller
policy in the portable `LensView` API. The recovered spring specs and hang
time are retained as evidence, but the existing portable interaction model
is not represented as an exact port of Apple's private integrator.

## Comparison results and validation

The [16-scene comparison](comparison/comparison.png) covers both styles and
appearances over uniform gray, selected artwork, a probe grid and stripes.
[Measurements](comparison/metrics.json) use fixed circular-capsule regions,
with no fitted alignment, geometry or color correction. Mean absolute RGB
error is in 8-bit channel values:

| Region | Before | After |
| --- | ---: | ---: |
| Body | 6.690 | 2.757 |
| Edge | 10.150 | 3.558 |
| Center | 2.753 | 1.846 |
| Exterior | 6.976 | 5.837 |

Mean body error falls 59%; every scene improves in body, edge and center.
Six uniform-gray exterior controls regress, despite the overall exterior
improvement. Complete rim/shadow parity remains unfinished. Selected ink is
retained separately for ownership and vibrancy; the destination-dependent
final highlight where that ink intersects the rim is still approximate.

Verification on the final source revision:

- Native profile, lens view, full retained refraction, motion and tab-controller
  tests pass; existing optical tolerances were not relaxed.
- Metal/raster checks pass for 480 resting cases on each backend and 48 lifted
  cases spanning 1×/2×/3×, both styles/appearances, vibrancy and partial/full lift.
  Lifted mean channel error is .0487, with maximum difference 24.
- Simulator Demo, macOS UIKit and physical iOS SDK UIKit builds succeed.
- The offline Metal catalog is rebuilt for shader ABI
  `5f51a676286acb9a02622e5cf2f3e61eff601e336081c67a1f2944794ffa3d03`.
  Bundled lens and navigation replays compile no additional libraries or
  pipelines during rendering. Simulator has no baked native GPU binary
  archive here; pipeline preparation happens before the render loop.

These checks establish neither sustained device frame rate nor physical
device/HDR parity. This investigation has not installed a build on the iPhone.

## Evidence ledger and limits

- Native parameter capture: REA
  `ev_00804c3e2e408ea042163c6889f41fa905db7c931b6708550354b3b1cb0865b7`,
  [record](rea-matrix-evidence.json). This observes the Python/simctl driver;
  UIKit is launched separately by CoreSimulator. Exit0 and complete before/
  after filesystem checkpoints retain all145 files. Overall capture reports
  `truncated:true`; complete terminal/event coverage is not claimed.
- Static extraction hash verification: REA
  `ev_9d1ecd0b72b35534e6c664095795f5f4d1a8c297fc083b826fea8cd81e753133`,
  [record](rea-air-verification.json), exit0. Saved AIR hashes match the
  identified shipped QuartzCore library. Overall capture also reports
  truncation. CPU/AIR conclusions are static analysis, not runtime uniforms.
- Four independent fresh lens recreations resolve actual portal targets and
  ordinary shadow/path state; [raw captures](portal-captures/index.json).
- Repeated resizing in the144-case matrix exposes drift in the native manual
  fixture's BackdropView frame. That matrix supports applied parameter rules,
  not pixel geometry. Visual comparisons recreate the lens for every case.
- Portable extension policy: shortest-side equations extend measured sizes;
  highlight spread interpolates80→82.5° between short sides80→120 and clamps
  outside. The intervening law and automatic variant selection are unmeasured.
- Unknowns include physical-device/HDR packing, exact native offscreen
  resampling/phase, complete rim and optical pixel parity, and exact motion
  timing. Profile equality is not a claim of complete visual compatibility.
- An early diagnostic invoked arbitrary zero-argument spec methods and could
  mutate defaults. Its pilot capture was discarded. Retained captures use
  only explicitly named getters, including `archiveDictionary`.

## Reproduce

```sh
/usr/bin/python3 Demo/GlassComparison/build-lens.py /private/tmp/lens-settings
/usr/bin/python3 Demo/GlassComparison/LensSettingsProbe/capture.py \
  --device SIMULATOR_UDID --fixture /private/tmp/lens-settings \
  --output /private/tmp/lens-settings/captures
/usr/bin/python3 Demo/GlassComparison/LensSettingsProbe/generate.py \
  docs/liquid-glass-lens/native-settings/captures --check
```

Build `NXKitNativeLensProfileTests`, `NXKitLensViewTests`,
`NXKitLensRefractionTests`, `NXKitLensMotionTests`, `UIKitTabBarControllerTests`
and `SimulatorGlassBlurTests`. `LensSettingsRenderer` renders the exact native
source images and exported geometry. `LensSettingsProbe/compare.py` measures
fixed image coordinates without fitting translation, geometry or colors.
