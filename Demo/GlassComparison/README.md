## Text rendering corrections

The [native UILabel comparison](../../docs/text-rendering/README.md) isolates font
weight, baseline placement, system-font resolution and tracking with identical
font-file controls at 1×/2×/3× in both themes. The full-screen gallery report is
refreshed with the corrected renderer.

## Full-screen iOS layout

**Glass → Full-screen iOS layout** opens a four-tab interactive glass screen with
custom navigation, actual Back, More/+, and working inset-grouped controls.
[Six native SwiftUI List comparisons and reproduction](../../docs/liquid-glass-lens/screen/README.md)
cover both themes, a colored background and scrolled detail rows. Typography uses
Apple HIG default iOS styles; see the report for remaining differences.

## Current refraction, shadows and lenses — 28 October 2026

The [review index](../../docs/liquid-glass-review/comparison.html) links the current
[regular-glass overlays](../../docs/liquid-glass-refraction/comparison.html),
[independent wave controls](../../docs/liquid-glass-refraction/holdouts/comparison.html),
[shadow measurements](../../docs/liquid-glass-shadows/README.md),
[shared material sizing](../../docs/liquid-glass-shadows/shared-material/README.md),
[96 retained joins](../../docs/liquid-glass-interaction/retained-joins/current-combined/README.md)
[lens content refraction](../../docs/liquid-glass-lens/refraction/comparison.html),
[background chromatic refraction](../../docs/liquid-glass-lens/chromatic/comparison.html)
and [recorded lens motion](../../docs/liquid-glass-lens/motion/comparison.html).
The dedicated interactive tab bar and separate lens study join the existing 17 shared gallery cases. Their public-glass
base and separate selection lens are a controlled tab-bar composition; it is not
a replacement for a complete UIKit tab controller. The real native UITabBar,
UISwitch and UISlider remain in the independent reference app for hierarchy and
geometry checks. [Research and private-API/license boundaries](../../docs/liquid-glass-lens/research.md)
distinguish observations from the portable approximations.

## Aligned iPadOS 27 optics and retained joins

The [optical matrix](../../docs/liquid-glass-optics/README.md) compares both themes,
five sizes, three tint choices and ten backgrounds. Its offline viewer overlays
UIKit and NXKit at measured coordinates, removes matching pixels and shows
unamplified residuals. The [retained-join matrix](../../docs/liquid-glass-interaction/retained-joins/README.md)
adds both tint orders and joins that remain connected after touch release.
These comparisons use color-managed sRGB; earlier raw-channel Display P3/sRGB
color-error figures are historical, superseded evidence.

## Shared Glass demo gallery

The [current interaction and mixed-material join comparison](../../docs/liquid-glass-interaction/README.md)
adds real UIKit touch traces, the measured 119/120-point size-mode boundary,
full press/pull/release replay tests, and separate neutral/tinted lighting
calibration in both appearances. It supersedes the old single-response motion.

The original shared Demo Glass gallery has 17 matched cases, plus the interactive tab bar and lens study. Its production
`GlassDemoScene` and independent `GalleryReference.m` UIKit adapter share
`GlassDemoSpec.h` geometry, palette and artwork. Retained production-layer
renders replace the old standalone-material-only comparisons for this gallery.
See [current results and reproduction](../../docs/liquid-glass-gallery/README.md),
including the two-view tint/order test on iPadOS 26.4.1 and 27.0 and the [fixed interactive buttons](../../docs/liquid-glass-gallery/fixed-buttons/README.md).

The older reports below preserve previous calibration evidence; their “current”
columns describe the material at capture time, not the new gallery calibration.

# Native UIKit material comparison

This fixture recreates the three-shape scene used to review NXKit's glass.
`main.m` links **Apple's UIKit** and uses real `UIVisualEffectView` instances
with `UIGlassEffect`, `interactive = YES`, and fixed `UICornerConfiguration`
radii. It does not link NXKit or emulate Apple's material. Both renderers use
`Scene.h`: a 720 × 440 point artwork, a clear 120 × 60 capsule, a regular
300 × 160 card (radius 52), and a regular 520 × 56 bar.

The native image must come from the onscreen Simulator framebuffer using
`simctl io screenshot`. `drawHierarchy`, layer snapshots, and image renderers
do not capture the live native glass compositor reliably.

`GlassComparisonRenderer.cpp` uses NXKit's production effect resolution, SkSL,
and the same blur/runtime-image-filter graph at 2×, on Skia's raster backend.
It isolates the material; it does not execute NXKit's layer shadows, retained
tree, Metal/Dawn backend, or input loop. The native capture includes UIKit's
default shadows and its own private continuous-corner spline. NXKit now uses
continuous corners and one antialiased clip by default; its portable spline
approximation and the missing full layer tree remain comparison limits.

## Reproduce on an Apple Silicon Mac

Requires Xcode with an iOS 26+ Simulator runtime, a compatible raster Skia
archive, and Python with Pillow. Run from the repository root; replace the
device ID with an available **iPad** Simulator. Do not scale the native scene
to fit a smaller iPhone screen.

```sh
python3 Demo/GlassComparison/prepare.py /tmp/nxkit-glass-comparison
sh Demo/GlassComparison/build-native.sh /tmp/nxkit-glass-comparison
sh Demo/GlassComparison/build-raster.sh /tmp/nxkit-glass-comparison
xcrun simctl boot DEVICE_ID
xcrun simctl bootstatus DEVICE_ID -b
xcrun simctl install DEVICE_ID /tmp/nxkit-glass-comparison/NativeGlassReference.app
xcrun simctl launch --terminate-running-process DEVICE_ID org.nxkit.NativeGlassReference
```

After launch and material settling, capture the screen and find the metadata:

```sh
xcrun simctl io DEVICE_ID screenshot /tmp/nxkit-glass-comparison/native-screen.png
xcrun simctl get_app_container DEVICE_ID org.nxkit.NativeGlassReference data
```

Copy `Documents/capture.json` from that returned container into the comparison
directory. It records the scene origin, scale, OS version and accessibility
settings. Then render and compare:

```sh
/tmp/nxkit-glass-comparison/GlassComparisonRenderer /tmp/nxkit-glass-comparison/scene.rgb /tmp/nxkit-glass-comparison/nxkit.ppm
python3 Demo/GlassComparison/compare.py /tmp/nxkit-glass-comparison/native-screen.png /tmp/nxkit-glass-comparison/capture.json /tmp/nxkit-glass-comparison/nxkit.ppm /tmp/nxkit-glass-comparison/results
```

The scripts crop by recorded geometry without shifting, retouching, exposure
correction, or per-shape color adjustments. The native app also accepts
`--black`, `--white`, `--gray`, `--dark`, and `--no-glass` for manual material
and accessibility checks. Both renderers accept `--dark`, `--size=0..1`,
`--tint=blue`, and `--tint=coral`. The raster harness supplies real copied
trait collections to production effect resolution; it does not simulate dark
appearance by changing shader uniforms after resolution.
`SKIA_RASTER_ARCHIVE` can override the default `out/mac-arm64-angle/libskia.a`.
`SKIA_RASTER_HEADERS` can select the header tree matching that archive. Mixing
the old raster archive with newer Skia headers is not a valid runtime test.
For the retained debug raster snapshot, pass `SKIA_RASTER_DEBUG=ON` to
`build-raster.sh` or `NXKIT_SKIA_DEBUG=ON` to the CMake raster harness.
The adapter now compiles neutral descriptors from UIKitCore and the Apple
material recipe from UIKit; see [module boundaries](../../docs/module-boundaries.md).

The production `UIGlassEffect::setAdaptsAppearanceToBackdrop(true)` flag enables
the optional iOS 26 contrast policy on a retained `BackdropEffectView`. It is
off by default. The standalone material renderer does not run a view hierarchy
or the asynchronous sampler, so it cannot validate adaptive content traits;
the layer-rendering regression target exercises that path on a raster surface.
See [`docs/LIQUID_GLASS.md`](../../docs/LIQUID_GLASS.md) for usage and limits.

## Captured evidence, 2026-10-27

Native: iPad Pro 11-inch (M5), **iPadOS 26.4.1**, scale 2, light appearance,
Reduce Transparency and Increase Contrast disabled. Compiled against the
iPhoneSimulator 27.1 SDK, deployment target 26.0. Captures and enlarged crops
are in [`docs/liquid-glass-comparison`](../../docs/liquid-glass-comparison).

| Mean absolute RGB channel error (0–255) | Before native calibration | Current |
|---|---:|---:|
| Clear capsule | 19.506 | 2.020 |
| Regular card | 63.633 | 3.901 |
| Regular bar | 65.436 | 3.230 |

These measurements exclude the outer four points of each shape to assess
material independently of corner contour and shadow differences. They are
evidence for **this scene**, not a claim of pixel-identical UIKit compatibility.
The neutral calibration samples also distinguished body tone from blur:
over black, native clear/small regular centers were 19/255 and the large
regular center was 179/255; over mid-gray the two regular centers were
205/255 and 219/255. Automated material regressions retain those samples.

Regular frost now continues through the bevel. A broader inward lens and
gentler blur preserve the native refracted shapes without a transparent band.
The body uses a size-dependent baseline and a common backdrop-dependent veil,
rather than trying to create material density by increasing blur. The rim is
a sub-point white reflection, as observed in these native captures.

Remaining differences include continuous corner geometry, fine ghost detail
and edge light, shadows, and content adaptation outside the sampled scenes.
The fresh-view measurements above do not establish parity during resizing.
The expanded comparison below identifies substantial remaining dark, tint,
and retained-size-state differences. Native touch/motion matching and
Metal/Dawn rendering still need matched end-to-end captures.

## Dark appearance, tint and retained resizing

The series captures 54 native checkpoints: light/dark × untinted/blue/coral ×
nine growth/contraction steps. One regular card grows through 120 × 64,
165 × 88, 210 × 112, 255 × 136 and 300 × 160 points, then shrinks through the
same sizes. Its center and 52-point requested radius stay fixed. The other
two shapes remain controls. Blue is sRGB (0, 122, 255); coral is (255, 69, 58);
both are supplied with alpha 1 to Apple's `UIGlassEffect.tintColor` and the
corresponding NXKit API. Untinted samples leave the native property unset.

The same native views are resized in place with a 0.3-second animation. Each
capture waits for an acknowledgement after 0.8 seconds, after material settling.
The GIFs show those **settled checkpoints**, not animation frames or timing.
NXKit's raster fixture resolves each matching target geometry. It does not
exercise NXKit's presentation-layer interpolation or input/render loop.

```sh
python3 Demo/GlassComparison/capture-series.py DEVICE_ID /tmp/nxkit-glass-comparison/NativeGlassReference.app /tmp/nxkit-glass-comparison /tmp/nxkit-glass-series
python3 Demo/GlassComparison/analyze-series.py /tmp/nxkit-glass-series /tmp/nxkit-glass-series/results
```

The fixture writes actual appearance, target geometry, palette, request ID,
OS, scale, and accessibility settings alongside each framebuffer capture.
`--light-only --neutral-only` repeats the neutral light checkpoints without
discarding the other cases in an existing manifest. Each appearance also saves
a fresh `initial` state before any resize. Test requests live in the fixture's
own `Documents/request.json`; `{ "id": "probe", "size": 0, "tint": "none",
"animate": false }` also permits comparison without animation.

Observed gaps on iPadOS 26.4.1, with rendering behavior unchanged by this audit:

- NXKit's regular dark material is substantially brighter and more saturated.
  The default clear material is much closer in both appearances.
- Native full-alpha colored tints strongly color clear glass and make the
  regular material nearly solid at the sampled centers. NXKit drops explicit
  tint on clear (its default tint opacity is zero) and gives regular tint the
  wrong strength/tone, especially in dark appearance.
- A fresh untinted native regular card in light mode has the neutral light
  body used for the earlier calibration. Resizing that same view changes its
  neutral tone even while its effective traits remain light. Returning to its
  original frame retains the changed tone. The same change was observed when
  applying the sizes without animation. This is an observation for this
  Simulator/runtime, not a general rule for every UIKit release or device.
  NXKit's current size-only tone model does not reproduce it.
- Corresponding settled growth/contraction checkpoints were stable after the
  initial change (full-scene mean RGB differences below 0.001/255). That does
  not establish animation-timing or transient-material parity.

Saved montages, checkpoint GIFs, original-pixel metrics, and a fresh-versus-resized
comparison are in [`docs/liquid-glass-size-tint`](../../docs/liquid-glass-size-tint).
Shader calibration for these newly demonstrated differences remains work to do;
the audit does not mark them compatible.


## Version-specific validation: iPadOS 27

The 26.4.1 retained-tone change is not reproduced on **27.0 (24A434)**.
The same binary and card geometry were tested with direct and animated
120 × 64 → 300 × 160 round trips, with the card's own effective traits logged.
Both light and dark appearance remain stable to below 0.001/255 mean channel
difference after returning to the original size. See the
[native version comparison](../../docs/liquid-glass-ios27/README.md).

```sh
python3 Demo/GlassComparison/probe-resize.py DEVICE_ID /tmp/nxkit-glass-comparison/NativeGlassReference.app /tmp/nxkit-probe27-light
python3 Demo/GlassComparison/probe-resize.py DEVICE_ID /tmp/nxkit-glass-comparison/NativeGlassReference.app /tmp/nxkit-probe27-dark --dark
python3 Demo/GlassComparison/compare-versions.py /tmp/nxkit-probe26-light /tmp/nxkit-probe27-light /tmp/nxkit-version-comparison
```

This establishes theme stability for the tested sequence, not removal of all
backdrop/size-dependent color adjustment. The 27.1 runtime installed on the
verification host reports no supported device types and could not be run;
the 27.1 SDK used for compilation does not establish 27.1 runtime behavior.

## Container reference

`ContainerReference.m`, `build-container.sh`, `capture-container.py` and
`compare-container.py` validate grouped glass against real UIKit. The retained
NXKit renderer and regression suite are built with `RasterTests/CMakeLists.txt`.
See [container results and reproduction](../../docs/liquid-glass-container/README.md).
