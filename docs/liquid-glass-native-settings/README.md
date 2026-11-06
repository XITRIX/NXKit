# Native Liquid Glass configuration investigation

Actual resolved parameters are readable at runtime. The useful source is the
`glassBackground` CAFilter attached to each rendered `CABackdropLayer`, together
with its SDF mask/highlight/color layers. A single legacy material recipe or a
single blur/tint scalar does not describe this pipeline.

This investigation exports **18 profiles, each with 70 glass-background inputs**:
regular, clear and regular-with-system-blue-tint; light/dark appearance; and
system/0/1 tint preferences. It also preserves the surrounding layer stacks,
color matrices, runtime method encodings and reversible mutation checks.
The subsequent [full translation](full-translation.md) consumes the complete
material stack using 560 additional size/style/preference observations and
controlled pass experiments. The original capture evidence below is unchanged.

## Evidence and coverage

- **E1 — controlled runtime replay:**
  `ev_a09b0b5ac66a2a8622fa23e817a59e7d0e0be4f754d3c8a9259f6d57b2c4f112`,
  [complete REA capture](rea-runtime-evidence.json). The checked-in driver exits
  successfully, produces six fresh UUID-identified captures and passes all
  18 before/change/restore checks. Capture is not truncated. REA observes the
  driver/simctl processes and output hashes; CoreSimulator launches the UIKit
  process separately. Sampled process trees, inherited host environment and
  network activity retain the limitations in the capture.
- **E2 — shipped resource inspection:**
  `ev_e00854df8117e76d66bc69b174359a9714ce045a1ec85e79ad49b2f5828aa6bd`,
  [REA CoreMaterial recipe inspection](rea-resource-evidence.json).
  [Resource inventory](resources.json) compares all 71 recipe/style files from
  iOS 26.0/23A343 and 27.0/24A434. All 45 material recipes are byte-identical.
- Environment: **iOS 27.0, build 24A434**, iPhone 17 Pro simulator, 3× display,
  SDK 27.1. Each effect is 280×80 points with a fixed 40-point corner radius.
  Backgrounds are uniform system black/white for dark/light appearance.
  The system preference resolved to **0.5 on this simulator**; that is an
  observed environment value, not a universal default.
- [Verification manifest](verification.json) records source/binary/capture
  hashes and coverage. [profiles.json](profiles.json) contains all 1,260 named
  input values. `captures/*.layers.json` preserves full filter/SDF/color stacks;
  compressed raw captures retain archives and runtime metadata.

The Hopper bridge timed out twice during startup, including after a health
check. No decompilation or recovered shader-equation claim is made. REA's keyed
archive inventory was unavailable for this target; the checked-in Python decoder
reads plist UID references as data without instantiating archived classes.
REA's process capture and shipped plist inspection succeeded.

## Applied settings, rather than generic defaults

Example values below come from the **dark, system=0.5** resolved profile. These
are native filter input values; they are not established as Skia Gaussian sigma
or portable pixel/point equivalents.

| Native input | Regular | Clear |
| --- | ---: | ---: |
| `inputBlurRadius` | 5 | 0.75 |
| `inputBlurFillBlurRadius` | 8 | 0 |
| `inputInnerRefractionAmount` | -40 | -52 |
| `inputInnerRefractionHeight` | 20 | 20 |
| `inputOuterRefractionAmount` | 20 | 16 |
| `inputOuterRefractionHeight` | 16 | 10 |
| `inputFaceColorMatrixBlack` | 0.125 | 0.125 |
| `inputFaceColorMatrixWhite` | 1.125 | 1.08 |
| `inputFaceColorMatrixSaturation` | approximately 1.3 | approximately 1.06 |
| `inputShadowOpacity` | approximately 0.13333 | 0 |

The backdrop scale is 0.5. The captured highlight pass uses
`CASDFKeyFillHighlightEffect`, including curvature 0.75, key/fill amounts 0.5,
key/fill heights 1, opposite light angles and separate diffuse controls.
Those applied values are more useful than the generic
`CASDFGlassHighlightEffect.defaultValues` dictionary, whose curvature is 1 and
height is 20. Generic defaults and resolved material settings are distinct.

`_UIViewGlass` exposes descriptors such as variant, size, tint, grouping,
content lensing and shadow exclusions. Its
`_resolvedMaterialUsingTraitCollection:size:` selector returned nil for these
cases, despite visible native glass. Inspecting the actual render layers and
archiving their filters supplied the resolved inputs instead.

## System tint and accent color are separate controls

At tint amount **0**, regular glass retains `inputBlurRadius=5` and its
refraction/face/shadow parameters. The clear style's blur radius becomes 0 in
this fixture. Regular glass does not simply become the clear style.
`inputBlurFillNormalOpacity` changes with the setting, while other tone controls
retain nonzero values or change independently. Three samples do not establish
an interpolation law between them.

Regular and accent-tinted glass have **identical 70-input backdrop dictionaries
in all six captured appearance/preference cases**. The tinted effect adds an
SDF gradient/fill/mask pass and a separate backdrop-aware `vibrantColorMatrix`.
The matrices and colors are preserved in the layer exports. This supports
keeping the accent-color path separate from global tint-amount modulation;
multiplying the entire accent material by that slider is not the observed
composition.

## Runtime mutation

The probe reads an active filter's `inputBlurRadius`, sets it to `before+1`,
reads the changed value, restores the original and verifies the restoration.
All 18 checks pass. For example, regular glass accepts 5 → 6 → 5; clear glass at
system=0.5 accepts 0.75 → 1.75 → 0.75.

This establishes writable runtime filter inputs. The mutation is restored before
presentation; it does **not** establish the visual response or units of the
modified parameter. The fixture's `UIViewGlassTintAmount` overrides use only the
process-local argument domain. Persistent device/user preferences are unchanged.

## What can transfer to NXKit

The exported numbers can become versioned source data for portable material
profiles. The renderer still needs equivalent behavior for the corresponding
native passes:

1. Separate main backdrop blur and blur-fill/tone composition.
2. Use inner and outer refraction controls rather than assuming one scalar.
3. Preserve the face color transform, bleed, shadows and highlights as distinct
   operations with their own distances, opacities and blend rules.
4. Apply accent coloring through its own matrix/gradient/mask path.
5. Compare the translated profiles against the existing native fixtures before
   replacing calibrated values, including offline Metal pipeline coverage.

`UIKit/lib/GlassShader.cpp` currently has portable uniforms such as `refraction`,
`curve`, `saturation`, `contrast`, `luminosity`, `tintMode` and specular controls;
`CAGlassContainerLayer.cpp` uses Skia blur. This is a different parameterization.
Copying native numbers into similarly named uniforms would not establish
behavioral equivalence. Native filter equations, sample coordinate/blur units,
blend ordering and color-space behavior still need translation/verification.

The four shipped `platformContentGlass*` resources all contain blur radius 45
and the same matrix, even across iOS 26/27. They are genuine material data, but
no evidence here maps them to the captured `UIGlassEffect` path. Do not substitute
that recipe for the resolved profiles.

Remaining coverage: physical iOS 27.2 device values; other OS releases; other
sizes, radii, scales and backgrounds; navigation/tab/control-specific variants;
interactive/deformed/merged states; accessibility and HDR behavior. These are
build- and context-specific resolved profiles, not a claim of one immutable
configuration for every Liquid Glass component.

## Reproduce

Use the already booted research simulator's identifier, or boot one running the
specified iOS 27.0 runtime. The app and private API calls are research fixtures,
separate from the production framework.

```sh
python3 Demo/GlassComparison/SettingsProbe/build.py /tmp/nxkit-native-settings
xcrun simctl install YOUR_SIMULATOR /tmp/nxkit-native-settings/Probe.app
python3 Demo/GlassComparison/SettingsProbe/capture.py \
  --device YOUR_SIMULATOR --output /tmp/nxkit-native-settings/captures
```

The capture driver verifies a new run identifier for every case and checks all
mutations are restored. `decode.py CAPTURE.json` can regenerate a profile file.
Binary extraction used Xcode's shipped `dsc_extractor.bundle`; extracted Apple
framework binaries remain temporary research inputs and are not included here.
Apple documents that extractor's entry point in its
[dyld source](https://github.com/apple-oss-distributions/dyld/blob/main/other-tools/dsc_extractor.cpp).
