# Continuous-corner correction

[Open the pixel overlays](comparison.html). The first five scenes isolate continuous-corner silhouettes; the remaining eight show actual retained regular glass over gray, white, blue and colored stripes in both themes. Opacity, raw before/after, amplified absolute differences, exact registration and unmatched-pixel views remain available. The unmatched view removes pixels within 2/255 per channel; measurements always use raw pixels.

Continuous corners were already the NXKit default. The previous corner-local superellipse shortened its easing as the radius approached half the height, making capsules nearly circular. This caused an abrupt-looking transition into the flat edge. The replacement combines a longer superellipse shoulder with a circular arc, using separate top/right octants for rectangular bounds.

The parameter reconstruction is adapted from [Flutter/Impeller's rounded-superellipse implementation](https://github.com/flutter/flutter/blob/5d93b2e71b3380bd0d2e4f0daf14379fcbd64a4d/engine/src/flutter/impeller/geometry/round_superellipse_param.cc), under [BSD-3-Clause](../../../licenses/Flutter-BSD-3-Clause.txt). This reuses its fitted exponent/junction model. NXKit builds adaptive Hermite cubics and analytic normals rather than porting Flutter's tessellation. The path and approximate signed distance share CPU-computed octants; glass refraction, container joins, highlight/shadow contours and lifted-lens sampling consume the same geometry. Explicit circular corners retain their previous shape.

## Native evidence and alignment

The independent public-UIKit `CornerReference.m` app was freshly built and run on an iPad Pro 11-inch simulator with iPadOS 27.0. The original [framebuffer](native-framebuffer.png), [metadata](native-metadata.json) and [cropped stage](native-scene.png) are retained. Its masks are byte-identical to the older 26.4.1 reference. Cropping uses the recorded stage origin (57, 225 points) and scale 2 only, with no fitted translation, resampling or color correction.

The solid baseline is the retained original production CALayer mask in the parent report. The corrected mask is freshly rendered through production CALayer. Glass before/after adapters use the same current `OpticalRenderer.cpp` and identical manifests, linked separately to frozen previous and corrected production core archives. Native glass images are retained iPadOS 27.0 sRGB framebuffer crops from the [optical matrix](../../liquid-glass-optics/README.md); all five public geometries and their coordinates are unchanged. [Provenance](provenance.json) records archive, executable and source hashes.

## Measurements

Unamplified mean absolute channel error on a 0–255 scale, over each fixed solid frame:

| Continuous shape | Previous | Corrected | Reduction |
| --- | ---: | ---: | ---: |
| 300 × 100, r12 | 0.0557 | 0.0464 | 16.6% |
| 300 × 100, r24 | 0.1826 | 0.0917 | 49.8% |
| 300 × 160, r52 | 0.4712 | 0.2269 | 51.8% |
| 300 × 100, r50 | 0.7418 | 0.2103 | 71.6% |
| 300 × 160, r80 | 1.1503 | 0.2223 | 80.7% |

All five explicit circular controls remain byte-identical to their previous rendering. Thresholded corrected continuous silhouette overlap is 99.91–99.99%; this does not imply exact edge coverage. The full [metrics](metrics.json) retain every shape and glass region.

All **40 regular-glass conditions improve**: five sizes across four backgrounds and both themes. Equal-condition mean ROI error decreases **2.1141 → 1.9314/255**. Each glass ROI includes its fixed frame and an 8-point exterior margin, so this score includes contour, material and nearby shadow. It is distinct from earlier reports' body/rim bands and is not comparable directly to their means. Colored stripes still show visible existing refraction and tint residuals; the 240 × 160 dark striped card remains at 6.269/255. No optical coefficients were fitted as part of this geometry correction.

## Verification and limits

Nine focused raster executables pass: Corner, Material, Rim, Container, Shadow, Lens, LensRefraction, LensTab and GlassScreen. New corner checks use independent native contour coordinates, verify path/distance classification under transposition and tiny radii, and retain fractional-position, transformed, clipped/unclipped and 1×/2×/3× edge-coverage checks. The isolated new transition regression fails five assertions when linked against the frozen previous core, and passes with the corrected core. The iOS Demo builds and links against the current Apple SDK and Graphite archives.

These are native framebuffer comparisons against production headless Skia raster rendering, using the matching raster archive headers. They do not establish Metal/Dawn runtime parity, performance, physical-device equivalence, every aspect ratio, or an exact private UIKit curve. The implicit field is a first-order distance approximation away from its contour. Lifted-lens regression checks pass, but a new complete private-lens optical matrix was not captured for this corner change.

## Reproduction

Use the parent report's `build-corners.sh`/simulator instructions to save `native27/ipad-framebuffer.png` and its original `native27/ipad-corners.json` in a run directory. Keep the signed fixture executable in `native27/NativeCornerReference.app`. Save the previous core as `before-core.a` before rebuilding.

Run `CornerTests RUN/after.ppm` (or `NXKitCornerRenderingTests` with the repository test target). Generate shared backgrounds using `prepare-optical.py RUN`. Build adapters with `build-optical.py RASTER_BUILD RUN/before --renderer-only --core-archive=RUN/before-core.a` and `build-optical.py RASTER_BUILD RUN/after --renderer-only`.

Each adapter manifest has one line per background/theme: quoted output path, quoted shared background path, theme 0/1, `neutral`, then the five `x y w h r` tuples:

```
40 64 52 52 26   150 50 80 80 40   280 30 120 120 60
480 62 132 56 28   240 230 240 160 36
```

Name outputs `before/gray-dark0.ppm`, etc., for gray/white/blue/stripes and themes 0/1. Run each adapter with its manifest. Finally run `python Demo/GlassComparison/compare-continuous.py RUN docs/liquid-glass-corners/continuous` in a Python environment with Pillow and NumPy.
