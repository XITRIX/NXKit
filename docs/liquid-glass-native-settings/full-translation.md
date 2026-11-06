# Full native-profile translation — 2026-11-05

The production `UIGlassEffect` path now translates the complete captured material
stack. It replaces the historical eight-input relative-calibration adapter;
that adapter's results remain in `portable-adapter.md` as historical evidence.
No private UIKit API or profile JSON is needed by the shipped application.

The initial spatial reconstruction had visible refraction and pass-order errors.
The follow-up [spatial correction audit](spatial-corrections.md) supersedes those
shader equations and the verification totals below. Complete captured settings
do not imply that every private shader equation has been recovered.

## Captured configuration

The compiled table contains 560 iOS 27.0/24A434 observations: shortest dimensions
44/52/62/64/80/120/160pt, circular and wide hosts, regular and clear styles,
neutral and explicitly colored materials, light/dark, and tint preferences
0/.25/.5/.75/1. Each record preserves all 70 `glassBackground` inputs, output
mask limits, SDF key/fill/diffuse settings, the vibrant color matrix, accent
mask/fill and cubic timing control points, backing scale and provenance.
`NativeProfiles/generate.py --check` verifies raw archive/source hashes and the
compiled data. See [runtime experiments](pass-semantics/README.md).

The 192 captured palette matrices are now test references. Production uses the
native DesignLibrary CPU equations for arbitrary sRGB tints: HSL adjustments
and luminance derivatives for chromatic colors, with a separate achromatic
composition. The equations reproduce those matrices and 32 independent
withheld matrices within 0.000001 coefficient error. See
[color derivation](color-transfer/native-cpu-accent.json).

The host's live shortest dimension selects/interpolates measured profiles.
Values outside the measured size range clamp to its endpoints. Between measured
preference and size anchors, numeric interpolation is a portable policy; flags,
text and context select the nearest observation. Face-fill coefficients
interpolate as straight RGBA; ordinary layer colors interpolate premultiplied.
The gradient's captured premultiplication flag controls its own interpolation.
A future native capture must retain its runtime/build identity.

## Pass translation

| Captured family | Production implementation |
| --- | --- |
| Blur radius, four distances/opacities | Native 13-tap mip pyramid; distance-dependent radius and continuous LOD sampling |
| Blur-fill radius and normal/lighten/darken opacities | Unrefracted two-point mip sample; darken/lighten before normal fill |
| Inner/outer refraction amounts/heights | Independent spherical-profile source samples along the live SDF normal |
| Refraction distances/opacity | Linear signed-distance blending of the separate outer sample near the rim |
| Face black/white/saturation/max-luma/fill/opacity | Independent luminance endpoint and chroma transfer, SDR coefficient selection and face-fill composition |
| SDR holding tone/distances/white, max headroom/clamp/hue | SDR tone ramp and output color limits; native cap is not mistaken for actual display headroom |
| Key/fill filter highlight angle/height/spread/SDR spread/bias/offset | Internal highlight before the external SDF highlight layer |
| Shadow radius/offset/distance/blur/height/amount/color/vibrancy/opacity/SDR opacity | Spherically displaced backdrop under shifted live-SDF exterior shadow |
| Ring blur/stroke/mask/offset/opacity | Independent ring coverage |
| Bleed amount/height/distances/blur/color/darken/opacity | Independent transmitted-color contribution |
| Source sublayer identity | Original backdrop source selection; captured source labels remain provenance rather than application view identifiers |
| Output minimum/maximum | Finite exterior coverage and host drawing outset |
| SDF key/fill angles/heights/amounts/spreads/scales/offsets/colors, curvature/diffuse/global/matrix | Native Schlick-biased SDF key/fill/diffuse fields, local/world normals and vibrant transform |
| Accent gradient colors/distances/premultiplication/timing, fill/compositing/matrix | Separate style-specific accent pass, independent of global material tint amount |
| Backdrop scale | Capture downsampling before filtering and reconstruction; untouched original remains available for composition |

The face response is independently translated from the shipped QuartzCore AIR
and verified by fresh-filter-copy native framebuffer perturbations:

```
y = dot(source, Rec709)
f = saturate(1 - y*(1 - maxLumaSDR))
l = f*y
q = l + (source*f - l)*(1 + 0.3*(1-f))
face = black + (white-black)*l + saturation*(q-l)
face = mix(face, fill.rgb, fill.alpha)
```

The 0.3 chroma compensation is required: omitting it caused the earlier dark
green mismatch. Endpoints change luminance independently of chroma. SDR uses
MaxLumaSDR; HDR behavior has not been measured. Holding tone is a separate
premultiplied multiplicative-white pass after material coverage and filter
highlight, rather than a luminance ceiling inside face transfer.

Core's generic `BackdropEffect` now supports named, independently filtered
samples of the same backdrop, capture resolution, and output extent. Equal
kernels share DAG inputs. An explicit sampling grid prevents the inverse matrix transforms from cancelling
the capture at zero blur. Groups use the same material source as standalone
views, preserve actual member geometry and transformed normals, and retain a
common neutral material context. Group source pixels are sampled once from the
original backdrop rather than from a previously composited member. Explicit
layer clipping remains authoritative. Larger groups shade batches of at most
three materials, then sum weighted premultiplied results and restore the
remaining original-backdrop weight. All batches evaluate the same full union
and ownership. This fits the 16-sampler budget without feeding one batch's glass
back into another. A four-member regression checks opacity, antialiasing and
untouched backdrop pixels at 1×/2×/3×. Public optical overrides remain usable;
`frostRadius` is still Gaussian sigma in local points and bypasses the native
radius conversion and reduced-resolution capture.

## Reconstruction boundaries

The [spatial correction audit](spatial-corrections.md) supersedes the initial
shader reconstruction and verification below. Numeric shader equations are
independently translated from readable AIR in the shipped simulator QuartzCore
library; CPU accent equations come from DesignLibrary. Captured settings remain
separate from shader math. Apple binaries and full disassemblies are not shipped.

The native blur radius selects fractional levels of a 13-tap pyramid, rather
than a fixed Gaussian sigma. Refraction uses separate spherical inner/outer
samples and evaluates each sample's blur radius at its displaced SDF. Bleed,
ring, exterior shadow and the filter microcontour precede separate accent and
external SDF lighting. Explicit optical overrides keep their portable semantics.

Profile interpolation between captured sizes/preferences remains portable
policy. Default profiles are from iOS27.0/24A434 simulator; other OS versions,
accessibility recipes, out-of-gamut/HDR rendering and physical GPU precision
are not established by these captures. See the current audit for exact test
coverage and remaining limits.

## Historical verification of the initial port

These results belong to the initial configuration port, before the AIR-based
correction. They do not describe the current working tree; use the
[spatial audit](spatial-corrections.md) for current tests/builds.

The 480 then-current flat-color controls pass unchanged tolerances (direct shader:
mean 0.170139/255, maximum 7/255). The retained-tree 80 texture controls pass
(mean amplitude error 1.237556/255, maximum 7.327343/255). Three older iPadOS 27.0
neutral 120pt controls failed their original tolerances: light yellow
[255,255,31] versus [255,255,14], dark green [3,180,3] versus [0,187,0], and light
brown [244,207,178] versus [251,212,181]. These failures were retained with their original reference values and tolerances;
the subsequent AIR-based face and pass-order corrections resolve them.
The older clear-black smoke expectation of 26 was replaced by the explicit
32/255 value present throughout the independent 480-control current fixture.

[Verification manifest](full-translation/verification.json), compressed strict
Metal replay logs, comparison images and build summaries accompany this report.
The initial port ABI was `a5e37d7c0a8c1dc687929d0f272a40c79adfb7992690d4e408e9e3166b8f9e0d`.
Fresh macOS/iOS bundles contain 180 libraries and 138 pipelines. The signed iOS
app was verified and installed on the connected iPhone. Mac and Switch builds
also succeed. All 37,566 archive-only replay frames use zero runtime library or
pipeline compilations across full navigation, preference anchors and 1×/2×/3×
sweeps in both themes. That material-suite run reported the three older optical
residuals above; the other 14 focused suites passed. Original native color and texture fixtures are never generated from
NXKit output or relaxed to fit the new implementation. Device visual testing
remains the next check, particularly animated grouping, clear and colored glass,
short buttons versus wide tab bars, and the tint-preference endpoints.
