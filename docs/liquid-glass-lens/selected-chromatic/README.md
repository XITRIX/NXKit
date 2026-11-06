# Tinted selected ink: chromatic transport

The lifted selection lens disperses its selected-color source independently
of the ordinary icons and backdrop. The previous source field had the correct
channel order but too little tangential separation. Blue/cyan glyphs therefore
looked much less dispersed than the background. The correction strengthens
the measured top/bottom and side sectors without changing the radial fold,
sampling footprint, channel-specific coverage or sharp interior.

[Open the comparison](comparison.html). Choose UIKit, NXKit before/after,
50% overlay, absolute difference, or remove pixels matching within 2/255.
The images use fixed stage coordinates with no fitted translation, color
correction or geometric warp.

## Evidence and results

These are retained native UIKit 27.0 (24A434), 2× captures, including isolated
colored markers in both themes. The native fixture independently creates the
large selection lens with the style and lifted-content mode observed in real
UITabBar. Its 116×56-point bounds are caller-supplied; this test does not measure
automatic size thresholds or control growth. No new native framebuffer capture
was obtained for this follow-up, and the user's newer screenshot is a qualitative
reference rather than a pixel-aligned test input.

Shader-only captures remove the material/rim contribution from each source
capture. Channel coverage is `(image-control)/(sourceRGB-control)`, clipped to
0…1, excluding denominators smaller than 16/255. Centroids use
`max(coverage-.1, 0)` weights. Four axial and two corner ROIs are fixed before
the comparison. Source-only markers have no ordinary white icon row: backdrop
fringes cannot satisfy these measurements.

| Selection markers, both themes | Before | After |
| --- | ---: | ---: |
| Mean axial G/B separation error | .2723 pt | .0166 pt |
| Mean six-marker RGB centroid error | .2807 pt | .2747 pt |
| Mean normalized coverage error | .03517 | .03485 |

The axial gap improves substantially; the aggregate centroid improvement is
small. **The two corner ROI centroid errors increase.** The earlier diagonal
field is retained, but these finite ROIs also collect fringes from neighboring
directions. No complete corner or spectral match is claimed.

Dense grids, colored glyphs and text provide additional validation. The table
reports mean RGB error in the lens band .5…18pt deep, in 8-bit channel units;
it includes existing material and optical differences.

| Validation scene | Before | After |
| --- | ---: | ---: |
| Gray, selected source only | 12.203 | 12.107 |
| Gray, combined ordinary/selected source | 15.313 | 15.334 |
| Blue, combined source | 13.473 | 13.507 |
| Red, combined source | 14.081 | 14.090 |
| Green, combined source | 9.741 | 9.648 |
| Ordinary underlay only | 19.508 | 19.508 |

The small blue/red/gray regressions remain visible in the overlays. Both
regular-style controls, source-free scenes, ordinary-underlay-only output and
pixels at least 18pt inside the lens are byte-identical before/after. This
change targets selected-ink dispersion; it does not resolve the remaining
material, tint, corner or spectral-footprint differences. Metal/Dawn output,
performance and physical iOS 27.2 remain unverified.

## Implementation

With `side=abs(normal.x)`, the selected-ink tangential gains are:

```text
normal.y: 1.45 + .27 * (1 - smoothstep(.05, .25, side))
normal.x:  .80 + .36 * smoothstep(.65, .95, side)
```

The field fades between 6 and 12pt depth and keeps the existing small normal
correction. Red/blue perturb in opposite directions **before** their radial
fold; green uses the unperturbed map. This is an observable approximation,
not Apple's shader. A uniform gain was rejected because it moved the curved
fold's inner/outer branches incorrectly. The existing `.6*exp(-depth/1.2)`
source footprint and two-draw independent RGB coverage composition remain
unchanged. No tint-specific shader branch or additional sample is introduced.

`LensRefractionTests` adds 16 source-only checks: two themes × blue/cyan ink ×
four edge positions. All 16 separation checks fail with the previous field and
pass with the correction. Existing black-ink, background, sharp-center,
regular-style and source-lifecycle checks pass. `LensTabTests`, `GlassScreenTests`
and `LensMotionTests` also pass; the iOS Demo builds with signing disabled.

## Reproduction and provenance

Build `LensRefractionRenderer` and `LensRefractionTests` using the raster harness
in `Demo/GlassComparison/RasterTests`, then run:

```sh
python3 Demo/GlassComparison/compare-selected-lens-chromatic.py \
  /path/to/previous/LensRefractionRenderer \
  /path/to/current/LensRefractionRenderer \
  docs/liquid-glass-lens/selected-chromatic
ctest --test-dir /path/to/raster-build --output-on-failure \
  -R '^(LensRefractionTests|LensTabTests|GlassScreenTests|LensMotionTests)$'
```

The before renderer uses `LensMaterial.cpp` from `233d6e1`, SHA-256
`6adce0d032dd0b2de8d6f06315fc93aede4d1b5ee4b930b6498fd95615eb7005`,
linked against the same current rendering adapter. The report records native
capture/input, source and before/after renderer hashes in
[metrics.json](metrics.json). Native RGBA inputs and metadata remain in
[`../native/edge-probes`](../native/edge-probes); the rich light-mode source
pattern is reused across its measured flat color backgrounds. Marker primaries
are identical across the two themes and styles.

The actual four-tab Demo can hold the lens halfway between Home and Favorites
using `TabPressRenderer OUTPUT_PREFIX DARK R G B 3 .5`; the optional position
only affects this comparison adapter. The existing
[tab-content](../tab-content/comparison.html) and
[full-screen](../screen/comparison.html) reports were refreshed with the shader.
