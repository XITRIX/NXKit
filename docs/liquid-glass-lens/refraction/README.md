# Lens content and perimeter refraction

This preserved report records the selected-source calibration before the later
[background chromatic correction](../chromatic/README.md) and
[interactive motion work](../motion/README.md). Its “after” images describe that
recorded archive; provenance is retained rather than silently replaced.

This comparison isolates the selected content sampled by the lens. The previous
comparison's centered icons could look correct while the perimeter lost every
refracted branch. The new native fixture draws grid lines, text, colored crosses,
sparse markers, and a coordinate ramp across all four edges. It exports the exact
RGBA source with 64pt of overscan around the bar.

The native reference is the standalone UIKit fixture on iPadOS 27.0. It is runtime
evidence for that build, not a claim about unmeasured versions or devices. Private
selectors are confined to the research fixture; NXKit uses its own renderer.

[Open the pixel comparison](comparison.html) · [Light preview](overview.png) · [Dark preview](overview-dark.png)

## Measurement

- Geometry comes from the native hierarchy at 2× display scale. There are no
  image translations, fitted colors, or post-render warps.
- Native profiles are converted to sRGB before comparison. Original captures
  are preserved; raster output is explicitly interpreted as sRGB.
- Original ink uses the exported source alpha. Center ink is reported separately
  from the top, bottom, left, and right edge regions.
- A matched no-ink native capture removes lens shading from the copy signal.
  Copy pixels must differ by more than 12/255 from that control and fall outside
  the original source alpha dilated by 1pt. These masks come only from UIKit and
  source assets. NXKit output never determines the scored mask.
- Each edge has a fixed 18pt band partitioned by normalized position in the lens.
  Copy RGB error and ink-signal error are reported independently of center ink.
  Copy energy indicates missing or excessive ink but is not a fitting factor.
- Sparse markers provide a 2D source-to-copy centroid measurement in a fixed 24pt
  neighborhood, excluding original ink. This describes the observed branch
  displacement; it does not assert a particular internal optical algorithm.
- The R/G coordinate ramp provides an apparent sample centroid after a common
  lighting correction using its constant B channel. It combines reflected and
  refracted paths. Quantization limits horizontal precision to approximately 3pt
  and vertical precision to .8pt; marker centroids remain the stronger local test.

The viewer includes the original image, NXKit before/after, absolute RGB error,
and versions with exact or within 2/255 matches made transparent. These are fixed
stage crops, never fitted crops. Scoring masks are also visible.

## Reproduction

Build isolated adapters with `Demo/GlassComparison/build-lens-refraction.py`,
then run `Demo/GlassComparison/compare-lens-refraction.py` with the native run,
frozen before renderer, after renderer, and this output directory. The before
archive/header/source snapshot was taken before the selected-content changes.
The generated provenance records binary, archive, source, and capture hashes.

`LensRefractionTests` checks all four edge copies, sharp center ink, source
mutation, source opacity, and actual ancestor visibility. The same native-informed perimeter
regression compiled against the frozen previous composition fails all 16
style/theme edge-copy checks while its center-ink checks pass. It also fails the
measured near-rim disappearance check (17 failures in total). Both copy and
disappearance checks subtract an empty-source optical control, so the lens’s
intrinsic colored rim cannot masquerade as refracted ink. Source depths come from the native
one-point line sweep: 8pt for selection and 12pt for regular, rather than an
assumption that all nearby ink must stay visible.

## Current evidence and limits

The first UIKit pilot preserves straight center ink but pulls and splits ink
strongly near the perimeter. The previous implementation clips a sharp selected
overlay there. UIKit's source portal also respects actual ancestor invisibility;
an alpha-zero source host is not a valid way to suppress its ordinary draw.
Private native hidden-lens cases are compositor diagnostics and are not used as
no-lens controls. Ordinary NXKit view visibility continues to follow its public
view semantics.

The native session contains 177 captures: 38 validation scenes, 60 source and
shader controls, 64 one-point line profiles, and 15 private-configuration
diagnostics. This report renders 71 matched scenes: the 38 validation scenes,
27 required controls, and six representative line checks. The complete native
64-profile correspondence evidence is retained in
[`line-support.json`](../native/edge-probes/line-support.json). Diagnostics are
not counted as validation. Coverage includes both themes, both lens styles,
red/green/blue/striped backgrounds, selected-source visibility, and additional
sizes and fractional drag positions. Extra size and drag controls are principally
in the light theme; this is not a full factorial matrix.

## Results

Mean absolute sRGB channel error, on a 0–255 scale, averaged equally across
scorable validation scenes:

| Native-derived region | Before | After |
| --- | ---: | ---: |
| Original ink | 41.352 | 17.093 |
| Center ink | 8.086 | 1.096 |
| Refracted copies | 39.900 | 24.958 |
| Top copies | 31.932 | 28.452 |
| Bottom copies | 33.279 | 26.561 |
| Left copies | 47.559 | 23.615 |
| Right copies | 48.122 | 24.502 |

The aggregate center improvement includes visibility and layout controls.
Already-visible rich/color center ink remains unchanged: its mean error is
0.011 before and after. The final renderer preserves the native fractional-pixel
phase instead of rasterizing an additional shifted source image.

All 24 positive selected-source marker branches are now recovered, compared with
zero before; their combined copy-centroid error averages 0.369pt. Including four
ordinary-underlay positives, the broader marker set improves from 4/28 to 28/28
recovered, with 0.600pt mean centroid error after. These are isolated marker
checks, not a claim that every grid or text branch matches. Across the six
representative line checks, support intersection-over-union improves from 0.370
to 0.940 and ink-signal error from 6.156 to 1.038. Full per-scene, per-edge,
per-channel, and control measurements are in [`metrics.json`](metrics.json).

The final production archive is recorded in [`provenance.json`](provenance.json)
(SHA-256 `b8c24b5dfb55f511ae1a529cb15f954a9554581a94e2da1024f587a24e52e191`).
All ten focused raster suites pass, including `LensRefractionTests`; the iOS Demo
and lens regression targets also build successfully. Regressions cover native
edge disappearance, all four copied edges, channel ordering at straight and
curved edges in both themes, fractional-origin center phase, half-opacity
composition, source mutation, lazy text rendering, and source ownership and
visibility.

## Remaining differences

The selected-source refraction is substantially closer, but does not match every
native branch. In the dark regular-style rich-grid scene
`edges-dark1-style0`, the top-copy energy is only 0.135 of UIKit over 41 native
copy pixels, and the bottom-copy energy is zero over 14 native copy pixels;
the sides are much closer. Selection-style spectral bands remain sharper than
UIKit, especially at the top and bottom. Its top-copy error slightly increases
from 24.461 to 24.786 despite the overall copy improvement. Corner branch
softness, color separation, and centroids retain measurable residuals.

Ordinary-underlay transport, bare-lens lighting, and contact shadows retain
existing approximations. The ink/interior metrics exclude the exterior shadow
area, which remains visible in the full residual images. The raster comparison
does not establish Metal/Dawn output parity. Evidence is limited to the measured
iPadOS 27.0 runtime, and these static tests do not claim parity for
velocity-dependent elasticity, interactive timing, or every private UIKit
configuration explored diagnostically.
