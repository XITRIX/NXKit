# Historical eight-input adapter — 2026-11-05

Superseded by [the full translation](full-translation.md). The measurements below
apply to the first adapter, not the current production pipeline.

The production `UIGlassEffect` resolver now selects a versioned native input
profile before resolving portable optical defaults. Profiles compile into the
UIKit component module; production does not load JSON, call private UIKit APIs,
or resolve a material on every draw. The existing host material caches and trait
invalidation remain in use. Grouped and standalone glass use the same resolver.

## Source and reproducibility

`profiles.json` is the original iOS 27.0 / 24A434 capture: 280×80-point glass,
40-point radius, 3× scale, white/black backdrops. Its source hash is embedded in
`UIKit/lib/NativeGlassProfileData.hpp`. Regenerate or check it with:

```sh
python3 Demo/GlassComparison/NativeProfiles/generate.py
python3 Demo/GlassComparison/NativeProfiles/generate.py --check
```

The generator verifies the complete 18-case matrix, finite supported inputs,
70 keys per input dictionary, unique appearance/style/preference identities,
and equality of regular/tinted backdrop inputs. It emits 12 neutral backdrop
profiles; explicit accent color remains in the separate existing tint path.
It never derives reference values from NXKit output. All unsupported input
values remain preserved in the original evidence.

The resolver uses the captured 0, 0.5 and 1 positions. Linear interpolation
between these positions is an explicit portable policy, not a recovered Apple
interpolation law. The observed simulator's system setting is not treated as a
universal system default. The existing trait/platform preference bridge still
provides the actual application setting.

## Supported translation

Eight native inputs currently drive the portable adapter:

| Native input | Portable role |
| --- | --- |
| `inputBlurRadius` | Main contribution to the effective Gaussian sigma |
| `inputBlurFillBlurRadius` | Separate fill contribution to effective sigma |
| `inputBlurFillNormalOpacity` | Fill/preference response, independent of view alpha |
| `inputFaceColorMatrixBlack` | Relative color offset and gain |
| `inputFaceColorMatrixWhite` | Relative color gain |
| `inputFaceColorMatrixSaturation` | Relative saturation |
| `inputInnerRefractionAmount` | Optical displacement strength |
| `inputInnerRefractionHeight` | Bevel depth |

`GlassMaterialProfile.cpp` contains the explicit conversion and its native
reference anchors. Native radii and portable sigma are different units. The face
mapping is a relative transfer around the independently calibrated framebuffer
response, not a reimplementation of Apple's complete color-matrix pipeline.
The previous size/appearance/body/detail calibration remains. A future capture
version must be reviewed with a corresponding adapter and optical comparisons;
it must not silently replace the table's runtime/build identity.

Main and fill blur inputs remain separate in profile space. The first adapter
collapses them into one effective kernel. Regular uses a variance ratio against
the calibrated reference kernel; clear uses calibrated main and fill terms.
This adds no blur pass or backdrop capture. A separate blur-fill blend pass
requires measurements of Apple's blend ordering and equations before replacing
this approximation.

An attempted direct zero-native-radius → zero-Skia-sigma conversion passed
flat controls but failed textured controls (maximum amplitude error 25.21/255;
mean 3.06/255). The final adapter retains the measured 0.4-point residual at
native zero, as a calibrated approximation. Filtering in other native passes/resampling
and the difference between the 27.0 input capture and 27.1 visual fixture are
not isolated by this experiment. The 0.4 value is not a recovered native kernel.
This is why the input units and conversion must remain separate. It is not a
claim that Apple's main blur radius is nonzero at that endpoint.

Explicit `UIGlassEffect` optical overrides apply after profile translation.
They remain in portable units: for example, `setFrostRadius(0)` still disables
the portable Gaussian. Existing custom `BackdropEffect` and lens shaders retain
their APIs and behavior. The native table does not contain an accent hue: the
captured regular/tinted background dictionaries are identical.

## Verification and remaining scope

The profile-data check and material tests cover anchor selection, interpolation,
invalid preferences, independent sensitivity to every supported native input,
regular blur at zero preference, and optical override precedence. The retained
480 native color controls and 80 textured controls provide independent visual
regressions across both themes, five settings, both styles, and neutral/blue
tint on two geometries. Comparison uses simulator framebuffer captures,
not `drawHierarchy` reconstruction.

This is the first profile-driven adapter, not full native glass equivalence.
Outer refraction, separate face/fill blend equations, bleed, ring shadows,
SDF highlight/diffuse parameters, HDR/clamp behavior and accent color matrices
still use the existing approximations. Their native data is retained, but they
are deliberately not presented as applied merely because it is available.
The 80-point capture is not proof of every component/geometry variant, and the
physical iPhone's iOS 27.2 defaults have not been captured here.

The current visual matrix passes without loosening its original thresholds:
480 flat controls, mean absolute channel error **1.803/255**, maximum **14/255**;
80 texture controls, mean amplitude error **1.531/255**, maximum **9.011/255**.
The adapter intentionally preserves the calibrated appearance at the current
captured values. Its purpose is to make native inputs usable through a tested
conversion; this change does not claim improved visual parity for unmapped passes.
See `adapter/` for measurements, comparison images and build/Metal verification.

Ten focused suites and macOS, signed iOS, and Switch builds pass. Fresh offline
bundles contain 162 libraries / 129 pipeline descriptions for source ABI
`13df58b6…`. Archive-only GPU replay covers 4,032 frames in 18 fresh-cache
profiles: push/pop in both themes, fixed preferences, and 161-position sweeps
at 1×/2×/3×. All frame counters report zero runtime library/pipeline compilation.
Raw content-addressed programs from the prior catalog are retained with
provenance; GPU libraries and native archives are freshly built for both SDKs.
This is local Mac GPU validation; the new build was not installed or visually
validated on the physical iPhone in this task.
