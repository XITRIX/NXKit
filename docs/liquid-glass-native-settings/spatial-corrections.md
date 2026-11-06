# Liquid Glass shader correction — 2026-11-05

The earlier configuration port still reconstructed several shader equations
incorrectly. The reported pinched refraction led to a direct inspection of
readable AIR in the shipped simulator QuartzCore library and the native CPU
accent generator in DesignLibrary. The current implementation independently
translates those numeric operations; Apple binaries and full disassemblies
remain outside the repository.

## Corrected passes

* **Refraction:** independent spherical inner and outer rays replace the
  summed smoothstep displacement. The outer ray has its own signed-distance
  mask. Each ray evaluates its blur radius at its shifted distance. The
  [coordinate fixture](pass-semantics/refraction/README.md) covers both axes,
  circles, capsules, changed amounts/heights and expanded masks.
* **Blur:** a native 13-tap mip pyramid replaces both the original fixed
  Gaussian and the interim fitted Gaussian mixtures. Radius is converted to
  continuous LOD; four distance/opacities vary radius rather than mixing one
  fixed blur with sharp pixels. The unrefracted blur-fill sample averages two
  opposite quarter-LOD offsets; darken/lighten precede normal fill. Core owns
  the generic filter atlas while UIKit owns native taps and radius semantics.
  See [kernel evidence](pass-semantics/kernel-step/README.md).
* **Face:** native quadratic luminance compression includes the separate
  `1 + 0.3*(1-compression)` chroma compensation. This resolves the previous
  dark-green mismatch. Endpoint luminance and chroma saturation remain
  separate, verified against 72 independent native perturbations.
* **Lighting:** key/fill/diffuse amounts are Schlick bias parameters, rather
  than paint amplitudes. Native distance bands, angular cones, diffuse scales
  and vibrant destination-color transfer replace the fitted crest/tail.
  Against 2,836 isolated interior native points, maximum error is 1/255.
  See [lighting evidence](lighting-spatial/README.md).
* **Bleed, shadow and ring:** bleed uses the native squared distance mask and
  fourth-power luminance gain, with its own displaced pyramid sample. Shadow
  amount displaces its backdrop; it does not darken the material interior.
  Shifted shadow and one-sided ring coverage use the native bounded polynomial.
* **Microcontour and holding tone:** filter highlight follows material
  coverage/shadow/ring composition, including native face extension and cubic
  color adjustment. Holding tone is a separate premultiplied multiplicative
  white pass. [Native holding-tone captures](pass-semantics/holding-tone/README.md)
  corroborate its AIR equation. Accent and external SDF lighting follow.
* **Accent:** the native CPU HSL and achromatic equations replace palette
  interpolation. All 192 original and 32 withheld matrices agree within
  0.000001 coefficient error. Production no longer includes the captured
  palette. See [CPU derivation](color-transfer/native-cpu-accent.json).
* **Coverage:** materials without exterior output use the host's single
  antialiased path. Materials with native exterior output preserve its live
  SDF extent. Standalone and grouped glass share the same pass equations.
* **Display scale:** same-screen UIKit trait overrides isolate scale from
  hardware and phone/tablet idiom. The microcontour uses1px with bias−.25 at1x/2x,
  and1.6px with bias−.3 at2.5x/3x. Its point height is that pixel width divided by
  scale, and its offset is the negative height. All336 independently captured2x
  material controls preserve the other scalar settings from3x. Application
  optical overrides are applied after resolving these native defaults.

## Evidence boundaries

The native target is iOS27.0/24A434 simulator. Static AIR/CPU evidence establishes
numeric operations; framebuffer captures establish the finite tested results.
This does not establish identical output on every GPU or OS release.

Size/preference interpolation between the 560 captured profiles remains a
portable policy, as does selecting the denser microcontour above2x at other
fractional scales. Arbitrary accessibility recipes, HDR/out-of-gamut rendering,
nondefault private flags and physical-device precision require separate
coverage. Invalid SDF-copy experiments remain excluded despite successful KVC
readbacks. Earlier rejected fits and initial translation totals are historical,
not current parity claims.

## Verification

All 18 integration regressions pass. The independent 32 step and 32 stripe blur
controls have mean errors 0.388509/255 and 0.803223/255; 480 material controls have
mean error 0.0347222/255 and maximum 1/255. Refraction covers 4,814 decoded source
coordinates. Native pixel references and tolerances remain unchanged; the
synthetic constant-point-width rim assumption is superseded by the measured
display-scale recipe.

The final offline bundles contain 200 source libraries and 151 native pipeline
descriptions. All 37,566 frames across 18 strict Metal profiles complete with
zero runtime library/pipeline compilations. A fully cold physical-phone launch
also captured two R8 stencil-only descriptions missing from the initial
catalog; see [device evidence](spatial-corrections/device-cold-start/README.md).
The final iPhone build passes bounded strict launcher and scrolling checks and
is left running normally. macOS/iOS source builds and the Switch NRO build pass.

The final source, native comparisons, regression tests, platform builds and
matching offline Metal archive are recorded in
[verification.json](spatial-corrections/verification.json). This report separates
source builds, actual runtime checks and physical-device validation.
