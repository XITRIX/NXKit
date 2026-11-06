# Regular glass: exterior contour and directional highlights

[Open the aligned pixel comparison](comparison.html), or the
[actual four-tab resting/held comparison](tabs/comparison.html).

The pale regular material previously had a white reflection but almost no dark
exterior contour. On a pale backdrop it lost its silhouette. Earlier optical
scores measured the rim .75–2.75 points **inside** the glass, omitting the
half-point contour immediately **outside** it.

The correction adds that exterior contour separately from the frosted body,
refraction, inner directional reflection and elevation shadow. Light regular
glass also retains a tight white reflection core when its body approaches white.
The contour attenuates at the two reflection poles; it is not a constant black
stroke or an all-around white border. In dark appearance its density increases
with the short side and depends on the underlying luminance. 52/80/120-point
controls were used in choosing the density and interpolation; they are
calibration checks, not independent holdouts. Pills and the continuous card
provide additional geometry checks.

![Updated NXKit Demo on a pale background](demo-pale-after.png)

This supplemental 3× capture shows the actual Demo after the correction.
[Before](demo-pale-before.png) and [capture provenance](demo-pale.json) are
retained; this supplemental scene has no native pixel score.

## Comparison method

The main matrix reuses 60 native UIKit 27.0/24A434, 2× sRGB framebuffer captures:
both themes × neutral/blue/coral tint × ten backgrounds. Each scene contains
52/80/120-point circles, a 132×56 pill and a 240×160 continuous-corner card.
Frames come from observed native geometry. There is no fitted registration,
exposure, color transform or shape warp.

The renderer uses the production retained layer/material path. Shared inputs
are checked against an untouched region of every native framebuffer before
scoring. Metrics weight scenes and geometries equally and separate:

- **Exterior:** 0–.75pt outside the boundary, including the existing shadow.
- **Highlight:** 0–3pt inside, including the near-edge directional reflection.
- **Body:** more than 18pt inside.

Continuous-card corner neighborhoods are excluded from edge metrics because
the native private curve is approximate. All five geometries contribute to the
scores; the viewer crops show the first four at full pixel resolution. Native,
before/after, 50% overlay, absolute difference and removal of matches within
2/255 are available. The threshold affects only the transparent visualization.

Mean absolute RGB errors below use 8-bit sRGB channel units, with equal weighting
over the five geometries:

| Region and scenes | Before | After |
| --- | ---: | ---: |
| Neutral light/white exterior | 21.567 | 9.416 |
| Neutral dark/gray exterior | 14.425 | 8.347 |
| All neutral exteriors | 17.235 | 12.417 |
| All 60 exteriors | 33.098 | 28.681 |
| All 60 inner highlights | 5.049 | 5.037 |
| All 60 bodies | 1.118 | 1.118 |

Across all tints and backgrounds, exterior error improves 49.547→42.818 in
light appearance and 16.649→14.544 in dark. Inner highlight error changes only
slightly overall; dark/white neutral highlight error regresses 3.664→3.806.
Native light tinted contours still have substantial color differences. Body
scores remain unchanged, and all six black-background matrix outputs are
byte-identical before/after. These results improve edge definition without
establishing complete pixel parity.

Full results and file hashes are in [metrics.json](metrics.json). Earlier
inner-rim/body scores remain historical in the optics/refraction reports; their
region definitions differ and should not be compared directly to this exterior
score.

## Rendering and regression coverage

`GlassRim.hpp` contains the shared exterior law. Standalone glass draws a
bounded inverse-clipped contour pass after sampling its optical body and before
the elevation shadow. Its additional input extent is 3 local points. A 2-point
outward read obtains backdrop color beyond the material's antialiasing footprint;
subtraction is capped by that backdrop contribution. This avoids darkening the
partially covered material itself on black or transparent backgrounds.

Containers evaluate the same law on their **merged** SDF and normal, with the
active owner's local metric and actual short side. They apply one exterior
contour after material blending, leaving internal joins unoutlined. Existing
independent shadow and touch-light behavior remain separate. The contour honors
edge strength, absorption and specular width; setting `shadowOpacity` to zero
removes elevation while retaining the optical contour. Clear style retains its
existing exterior. Explicit view/ancestor clipping can clip the outside contour.

`GlassRimTests` verifies native light/dark gray/white exterior probes, dark
52/80/120-point white/gray/green/yellow controls, point width at 1×/2×/3×,
edge/clear overrides, merged internal edges, hiding/removal/reparenting and
animated presentation placement. Existing material, container, shadow,
refraction, actual tab touch and full-screen suites also cover the changed path.
The actual UITabBar comparison uses ten retained native phone captures over
neutral, blue, brown, coral and green backgrounds in both themes, including
resting and held phases. Its before pixels are retained from the previous
tab-content report; the production Demo is rendered again after this change.

The iOS Demo is built with signing disabled. These checks do not establish
physical iOS 27.2, Metal/Dawn pixel or performance parity. Native tinted-light
contour color, curved-edge coverage, exact spectral/refraction response and
full tab appearance still differ; residuals and individual regressions stay
visible. No new native framebuffer capture is claimed for this follow-up.

## Reproduction

Build the raster harness targets `RimTests`, `MaterialTests`, `ContainerTests`,
`ShadowTests`, `LensRefractionTests`, `LensTabTests` and `GlassScreenTests`, then:

```sh
ctest --test-dir /path/to/raster-build --output-on-failure -j2 \
  -R '^(RimTests|MaterialTests|ContainerTests|ShadowTests|LensRefractionTests|LensTabTests|GlassScreenTests)$'
python3 Demo/GlassComparison/compare-glass-rim.py \
  /path/to/before/OpticalRenderer /path/to/after/OpticalRenderer \
  docs/liquid-glass-rim
```

`build-optical.py RASTER_BUILD OUTPUT --renderer-only --core-archive=ARCHIVE`
links the same adapter against a frozen core. The optional
`--before-images=DIR` reuses complete, unmodified `ID-before.ppm` frames from
that frozen renderer; their hashes are recorded. The report's source, capture,
background pixel and renderer hashes identify the exact inputs.

The original independent public-UIKit fixture and capture procedure remain
in [`Demo/GlassComparison`](../../Demo/GlassComparison/README.md) and the
[optical report](../liquid-glass-optics/README.md).
