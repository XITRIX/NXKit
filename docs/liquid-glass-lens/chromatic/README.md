# Lens background chromatic refraction

[Pixel comparison](comparison.html) · [UIKit / before / after](overview.png)

This comparison isolates the ordinary background behind `LensView`. No selected
content is installed. The earlier [selected-source comparison](../refraction/README.md)
measured a different optical path: its calibrated channel transport was already
present, while the background shader explicitly set its chromatic spread to zero.

The native reference is the standalone UIKit fixture on iPadOS 27.0, at 2× display
scale. Black and white dots at 3pt depth reveal the missing background fringes;
8pt dots check that the deeper body remains nearly achromatic. A monochrome grid
with colored reference lines supplies independent edge-crossing evidence.
Regular-style controls show little channel separation, so the fix applies to the
selection lens. It does not add a colored outline to regular lenses.

## Reconstruction

Native background markers at 3pt depth land approximately 6.9pt inside the rim;
a 12pt-deep background grid line lands about 14.3pt inside. The old selection
material placed the near marker about 5.9pt inside. A stronger, narrower
refraction falloff corrects this geometry without changing the selected-source
fold or regular-glass shader.

The selection background now samples separate RGB rays along the edge tangent,
with opposite ordering at opposite edges and a finite sampling footprint. The
separation fades over the first 10pt. Background rays separate after their
geometric map; the selected-source path retains its independently measured
before-fold separation. The center remains sharp. Spectral surface lighting is
unchanged; the new effect transports actual background pixels.

## Measurement

The report reuses exact native background assets and measured rectangles. There
are no fitted translations, fitted colors, or post-render image corrections.
Embedded color profiles are converted to sRGB before comparison. Full native
captures and source assets remain unchanged.

Perimeter scores use the native capsule between 0.5pt and 10pt depth, split into
four edges. Transition and center scores are separate. RGB error measures the
absolute difference per channel; chroma error removes each difference pixel's
channel mean to expose color residuals independently of neutral brightness.
Exterior shadows are visible in the viewer but excluded from these masks.

For each sparse marker, matched gray captures remove the lens's own colored
lighting. Per-channel coverage is `(image − control)/(sourceRGB − control)`;
weights below 0.1 are excluded. Centroids, mass, and two-axis spread are measured
in fixed 15pt neighborhoods partitioned by the nearest declared source marker.
These describe the observed footprints; they do not assert Apple's private
shader equation.

The viewer provides UIKit, NXKit before/after, aligned overlays, absolute RGB
error, and residuals where exact or within 2/255 matching pixels are transparent.
Its close-up uses the integer pixel rectangle enclosing the native lens plus
12pt context, identically for every image.

## Results

The report contains **29 scenes: 23 validation scenes and six matched gray
controls**. It includes both themes, both lens styles, bare/clear/regular bases,
3pt and 8pt marker depths, a black/white/red/blue grid, and red/green/blue/striped
backgrounds. The extra near-edge regular-style and clear/regular-base holdouts
are in the light theme. This is not a full factorial matrix.

Mean absolute channel error on a 0–255 scale, equally weighted by scene:

| Region | Before RGB | After RGB | Before chroma | After chroma |
| --- | ---: | ---: | ---: | ---: |
| Perimeter | 6.881 | 6.330 | 3.351 | 3.444 |
| Top | 7.288 | 7.510 | 3.455 | 3.916 |
| Bottom | 5.937 | 5.107 | 3.158 | 2.912 |
| Left | 7.285 | 6.287 | 3.538 | 3.532 |
| Right | 7.395 | 6.446 | 3.313 | 3.458 |
| Transition | 3.320 | 3.045 | 1.045 | 0.937 |
| Center | 2.402 | 2.359 | 0.801 | 0.777 |

The twelve bare near-edge marker channel-centroid errors average **1.044pt →
0.378pt**. The top R-minus-B horizontal separation becomes 1.84pt against
UIKit's 1.87pt; the bottom becomes −2.11pt against −2.26pt. Side separation is
approximately 1.02pt against UIKit's 1.37pt. Native secondary branches and
channel-dependent footprint widths remain different.

The color splitting is now visible with the correct direction, but aggregate
chroma-only error increases slightly. The clear-grid chroma error improves
about 6.08 → 4.92, while the bare-grid error increases 10.98 → 13.78 because its
spectral footprints remain misaligned. The top perimeter also retains a small
RGB regression. All five measured regular-style frames are byte-identical to
the previous material. Selected-source C4 and contact rendering code are
unchanged. Solid-color perimeter RGB changes are at most 0.044/255 in this
matrix; textured scenes improve in overall perimeter RGB error.

The delivered comparison was rerendered using the final combined production
archive `c734d3d2c3e32dc53528ce54c87d8880bd90c9a74673c9f622df567f80f84d38`.
All 29 full-resolution frames are pixel-identical to the independently tested
material build; inactive interaction contributes an identity transform. The
archive and source identities, corrected native fixture build provenance, and
pixel-identity check are recorded in [`provenance.json`](provenance.json).

## Validation and limits

`LensRefractionTests` now places ordinary monochrome stripes behind the lens,
without a selected portal. The frozen previous material fails all eight
selection-style RGB-ordering checks across four edges and both themes, plus
both near-edge placement checks (ten failures in total). The corrected material passes these checks, native near/deep refraction placement,
and the existing selected-source, ownership, opacity, text, curved-edge, and
fractional-center regressions.

This is a fix for a missing optical path, not complete visual parity. UIKit still
has broader and sometimes paired spectral lobes, channel-dependent secondary
normal branches, and a softer, more distributed rim. Restoring visible color
separation can increase chroma-only pixel error where these footprints remain
misaligned, even when the marker centroids and overall RGB error improve.
The dark narrow contour remains an existing approximation.

Evidence is limited to this measured runtime, geometry, and static raster
backend. These images do not establish Metal/Dawn parity or chromatic behavior
during real velocity-dependent deformation. The separate motion fixture covers
interaction behavior. The original C4 report remains preserved as historical
selected-source evidence.

## Reproduction

Build retained adapters with `Demo/GlassComparison/build-lens-refraction.py`,
then run `Demo/GlassComparison/compare-lens-chromatic.py` with the new native run,
frozen before renderer, final renderer, and this directory. The optional
`--additional-run` adds the preserved grid and color-background controls.
Binary, source, native metadata, and background hashes are recorded in
[`provenance.json`](provenance.json); complete per-scene and marker measurements
are in [`metrics.json`](metrics.json).
