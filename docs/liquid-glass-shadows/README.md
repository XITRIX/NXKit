# Default glass shadows

Native target: iPad simulator iOS 27.0, UIKit built with the iOS 27.1 SDK, 28 October 2026. These are observations of public `UIGlassEffect` and `UIGlassContainerEffect`, captured from the real simulator framebuffer. They do not establish behavior on other OS releases.

The old default was a light-theme-only pair of blurred masks. It produced a broad halo around small items, insufficient elevation around large items, and an unwanted shadow on clear glass. The replacement uses a downward-shifted signed-distance field and a Gaussian survival function. The native profiles favor this field over convolution of a filled shape: a small member in a large container retains the same edge profile as the large member.

## Observed behavior and model

- Regular glass casts a black exterior shadow in both themes. The tested neutral, fixed blue and fixed coral tints have identical exterior pixels on a given background.
- Clear glass has no automatic shadow in either theme, including shared containers.
- The downward offset is approximately 8 logical points.
- Elevation grows with the shortest member dimension from roughly 64 to 160 points, then saturates in the sampled range.
- A container shares its largest member's elevation with every member, including distant disconnected members. Two 52-point members remain at small elevation when their aggregate bounds grow through diagonal placement. Container frame size and item spacing do not determine elevation.

For `t = clamp((min(width,height)-64)/96, 0, 1)`, the rounded generic fit is:

| Parameter | Light | Dark |
| --- | --- | --- |
| Sigma, points | `2.7 + 14.1*t` | same |
| Downward offset, points | `8` | same |
| Peak opacity | `.012 + .108*t` | `.012 + .168*t` |

The black alpha is `opacity * GaussianSurvival(shiftedDistance/sigma)`, inverse-clipped by the original glass contour. This is a native-calibrated model, not an image correction or a color-specific lookup. `profile-fits.json` preserves independent per-size diagnostic fits; those unrestricted per-case values are not used in production.

The backdrop is sampled before the shadow is drawn, preventing the glass from refracting its own shadow. Explicit application shadows retain precedence, and `shadowOpacity` scales or disables the automatic shadow. Group shaders keep the original merge contour, ownership and material optics. Shadow geometry is evaluated at the shifted point with recording disabled, so it cannot overwrite material weights. Affine offsets and sigma covariance follow the item transform. Backing scale remains separate from logical elevation.

## Evidence

The original 60-scene optical matrix supplies five geometries (52, 80 and 120 circles, a 132×56 pill and a 240×160 continuous card), both themes, neutral/blue/coral and ten backgrounds: black, white, gray, red, green, blue, yellow, ramp, lines and stripes. It is retained under `docs/liquid-glass-optics` and `/private/tmp/nxkit-optical-matrix/native` in this session.

Twenty-four additional shadow scenes cover 64/96/144 circles, the pill/card, standalone versus shared containers, regular versus clear, white/gray, a close pair alongside large members, and horizontal/diagonal pairs containing only small members. The new sRGB crops, original framebuffer PNGs, request/geometry metadata and executable/source hashes are retained in `native/`. `ShadowReference-first.m` preserves the exact source for the first 20 captures; the final source adds only the four small-only controls.

Original 60-scene exterior RGB MAE decreased from **0.8393 to 0.0101** levels on the 0–255 scale. Light decreased from 0.6317 to 0.00770, dark from 1.0468 to 0.01248. Every tested nonblack background improved; black remains exact. These measurements exclude body/rim pixels and therefore do not claim material parity. The complete **84-scene** comparison decreases mean RGB MAE from **1.14077 to 0.01445** (98.73%). Light: 0.87497→0.01099; dark: 1.40657→0.01791. Standalone: 0.85460→0.01065; shared containers: 2.57160→0.03347. Clear-glass exterior error is exactly zero in all 40 sampled element regions. None of the 408 element regions worsens. The largest remaining mean error is 0.174 RGB levels on a 144-point dark circle in an elevated shared container. Full per-region results are in `metrics.json`; masks, transparent overlays and montages are in `images/`.

Measurement uses observed native geometry, exterior distances 1.5–40 points, exclusions for other item bodies and conservative continuous-card corner bands. Group masks exclude the smooth-union bridge. Embedded native ICC profiles are converted to sRGB before comparison; originals are preserved. There is no fitted translation, contrast or color correction. Metrics average per-element RGB errors; saved masks plus per-element pixel counts, MAE and P95 prevent a single aggregate from hiding differences. Difference montages amplify exterior errors 8×. Transparent residual overlays use red for a weak shadow and blue for excessive darkness, opacity amplified 32×.

![Native, old, new and exterior difference: dark regular glass on white](images/white-dark1-none.png)

![Shared maximum elevation: dark container on white](images/white-dark1-clear0-group1.png)

## Regression coverage and limits

`GlassShadowTests.cpp` verifies native exterior probes at 1×/2×/3×, both themes, all tested tint/background colors, clear glass, custom-shadow precedence, opacity updates, inverse clipping, absence of self-shading/refraction, exact one-item container behavior, shared maximum elevation, hidden-member updates, diagonal bounds and affine container/standalone agreement. The private candidate build passes this harness; the parent task verifies the combined production builds and complete regression suite.

Standalone rendering adds one cached runtime shader and no intermediate image/filter pass. Shared rendering adds a shifted union-distance evaluation only for exterior or antialiased boundary pixels, with no additional texture or filter child. Shadow allocation includes the extended profile. GPU performance has not been measured.

Native affine transforms, mixed clear/regular groups, different transforms inside one shared group, arbitrary application alpha, animated style/size changes and other OS versions have not been sampled exhaustively. Transform tests establish internal coordinate consistency, not native equivalence for every transform. The large-size saturation and interpolation between sampled dimensions remain a generic approximation.

## Reproduction

Build `Demo/GlassComparison/ShadowReference.m` as a UIKit simulator app with UIKit, QuartzCore, Foundation and CoreGraphics, bundle identifier `org.nxkit.ShadowReference`, including the optical matrix background PNGs. Use the installed SDK reported by `xcrun --sdk iphonesimulator --show-sdk-path`; preserve the device runtime in the report. Install/capture with:

```
python Demo/GlassComparison/capture-shadows.py DEVICE OUTPUT CASES_JSON
```

`OUTPUT/ShadowReference.app` must exist. The case lists are saved beside this report. The tool waits for matching acknowledged requests and captures the simulator framebuffer, retaining original color metadata.

Configure the existing headless raster project with the matching prebuilt Skia archives and build `ShadowRenderer` and `ShadowTests`. Arrange `MATRIX/native/series.json`, its native PNGs, and `MATRIX/backgrounds/<name>.png`, then render each frozen core version and compare:

```
python Demo/GlassComparison/render-shadows.py SHADOW_RENDERER MATRIX RENDER_OUTPUT
python Demo/GlassComparison/compare-shadows.py MATRIX BEFORE_OUTPUT AFTER_OUTPUT REPORT_OUTPUT
```

The native source, renderer source, frozen archive and candidate archive hashes are in `provenance.json`. The baseline is HEAD `9345af9631a4e01188c1862bf1a9a31d5efc9cbe`, before shadow changes. No native image has been recolored or modified to improve the fit.

## Shared neutral material follow-up

The extra mixed-size fixtures also identified a separate material sizing mismatch. [The measured correction and body residuals](shared-material/README.md) are reported separately from the isolated exterior shadow fit above. It improves neutral group body MAE11.605→0.466, while preserving all96 existing two-button rasters byte-for-byte.
