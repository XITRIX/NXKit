# Native glass pass and size probes

Captured 2026-11-05 from UIKit on iOS 27.0 (24A434), iPhone 17 Pro simulator, display scale 3. Built against the installed iPhoneSimulator 27.1 SDK. No NXKit renderer is linked into these probes. No physical-device settings were changed.

## Evidence and reproducibility

`Demo/GlassComparison/NativePassProbe` contains the native fixtures and drivers. Build `Probe.m` with `build.py OUTPUT`, or the size fixture with `build.py OUTPUT --source Demo/GlassComparison/NativePassProbe/SizeProbe.m`. Install the resulting `Probe.app` into the research simulator, then run the corresponding capture driver. `decode-sizes.py` decodes NSKeyedArchive property lists as data, without instantiating archived classes.

The base size replay is REA evidence **ev_3904c57036cd3d9512b0caab1220949cef603e2974b285ddbbab507bac5cb303**; its manifest, exit, process samples, six fresh-run signals and file hashes are in `rea-size-evidence.json`. Exit is zero and capture truncation is false. Four additional quarter/threequarter preference launches are evidence **ev_fbe314386df85ee49ca07ef1ebc59e8ad78d55d145c22f40a996c75d1806b2f5**, preserved in `rea-intermediate-size-evidence.json`, also exit zero and not truncated. REA observes the Python/simctl driver and output files; it does not trace UIKit shader execution inside the simulator. Process sampling and inherited-environment limitations still apply.

The initial simulator cold-start replay timed out without producing captures. The subsequent three-case replacement-filter replay succeeded: **ev_4e622ccb73096403da9315bc55ef4a93744fa315cf0ec8bc7c72a3645a756932** (filesystem snapshot truncation present). Full flat and stripe matrices were then captured with the checked-in simctl driver directly. Screenshots are actual simulator framebuffer captures, not `drawHierarchy` images. `verification.json` records checked-in file, source and native executable hashes.

## Applied size profiles

`sizes/{dark,light}-{0,quarter,half,threequarter,1}.json.gz` contain ten fresh-process captures. Each has 42 attached public `UIVisualEffectView` instances: regular, clear and system-blue tinted, for every combination of:

- Wide capsule: 280pt × height 44, 52, 62, 64, 80, 120 or 160pt.
- Circle: width and height both 44, 52, 62, 64, 80, 120 or 160pt.

Each radius is half the shortest dimension. There are no transforms. Views overlap to fit the configuration probe; some extend beyond the visible screen. These are applied configuration captures, not claims that every control was visibly composited in isolation. UIKit places material layers outside the `UIVisualEffectView.layer` subtree, so the window layer tree is captured instead. The decoder associates each `_UIMultiLayer` with its requested surface in order and asserts exact material bounds, 42 actual `glassBackground` filters per launch and 70 inputs per filter.

`size-profiles.json` contains 420 normalized profiles, preserving complete material subtrees, SDF effects, backdrop scale and archived vibrant matrices. All 70 inputs match between the wide and circular shape at every corresponding measured height, appearance, preference and style. This does not imply that SDF geometry is identical.

For example, dark regular glass at the half preference resolves `inputFaceColorMatrixMaxLumaSDR` to approximately .6 at 44–64pt, .5 at 80pt, and .35 at 120–160pt. Using the original 80pt profile for every navigation item would therefore give a different face response. Exact behavior between the measured sizes remains an interpolation decision in the portable adapter. The added quarter/threequarter captures establish that dark fill RGB coefficients interpolate even when alpha is zero: preference .25 resolves gray .0625 with alpha zero; preference .75 resolves gray .0625 with alpha .25. Premultiplying the profile coefficients before interpolation would erase this native response.

## Complete four-style profiles and native gradient curves

The final, preferred dataset is `all-styles/{dark,light}-{0,quarter,half,threequarter,1}.json.gz`, normalized in `all-styles-profiles.json`: **560 profiles** covering all 14 geometries, both appearances, all five tint preferences, and regular/clear/regular+tint/clear+tint styles. The last variant is labeled `clearTinted`. Every launch asserts 56 actual filters. `clearTinted` has the same 70 backdrop inputs as clear in all measured contexts, while preserving its own accent/SDF layer pipeline.

REA replay **ev_f9b7568d80e696e5ffdc37ec5bbb6376c6cd975937f387e7da597abca8e35b99**, saved in `rea-all-styles-evidence.json`, exited zero with ten fresh run signals. Its overall truncation flag is true because the before-checkpoint directory did not yet exist; the after-checkpoint is complete, records all ten generated files, and is not truncated. The earlier 420-profile size dataset remains available for independent comparison.

`Value` now reads `CAMediaTimingFunction.getControlPointAtIndex:values:` directly. `controlPoints` stores the two inner Bézier points as `[x1,y1,x2,y2]`, with endpoints (0,0) and (1,1). Across the complete captured matrix, the two accent gradient interpolation curves are:

- Linear: `[0,0,1,1]`.
- Eased: `[0.6093800067901611,0.006630000192672014,0.47124001383781433,0.9911500215530396]`.

These are observed curve coefficients, not a guessed easing function.

## Explicit primary accent color transfer

`accent-colors/*.json.gz` and `accent-colors-profiles.json` contain four fresh captures: explicit red and explicit green, each in light and dark appearance, at preference half and 280×80pt. The actual effect trait collection resolves system blue to `(0,145/255,1)` in dark and `(0,136/255,1)` in light. Querying a dynamic color outside that trait collection can therefore give the wrong reference green component.

The backdrop-aware vibrant matrix for explicit red or green agrees, within approximately 4e-7 coefficient noise, with per-channel endpoint interpolation of the captured blue matrix:

```
newRow[channel] = mix(blueMatrix.rowR, blueMatrix.rowB, tint[channel])
```

The interpolation includes each row's bias. Dark blue's zero-color endpoint has gain approximately .2041667 times Rec.709 luminance and bias −.030625; its full-color endpoint has gain −.0375 and bias 1.005625. Explicit red selects the latter in the R row and the former in G/B; explicit green selects the latter in G. Light red/green select the full-color row `[.08504,.28608,.02888,0,.6]` only in their corresponding channel; zero-color rows are zero. This directly verifies endpoint selection for these primaries only. It does not establish a universal linear tint conversion; later nonprimary captures below disprove that generalization.

## Face transfer perturbations

The flat fixture uses eight separate 280×80pt glasses over full-width uniform 80pt background bands: gray 0, .25, .5, .75, 1, red, green and blue. Each band is separated by 15pt of black. Center samples are 40pt from horizontal material boundaries and 140pt from rounded sidecaps. Large-radius perturbations can reach adjacent bands; they are not used to infer flat face transfer.

The first attempt mutated the already attached CAFilter instance. KVC readback succeeded, but all framebuffer samples were unchanged, even for saturation zero and face opacity zero. Those failed visual experiments are retained under `object-mutation-dark`; readback alone is not evidence of rendering behavior. The working fixture copies the filter, changes the fresh copy, then assigns a new filters array. The `replacement-dark` matrix contains 20 visually active perturbations. Raw PNG component samples are in `replacement-dark-centers.json`.

Use raw bitmap components, not `NSColor`'s additional calibrated-color-space conversion. The screenshot is tagged sRGB; the raw background controls are exactly [0,64,128,191,255]. The checked-in Swift pixel helper uses `getPixel` accordingly.

Observed gray center values (8-bit, dark regular, preference half):

| Mutation | black | gray .25 | gray .5 | gray .75 | white |
|---|---:|---:|---:|---:|---:|
| Baseline |32|86|125|150|159|
| Face black endpoint 0 |4|61|105|133|143|
| Face white endpoint 1 |32|79|114|135|143|
| MaxLuma and MaxLumaSDR .2 |32|81|107|109|88|
| Face identity |4|62|124|186|247|
| Face opacity 0 |4|62|124|186|247|
| Fill black alpha .5 |18|44|63|76|81|
| Fill white alpha .5 |143|171|190|203|207|

These perturbations support a quadratic luminance compression followed by independent luminance endpoint transfer and chroma saturation:

```
q = c * (1 + (maxLumaSDR - 1) * luma(c))
qLuma = luma(q)
face = black + (white - black) * qLuma + saturation * (q - qLuma)
```

For a gray input, `q = x*(1+(maxLumaSDR-1)*x)` and the chroma term is zero. This explains the non-monotonic .2 case and is inconsistent with interpreting MaxLuma as a hard clamp or a rational shoulder. For example, the .5 profile maps ideal white to .125 + .5 = .625, approximately 159/255. Residual differences of a few bytes reflect the remaining glass composition and sampling. This is a bounded reconstruction supported by controlled perturbations, not extracted proprietary shader source.

Endpoint gain applies only to luminance, not to the entire saturated RGB vector. The independent white-endpoint perturbation distinguishes them: reducing white from1.125 to1 changes red G/B16→10, green G213→198, blue R26→24. Predicted luminance-only changes for those primary channels are −6.05/−14.64/−2.22 component values, matching the observed −6/−15/−2. Applying endpoint gain to saturated RGB would instead increase the red background G/B channels, the wrong direction.

The profile's saturation affects colors while preserving grays. Saturation zero changes red from [255,16,16] to [84,77,77], green [0,213,0] to [147,147,147], and blue [26,27,255] to [48,48,60]. Remaining channel differences show that the final pixel includes other contributions. A luminance-preserving saturation operation commutes with the scalar luminance compression, so these samples do not uniquely establish their internal ordering.

The separate `max-luma-dark`/`max-luma-light` tests change MaxLuma and MaxLumaSDR independently. SDR framebuffer output follows MaxLumaSDR. Changing only MaxLuma has no effect in these SDR captures. HDR/headroom behavior has not been measured.

## Blur and fill blending

Changing normal, lighten or darken fill opacity from zero to one leaves uniform centers within approximately one component value. Thus treating these inputs as black or white tint opacities would be unsupported. Their names and spatial response are consistent with mixing a second blurred source using normal/lighten/darken operations.

`blur-matrix` uses 4pt black / 4pt white vertical stripes. Rows 0–2 change main radius to 2, 5, 10; rows 3–5 set main radius zero and fill radius 2, 5, 10; row 6 sets main radius zero; row 7 sets main radius 20. The process-local tint preferences are 0, .5 and 1. Each capture preserves the original filter archive and scale for every row. Raw horizontal samples, mean, standard deviation and extrema are in `blur-contrast.json`.

Main radii 2/5/10 at preference zero produce stripe standard deviations about 42.0/30.7/8.9 versus 53.3 for main radius zero. At preference one, the additional fill blur dominates and main radius changes leave the center nearly flat. Native backdrop scale changes from .5 at preferences 0/half to .125 at preference one. These establish that both blur stages and native sampling scale matter; they do not establish a universal conversion from native radius to Skia sigma. Blur opacity distance interpolation, exact native kernel, downsampling filter, refraction pass ordering, HDR behavior and all contextual UIKit material variants remain reconstruction details rather than proven private equations.

## Final isolated main-blur kernel experiment

`KernelProbe.m` and `capture-kernel.py` isolate the main filter by using a fresh filter copy with face endpoints 0/1, saturation 1, both face luminance coefficients 1, transparent face fill, face opacity 1, all four blur opacities 1, all three blur-fill opacities 0, inner/outer refraction amounts and refraction opacity 0, bleed/shadow/ring opacity 0 and key-fill amount 0. Four square-wave periods 4/8/16/32pt repeat across eight separate 280×80pt controls. Radius 0/1/2/5 is captured at preference half (backdrop scale .5) and one (backdrop scale .125), eight fresh processes total.

Raw framebuffer captures and mutation readbacks are under `isolated-kernel`. `scanlines.json` preserves 384 raw component samples across the center 128pt of each surface. `analyze-kernel.py` measures each square wave's fundamental and compares its amplitude against the same preference's radius-zero frame. The Gaussian-equivalent added sigma is `sqrt(-2*log(amplitudeRatio))/angularFrequency`, expressed in view points. It is a frequency-response estimate, not the private kernel's literal parameter.

| Preference | Native radius | Equivalent added sigma at period 16pt | Equivalent added sigma at period 32pt |
|---|---:|---:|---:|
| half |1|2.560|2.605|
| half |2|3.630|3.986|
| half |5|7.508|9.026|
| one |1|2.362|3.042|
| one |2|3.571|4.528|
| one |5|7.441|9.955|

The differing estimates across spatial frequencies establish that a single exact Gaussian sigma does not reproduce this native kernel. Around the default radius 5, using native radius directly as Skia sigma is substantially weaker than native low-frequency attenuation. Approximately twice the radius is a better low-frequency approximation for these measured configurations; it is not a proven universal conversion law. Scaling sigma by `1/backdropScale` is contradicted by the data: preference-one radius5 has approximately 10pt low-frequency sigma rather than 40pt.

Radius zero also exposes the native capture-resolution response. At period4pt, its fundamental amplitude falls from 153.16 at preference half to 12.25 at preference one, while period32pt remains 162.19 versus 160.45. The captured scale .125 therefore changes sampling even without requested blur. Main blur and baseline downsampling must be treated separately. Radius5 controls remain near mean127 in all stripe rows, confirming that this experiment does not introduce the default tone or fill density.

REA replay **ev_d304cd6ab8ba57c29038322512790a81e6642ecb8b521d0ece3adf1982a4a6a4**, saved in `rea-kernel-evidence.json`, exited zero with all eight fresh-frame signals. Its after-checkpoint lists all16 files, but the aggregate file hashing budget was exhausted, so the REA filesystem evidence is truncated and later hashes are null. `verification.json` independently hashes every frozen local capture and analysis file. Exact native kernel taps, resampling reconstruction filter and behavior at other display scales remain unknown. The owned simulator was shut down after the experiment.

## Nonprimary tint conversion and production palette

Seven additional fresh-process captures (`tint-curves`, normalized in `tint-curves-profiles.json`) establish that native arbitrary tint conversion is nonlinear and includes different chromatic/achromatic matrix structures. For example, light coral `(1,69/255,58/255)` has RGB-row gains `[.223391,.318339,.267589]` and biases `[.776609,−.047751,−.040138]`; interpolating the primary-color endpoint matrices would produce different values. Gray .5 has diagonal RGB coefficients .30864197 and biases .33950612, preserving background chroma rather than collapsing it into a single luminance input. A universal HSV, RGB gamma or endpoint formula has not been recovered.

The bounded production solution therefore uses a measured palette. `PaletteProbe.m` captures **48 literal/semantic colors in both regular and clear glass, in both appearances: 192 matrices**, from two fresh processes at 280×80pt and preference half. This includes the exact historical comparison colors `(0,122,255)` and `(255,69,58)`, saturated primaries, achromatic anchors, the existing optical palette, all UIKit system accent colors and label/gray variants. Alpha is forced to1 for matrix resolution and remains a separate composition parameter in NXKit. Resolved RGB, original semantic name and complete filter options are preserved in `accent-palette.json` and the raw `accent-palette/{dark,light}.json.gz` captures.

REA evidence **ev_2674887c2971946e9d0d14c01bff67ac1cf054667d8a6dc801d46d06416d6047**, preserved in `rea-palette-evidence.json`, exited zero, records both fresh 96-material processes, and is not truncated. No same-RGB literal/semantic ambiguity appears in the captured matrices, within floating-point noise. Light regular/clear matrices match for every palette color; dark regular/clear matrices differ for42 of48 colors. The earlier complete system-blue matrix is invariant across all seven measured sizes and five preferences within each appearance/style. Applying the broader palette across those other size/preference contexts is an NXKit policy based on that observed pass separation, not a new measurement of every color cross-product.

`generate-accent.py` deterministically generates `UIKit/lib/NativeGlassAccentData.hpp` from both raw captures, asserting native runtime/build, geometry, fresh filter count, unambiguous RGB samples, and full 4×5 matrices. `--check` verifies both generated code and normalized evidence against the original archives. `NativeGlassAccent.hpp` selects the captured style/appearance variant using its unchanged system-blue reference matrix; exact sampled RGB values retain all native coefficients and options.

Other colors use an explicitly approximate continuous inverse-distance interpolation in RGB/HSV space, including the native achromatic samples. Hue and saturation distances vanish with chroma/value to prevent singularities near gray or black. The interpolation converges to every captured anchor and keeps all four matrix rows, including diagonal achromatic behavior. It is not presented as UIKit's unrecovered color-conversion algorithm. Alpha and HDR behavior remain separate concerns.

`UIKit/tests/NativeGlassAccentTests.cpp` verifies all192 captured selections, convergence approaching every anchor, finite achromatic results and a safe nonfinite-input fallback. `NativeGlassAccentTests` and the generator's `NativeGlassAccentData` check are registered in the normal UIKit test configuration and the headless raster harness. The standalone compiled lookup test passed; the main implementation task owns final shared-build and pixel-regression runs.

## Native CPU tint conversion supersedes palette interpolation

The earlier arbitrary-color interpolation limitation is resolved by shipped
DesignLibrary CPU inspection. The native HSL endpoint and achromatic
normal/dodge/burn equations match all192 original matrices and32 independent
held-out matrices within `6e-7` per coefficient. Production no longer samples
the RGB/HSV palette. See [derived equations and replay](../color-transfer/README.md)
and [image hashes/instruction locations](../color-transfer/native-cpu-accent.json).
