# Spatial native lighting isolation

The native target is UIKit on iOS27.0 (24A434), iPhone17Pro simulator at3×,
built with the installed iPhoneSimulator27.1 SDK. The process-local glass tint
preference is0.5. Nine independent regular-glass circles cover44,80,120pt on
black, half gray and white. This isolates spatial passes; it is separate from
flat-color and texture-center comparisons. No NXKit renderer creates native
fixtures, and no physical-device preference is changed.

## Observed transfer

The CASDF highlight vibrantColorMatrix is active even though its backdropAware
input is absent. Applying that matrix to the white light color would clamp
almost entirely to white and cannot explain the native screenshots.

For dark44pt glass, the captured matrix maps grayB to
`M(B)=.9118*B+.1471*255`. The observed result agrees with
`C=B+response*(M(B)-B)`. The spatial response can exceed one:

| Inside depth,pt | Black-derived response | Gray-derived response | White-derived response |
|---|---:|---:|---:|
|.1667|1.586|1.596|1.604|
|.5|1.182|1.168|1.175|
|1.1667|.317|.311|.329|
|2.1667|.173|.194|.188|

At.5pt depth, no-highlight/normal/no-matrix pixels are32/73/255 over black,
134/164/255 over gray, and184/209/255 over white. This establishes a vibrant
backdrop contribution rather than clamped white paint opacity. Byte
quantization and source-field details explain the small response spread.
The framebuffer observation alone does not establish the kernel. Subsequent
inspection of the shipped AIR independently explains this response.

In the dark subset, disabling the filter's internal highlight produces no
sampled change in the-3..0pt rim band. Removing the ring changes at most4RGB
levels there. Bleed is absent at44pt and changes at most3 levels near the120pt
rim. Removing elevation leaves interior pixels unchanged. These negative
observations apply only to the sampled profiles, backgrounds and points.

## AIR-derived portable equations

The shipped QuartzCore `default.metallib` on the installed iOS27.0 simulator
contains readable AIR. The portable implementation independently translates
its equations; no Apple IR, shader payload or binary is stored in this repo.
The parent artifact manifest records the original binary hash. Relevant
functions are `sdf_diffuse_kf_highlight`, `sdf_key_fill_highlight`,
`vibrant_color_matrix_sover`, and `glass_background_*_lpf`.

The external SDF highlight uses a curvature blend between a step and a linear
band, multiplied by two antialiasing gates. Its angular response is linear in
normal/light cosine above `cos(spread)`. Amount is a Schlick bias parameter:

```
t = saturate(depth / height)
band = mix(t < 1 ? 1 : 0, 1 - t, curvature) * entryAA * exitAA
angular = saturate((dot(normal, light) - cos(spread)) / (1 - cos(spread)))
bias(v, amount) = v / (1 + (1 / amount - 2) * (1 - v))
```

Key and fill each add a diffuse lobe with curvature1, scaled height/spread,
and a scaled bias amount. Their sum can exceed1. The attached vibrant matrix
transforms the destination color, not the white light color; source RGB is
ignored and source alpha carries the lighting response. The previous empirical
crest/tail fit is superseded. `fit-lighting.py` and `sdf-fit.json` are retained
only as historical evidence and do not configure production.

`verify-air-lighting.py` performs no fitting. On2,836 registered native points
more than.25pt inside, its mean absolute error is.1474/255 and maximum1.376/255.
The production SkSL byte comparison has mean.00247/255 and maximum1/255
over those2,836 points; including the antialiasing neighborhood gives
mean.00416/255 and maximum1/255 (`kernel-comparison.json`).
These comparisons isolate SDF lighting using native unlit pixels as inputs;
they do not establish whole-image glass equivalence or refraction fidelity.

The bleed source displacement is spherical:
`amount * (1 - sqrt(t * (2 - t)))`, where `t=saturate(-sdf/height)`.
Height bounds displacement, not the compositing domain. The blend weight is
`opacity * edge² * tone⁴`, with a linear SDF edge mask and an affine luminance
tone. The source passes through the captured affine bleed matrix; it is not
clipped using a darken/minimum blend. The host mapping of the darken boolean
was recovered from the CPU uniform stores: true selects `L`, false selects
`1-L`. The fourth-power kernel and matrix/compositing order come from AIR.

Entire-frame gray controls independently prove large-material center bleed:
the120pt center is60pt inside while bleedHeight is42pt. Native light centers
at44/80/120pt are195/197/200; disabling bleed gives195 throughout. Dark centers
are134/127/117 versus134/128/118. Three vertical positions reproduce them.
Six native uniform controls retain tolerance2; eighteen independent native
SDF controls retain tolerance4. Synthetic tests separately cover changed
parameter semantics rather than changing native fixture bytes or tolerance.

Shadow amount displaces the independently filtered exterior sample; it does
not darken the material interior. Shadow coverage and the one-sided ring use
the shipped bounded half-precision erf polynomial, at their respective radius
conventions. Shifted contours are evaluated at shifted coordinates. Ring
masking uses the body coverage once. The internal filter microcontour uses
its own outside band and cubic RGB adjustment after body/shadow/ring coverage,
followed by premultiplied SDR holding tone. Internal extended-range clamping
preserves negative components; the physical SDR attachment clamp is separate.

The AIR alpha path is independent of backdrop opacity: interior source RGB is
unpremultiplied and starts with material alpha1. Outer refraction mixes raw
premultiplied source RGBA into this state. FACE then operates directly on that
mixed RGB without another unpremultiplication, and preserves its alpha. Bleed
weight does not multiply source alpha. Shadow and ring coverage use the contour
field alpha; transparent backdrop texture alpha affects their sampled colors,
not geometric coverage. Focused synthetic tests exercise these shader laws;
the native framebuffer controls here remain opaque.

The default opaque SDR path is the captured target. CPU uniform stores also
establish `highlight_extension=(effectOffset<0)` and
`wants_highlight_bias_correction=(colorBias!=0)`. All560 captured profiles
enable both flags. The outside stroke mode is also CPU-derived:
`effectOffset<0 && abs(height+effectOffset)<.001`. It bypasses the curvature
lobe and instead multiplies outside body coverage by the offset edge AA gate;
all captured profiles select it. The internal direction packs
`(sin(angle),-cos(angle))`, matching the native normal basis.
Nondefault transparent/HDR behavior and arbitrary
coefficient mutations need additional runtime coverage. The renderer's source pyramid and texture filtering are
verified separately from these scalar equations.

## Evidence and reproduction

There are17 valid screenshots:13 spatial pass-isolation cases and four uniform
gray controls. `verification.json` lists hashes and coverage. `LightingProbe.m`
copies each glassBackground CAFilter before changing inputs and replacing the
filters array. The highlight knockout changes only the corresponding CASDF
layer's opacity; the matrix knockout removes only that layer's filters. Each
accepted JSON acknowledgement contains9 filter and9 highlight records, fresh
runID, requested changes, readback, geometry, scale and OS. Screenshots use the
simulator framebuffer. Analysis reads raw PNG components, avoiding NSColor's
additional color conversion.

```
python3 Demo/GlassComparison/NativePassProbe/build.py OUTPUT --source Demo/GlassComparison/NativePassProbe/LightingProbe.m
xcrun simctl install DEVICE OUTPUT/Probe.app
python3 Demo/GlassComparison/NativePassProbe/capture-lighting.py --device DEVICE --output REPORT
swift -module-cache-path /private/tmp/nxkit-lighting-swift Demo/GlassComparison/NativePassProbe/lighting-pixels.swift REPORT > REPORT/samples.json
python3 Demo/GlassComparison/NativePassProbe/analyze-lighting.py REPORT
```

The default driver offers11 knockouts per theme. `--only` limits the cases;
`--dark-only` and `--light-only` select an appearance. `--uniform .5` makes the
whole viewport gray. `LightingKernelCompare.cpp` evaluates the production
helper for registered native knockout pairs, and `compare-lighting-kernel.py`
reports full and antialiasing-excluded errors. Run
`verify-air-lighting.py REPORT PROFILES_JSON` for the independent AIR equation
check. `fit-lighting.py` reproduces the superseded bounded calibration. This checked-in run is a finite subset rather
than a claim that every driver combination was captured.

## Excluded attempts and limitations

`invalid-sdf/` retains six effect-coefficient attempts. KVC readback succeeded,
but copying and reassigning the CASDF effect produced full white output even
when key, fill and diffuse amounts were zero. These screenshots do not
establish coefficient semantics and never enter calibration or metrics.

`invalid-uniform/` retains two unpaired screenshots from the initial uniform
run. A decimal point in output labels caused Path.with_suffix to overwrite
baseline with no-bleed. Labels were sanitized and all four controls recaptured
with distinct fresh runIDs. A launch while the shared bundle contained the
parent's blur fixture failed the nine-frame schema check and produced no
accepted artifact.

The current fixture and executable hashes describe the final uniform replay.
The executable was rebuilt between earlier pass captures and that replay;
earlier executable hashes were not retained. Source, runIDs, readbacks and raw
screenshots remain available, but the manifest does not invent binary hashes
for earlier launches. The framebuffer evidence establishes these finite results. AIR-derived
equations add shipped-artifact evidence; neither establishes physical iOS27.2
whole-image equivalence.
