# Selected-source rendering

`LensView` keeps its optical background and its selected-content input separate.
The background still samples the scene already rendered below the lens, including
regular glass frost. The selected source is recorded in full before the final
capsule clip. This lets ink outside the silhouette enter the refracted perimeter
without introducing ordinary-row ink from the punched-out background.

## Ownership and visibility

`contentView()` is the default source. `setLiftedContentView()` supplies a weakly
held view, usually a stationary full row underneath the moving lens. The source
must share a view ancestor with the lens. It cannot contain the lens. A detached,
expired, or subsequently reparented ancestor source is skipped safely.

A private weak-owner list suppresses only the source layer's ordinary draw while
it belongs to a connected lens. Model and presentation copies share that list;
clearing or destroying the owner releases suppression. The caller does not need
an alpha-zero host or a second clipped copy. Real source and ancestor visibility
remain effective for optical input.

The source is prepared directly before recording, so lazy text drawing and live
source invalidation continue to work even though ordinary source rendering is
suppressed. Presentation-layer transforms, bounds origins, source alpha, and
ancestor alpha below the common ancestor are included. The common ancestor's
alpha is applied by the ordinary lens traversal rather than multiplied twice.
A per-draw recursion guard stops cross-linked lens sources.

Only a detached copy of the source root loses its output mask, bounds clipping,
and source-ownership suppression. Descendant masks remain effective. The actual
source mask and its parent link are untouched. The recording includes 64pt of
source overscan, and its zero-based picture tile preserves negative source
coordinates on the raster backend. Source capture uses a retained picture; it
does not read GPU pixels back to the CPU.

## Measured perimeter transfer

The native one-point line and isolated-marker sweeps are documented in
[the runtime evidence](edge-refraction.md). They establish a straight center and
an inward, nonmonotonic perimeter fold. The selected source is actually remapped
near the edge; drawing the original selected row over a weak edge copy would
incorrectly preserve source bands that disappear in UIKit.

The effective radial fit uses local inward depth `d` in points:

```
projected = sqrt(7.94^2 + (d - 3.42)^2)
target = mix(projected, d, smoothstep(4.34, 13.70, d))
```

Regular style adds an outer fold of `5.3 * exp(-d / 1.5)`, fading to zero between
3pt and 5pt. Target depth is bounded by half the shorter lens dimension. A small
normal sampling footprint, `0.6 * exp(-d / 1.2)`, represents the broader observed
outermost pixel band. These are fitted, portable reconstruction parameters, not
Apple's private shader equation. The four-parameter selection fit has an
approximately 0.023pt RMSE against the effective green-channel dense-line map.

The final implementation and comparison artifacts distinguish radial geometry,
channel transport, and source sampling phase. In particular, a secondary Picture
image-filter input introduced a half-device-pixel source phase error for a
fractionally positioned lens. Direct picture-shader sampling preserves the
source grid and the sharp identity center.

The matrix covers static native geometry and source transfer. It does not
establish parity for private UIKit switches, velocity-dependent interaction,
other OS releases, or every GPU backend.

## Channel transmission and the source grid

The measured selection variant separates channels around isolated two-dimensional
markers. Each channel therefore needs its own coverage as well as its own color.
Using one maximum alpha for ordinary source-over would wrongly block background
channels that remain uncovered, particularly around black glyphs.

The direct source-shader path implements
`inkRGB + destinationRGB * (1 - coverageRGB)` in two destination-aware draws:
first attenuate the destination by channel coverage while keeping its alpha;
then add the premultiplied sampled ink and composite the maximum coverage into
result alpha. The capsule's antialiased coverage applies once to each linear
term. No separately rasterized source image-filter input or GPU-to-CPU readback is required.

This preserves the original picture's sampling phase. In the fractional-x pilot,
all pixels deeper than 18pt are byte-identical to the achromatic direct-source
candidate. The rejected secondary-Picture image-filter candidate differed by up
to 125/255 there, despite using the same identity center mapping. This is a
source-grid regression check, independent of the optical fit.

The shader performs two lens-area draws. Including the original identity tap,
its outer selection footprint can use up to 16 picture samples per draw;
regular style uses up to six. Backend frame
time, particularly on Switch, remains to be profiled. Compilation and raster
pixel comparisons do not establish GPU performance.

For selection style, the selected-ink follow-up strengthens the tangential
field in the measured axial sectors. With `side = abs(normal.x)`, its gains are
`1.45 + .27 * (1 - smoothstep(.05, .25, side))` for `normal.y`, and
`.80 + .36 * smoothstep(.65, .95, side)` for `normal.x`. This raises the top
gain to 1.72 and the side gain to 1.16 while retaining the earlier field in
the intervening diagonal sector. A uniform increase incorrectly moved the
corner fold branches. The small normal correction and fade between 6pt and
12pt depth remain unchanged. Red and blue perturb in opposite directions; green
uses the radial fit directly. Crucially, the perturbation is applied to the
output coordinate **before** computing each channel's radial fold. The fold's
local derivative can reverse radial channel ordering at corners. Adding the
same perturbation after the fold matched straight-edge directions but produced
a strong incorrect blue corner branch.

The preserved C4 report, before this gain adjustment, measured the held-out
cyan corner's outer-branch normalized RGB energies as
21.2/15.2/7.9 in UIKit and 23.4/13.3/8.4 in the retained before-fold model; the
rejected after-fold model produced approximately 0/8.4/52.4. Inner-branch
energies are 75/75/86 native and 70/77/92 retained. All four axial channel
directions agree with the native markers. These held-out measurements support
the generic coordinate placement rather than a per-corner color correction.

In that earlier report, residual outer corner centroid errors reach approximately 1pt;
left-side R/B separation is 1.45pt versus 2.03pt native, and the top separation is
2.68pt versus 2.98pt native. The top and bottom spectral bands remain too crisp
and discrete compared with the broader, softer native bands.
The [selected-ink follow-up](selected-chromatic/README.md) isolates colored
markers from ordinary backdrop ink in both themes. Mean axial G/B gap error
improves from .272 to .017pt. Six-marker centroid error improves only slightly,
from .281 to .275pt: corner ROI centroids regress, and a dense combined-source
gray scene has a small pixel error increase. The report retains those residuals
alongside the improvements and colored-background validation. The radial map,
sampling footprint and independent per-channel coverage are unchanged. Regular
style, no-source controls, ordinary-underlay-only scenes and the deep center
remain byte-identical. This does not establish a pixel-perfect match.


## Continuous-corner center correction

The [center-identity follow-up](center-identity/README.md) fixes a regression introduced when sampling adopted the continuous-corner distance field. Clamping the final mixed source depth to half-height created center kinks during movement. The perimeter fold is now bounded before it blends back to identity. Forty-eight retained texture cases isolate this failure across themes, styles, aspect ratios and fractional translations.


## Side-normal dispersion of selected ink

The [selected radial follow-up](selected-radial/README.md) adds the side-normal color separation missing from long blue and white glyph strokes. Native markers constrain red outward and blue inward from green. The offsets are applied to source coordinates before the radial fold, with independent RGB coverage; existing tangential transport and sharp-center mapping remain active. Its report retains all 18 native comparisons and exposes small rich-grid pixel-error regressions rather than treating the marker result as complete spectral parity.
