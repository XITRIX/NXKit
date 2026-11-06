# Selected glyphs: side-normal chromatic transport

[Open the UIKit overlays](comparison.html). Also compare the actual Demo tab bar at an identical held pose in [dark](tab-dark-comparison.png) and [light](tab-light-comparison.png) themes. The two Demo images have exactly matching recorded model geometry, platter feedback, font frames and source placement; they are not visually registered after rendering.

The selected-source shader already separated channels tangentially, but its side-normal component was almost absent. A round marker can show that sideways separation while a long vertical glyph stroke remains one blue shape. White ordinary content also passes through a different backdrop optical path, so its obvious fringes did not establish that selected blue glyphs were correct.

The native side markers put blue approximately .30–.32pt inward from green, and red outward. The correction adds independently calibrated red/blue source-ray offsets near the side normals. They enter before the existing radial fold; the field smoothly tapers with normal direction and depth. RGB coverage remains independent, including for zero-valued source color channels. It changes coordinate transport rather than applying a colored highlight over the glyph. Deep-center pixels retain identity mapping from the previous fix.

## Native evidence and results

These are 18 retained UIKit 27.0 (24A434), 2× captures from the independent private-lens fixture, plus freshly rendered before/after NXKit retained layers. Both themes include separate shader-only controls and six colored markers. Source-only probes remove ordinary white content from behind the lens, allowing selected-source transport to be scored independently. Rich selected grids and combined ordinary/selected content over gray, blue, red and green validate the change. No new native capture was taken for this follow-up.

Geometry, RGBA source assets, color handling, fixed ROIs and native capture provenance follow the [previous selected-ink comparison](../selected-chromatic/README.md). The new [metrics](metrics.json) record all native/input/source/renderer hashes. Native coordinates are used directly; no translation, exposure, color or image fitting is applied.

| Measurement | Before | After |
| --- | ---: | ---: |
| Side-normal G/B separation error | .27484pt | .10084pt |
| All six marker RGB centroid error | .37703pt | .36047pt |
| Tangential G/B separation error | .01670pt | .06065pt |
| Mean normalized marker coverage error | .036028 | .036074 |

The targeted side-normal error decreases about 63%; the existing tangential separation becomes slightly less accurate. Marker coverage and several corner residuals remain approximate.

The dense validation scenes also expose a tradeoff. Lens-band RGB MAE (0–255 channel units) changes 12.148→12.408 for the gray selected-only grid, 15.344→15.576 for combined gray content, 13.502→13.807 on blue, 14.156→14.312 on red and 9.544→9.655 on green. These small regressions are retained in the overlays; the adjustment improves the missing channel placement, not full-image RGB parity. Underlay-only, regular-style, no-source and deep-center outputs remain byte-identical.

## Regression and demo checks

The long-stroke regression isolates side-normal transport without relying on round-dot tangential fringes. Blue and white strokes are checked on both sides in both themes; their normalized G/B positions must agree within .03pt. The existing blue/cyan marker checks now also constrain radial separation. Together the eight stroke and eight tinted-marker assertions fail with the frozen previous core and pass after correction.

The full lens-refraction suite also passes its 48 sharp-center translation/size checks, channel-order/near-edge tests and source lifecycle coverage. Lens, tab-composition and full-screen suites pass. The iOS Demo builds and links against the current SDK and Graphite archives; isolated temporary build outputs avoided a shared Xcode database lock. The main API remains Partial: these are production raster checks, not a Metal/Dawn or physical-device parity assertion.

`TabPressRenderer --fixed-optics` is a fixture-only option after its scale and selection-position arguments. It fixes lens model placement, clears presentation/motion phase after the hold, and leaves the settled base-platter feedback active. This isolates glyph optics from wall-clock animation timing. Normal Demo interaction is unchanged. [Metadata](tab-geometry.json) verifies exact before/after geometry in both themes; this fixed-pose comparison does not establish touch-physics parity.

## Reproduce

Build the current standalone raster `LensRefractionTests` or repository `NXKitLensRefractionTests`; use `--ink-only` for the focused blue/white stroke and tinted-marker checks. Save the old core archive first, then build `LensRefractionRenderer` twice with `build-lens-refraction.py RASTER_BUILD OUTPUT CORE_ARCHIVE`.

Run `python Demo/GlassComparison/compare-selected-lens-radial.py BEFORE_RENDERER AFTER_RENDERER OUTPUT` with Pillow and NumPy. Its native fixtures are already retained in the repository. The exporter requires side-normal G/B error to improve in both native themes and asserts exact regular/control/center pixels.
