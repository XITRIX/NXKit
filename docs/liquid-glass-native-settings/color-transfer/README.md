# Additional native color-transfer experiments — 2026-11-05

These observations use the iOS 27.0/24A434 simulator at 3×, fresh processes and
process-local tint preference `.5`. The research executables link Apple UIKit
and contain no NXKit renderer. Raw layer/filter archives, framebuffer PNGs,
center component samples and SHA256 provenance accompany this report. The
previous 192-matrix production palette remains unchanged. No failed comparison
reference was relaxed or regenerated from NXKit output.

## Neutral body: observed

`NeutralColorProbe.m` places 52pt and 120pt circles side by side on six 145pt
uniform color bands: yellow `(255,255,0)`, brown `(200,160,128)`, green,
red, gray128 and white. Center positions are `(58,92.5+145*row)` and
`(300,92.5+145*row)` in points. PNG components are read directly with
`NSBitmapImageRep.getPixel`; there is no AppKit color-space conversion. Bands
have finite extent: a large bleed kernel can sample neighboring bands. These
measurements establish the attached scene, not an infinitely uniform field.

Changing only the native bleed opacity to zero produces these 120pt centers:

| Theme / source | Baseline | Bleed opacity zero |
| --- | --- | --- |
| Light yellow | 255,255,20 | 255,255,27 |
| Light brown | 250,212,179 | 245,207,177 |
| Light gray128 | 201,200,200 | 195,195,195 |
| Dark green | 0,186,0 | 0,187,0 |
| Dark brown | 155,121,94 | 156,122,95 |

In light appearance, the bleed-disabled 120pt centers equal the 52pt centers.
The captured light 120pt filter retains bleed height42, bleed blur radius42 and
opacity `.2916667`, although the circle center is 60pt from its edge. This is
runtime evidence against treating bleed height as a hard spatial cutoff. The
lighting investigation uses these samples to check its separate reconstruction.

The dark face equation still has an independent chromatic residual. A native
120pt green center with bleed disabled is187, versus180 in the prior portable
face equation. The native face coefficients remain black `.125`, white1.125,
saturation1.3 and maxLumaSDR `.35`; this is not a changed profile setting.

Additional fresh filters disable bleed, ring/shadow opacities, shadow amount and
internal filter-highlight amount, then set saturation0 or saturation0 plus
white `.5`. Green120 becomes129 and68 respectively. Their difference gives
`qLuma ≈ (129-68)/(255*.625) = .382745`, agreeing within quantization with the
existing quadratic luminance model. It does not establish the corresponding
chroma equation.

A proposed chroma correction based on the face-black endpoint was explicitly
rejected using two more independent native processes. At face black0, green120
is167 while the candidate remains160. At black `.25`, native green120 is207
while the candidate overcorrects to215. A hybrid BT.601 chroma-compression
hypothesis improves green but produces a 13-byte error on the yellow control
at black `.25`. Neither candidate is shipped as a recovered native equation.
A final candidate `chromaGain = saturation * pow(F,1/saturation)` with the
existing luminance factor F matched36 default/black-endpoint controls to at
most2/255, but failed independent saturation1/2 observations. At saturation1,
native green120 is `(18,174,18)` versus candidate `(32,168,32)`; at saturation2,
native green120 is218 versus candidate236. This candidate is also rejected.

The saturation1/2 captures instead establish an approximately linear chromatic
response, with a compression factor different from the current portable F.
For brown120, both saturation settings independently produce the identical
factor `.6527899`, versus current `.576369`. The derived multipliers use the
saturation0 intercept and only unclamped output components, rather than fitting
a formula across colors or sizes:

| Source luminance | Native factor52 /120 | Current factor52 /120 |
| --- | --- | --- |
| Red .2126 | .922288 / .903842 | .914960 / .861810 |
| Brown .651740 | .800181 / .652790 | .739304 / .576369 |
| Green .7152 | .772979 / .610191 | .713920 / .535120 |
| Yellow .9278 | .700653 / .468430 | .628880 / .396930 |

`analyze-native-face.py` reproduces these observations and quantization limits;
`native-face-slopes.json` retains the individual saturation1/2 multipliers and
which unclamped components were used. Gray inputs cannot reveal a chromatic
slope. `centers.json` preserves all observations, including rejected candidate
input cases. The initial runtime experiments alone did not establish the chroma-compression
function; all rejected fitting laws were kept out of production. The subsequent
read-only shipped-artifact inspection below resolves this specific gap.

## Native AIR resolves the face chroma equation

Apple's installed Metal Toolchain can print readable AIR/LLVM from the
simulator's shipped `QuartzCore.framework/default.metallib` using
`metal-objdump --metallib --disassemble`. This is static shipped shader
intermediate representation, not original Metal source and not a shader inferred
from screenshots. REA's direct native loader does not accept the MTLB format;
the Apple tool is the successful inspection path.

The source library SHA256 is
`5d59e4e26569b90f3e7e6331be5cf8931aab96718306ee62a72862e4e5192073`.
The `glass_background_minimal_lpf` module's `glass_background_base` function
contains the missing chroma compensation immediately before its face3×4 matrix:

```text
L = dot(sourceRGB, Rec709)
F = saturate(1 - L*(1 - maxLuma))
compressedLuma = F*L
compressedRGB = F*sourceRGB
compensatedRGB = mix(compressedLuma, compressedRGB, 1 + 0.3*(1-F))
result = faceColorMatrix(compensatedRGB)
```

The native literal is float `0.30000001192`. Equivalently, the face chroma gain is
`saturation*F*(1+0.3*(1-F))`; its saturation dependence is linear, matching the
independent saturation1/2 observations. The ordinary matrix's endpoint gain
still affects luminance independently of its saturation gain. This identifies
why previous formulas reproduced grays but underrepresented saturated green.

A fresh read-only Apple-tool replay is recorded as REA Evidence
`ev_4d50e9f5ba8bb8613dbc0cbf457cc19d62609bdce1d2976d45323b9670a10b0b`:
exit0, untruncated. [Machine-readable provenance](native-air-face.json) records
its artifact/module hashes and limitations. Applying the derived arithmetic to
all252 independent RGB components from seven native endpoint/saturation cases,
two sizes and six backgrounds gives **mean error0.003968/255, maximum1/255**.
The default, saturation0/1/2, white `.5` and black `.25` cases match every
rounded component exactly; one black0 component differs by1/255.

The Apple library and complete disassembly remain outside the repository.
Only hashes, function identity, derived equations and independently captured
controls are retained. CPU preparation is independently resolved below.

## Arbitrary accent colors: native CPU equations

`AccentHoldoutProbe.m` captures eight colors excluded from the original palette:
`(61,119,213)`, `(213,97,61)`, `(100,180,140)`, `(110,111,110)`, gray70,
gray204, `(127,128,127)` and `(20,80,150)`. Both regular/clear styles and
appearances produce32 independent full matrices.

The shipped DesignLibrary CPU image contains the actual conversion at
`0x1cad58ac8`. Apple’s cache extractor omits its Swift exports, so the original
cache export trie was parsed separately and 1,404 import stubs resolved to
SwiftUICore functions. The conversion uses **HSL**, with an achromatic RGB-range
threshold of `0.0003452669770922512`. Native converts adjusted HSL back to RGB
with saturation allowed above1, then clamps the generated RGB components.

For dark regular glass:

```text
adjusted = clampRGB(HSL(hue, .95*saturation, 1.1*lightness))
slope = (adjusted - originalRGB) / .6
bias = originalRGB - .15*slope
```

For light regular and both clear appearances:

```text
adjusted = clampRGB(HSL(hue, 1.2*saturation, .6*lightness))
slope = (originalRGB - adjusted) / .85
bias = originalRGB - slope
```

Each chromatic matrix row multiplies its slope by the Rec709 luminance weights.
For achromatic colors, native composes a normal gray fill at alpha `.75`, then
dodge with `.8+.2*gray`, then burn with `.2*gray`. The resulting diagonal matrix
has `gain=.25/denominator` and `bias=(.95*gray-.2)/denominator`, where
`denominator=(.8+.2*gray)*(1-.2*gray)`.

These independently translated equations match **all192 original matrices and
all32 withheld matrices**, including semantic and near-gray colors. Across
4,480 coefficients the maximum error is `5.99924e-7`. The portable C++ renderer
adapter also compares every held-out coefficient within `1e-6`; its seven-gray
backdrop projections have mean error `0.00000769/255`, maximum
`0.0000304/255`. The former RGB/HSV palette interpolation (maximum28.2963/255)
has been removed from production. The captured palette remains test evidence;
only four small reference profiles are used to identify the resolved recipe.

[CPU provenance and instruction locations](native-cpu-accent.json) records
image hashes, equations and the replay. The Apple binary and full disassembly
remain in scratch storage, outside the repository. Matrix captures cover
resolved sRGB colors at the half preference; HDR/out-of-gamut colors, alternative
tuning recipes, accessibility variants and translucent tint alpha remain outside
this specific verification.

## Reproduction

Use a simulator dedicated to this research; the capture script replaces its
probe bundle between the neutral and accent fixtures. It changes no persisted
system preference.

```sh
python3 Demo/GlassComparison/NativePassProbe/capture-color-transfer.py \
  --device SIMULATOR_UDID --output /private/tmp/nxkit-color-transfer-replay
python3 Demo/GlassComparison/NativePassProbe/generate-accent-holdouts.py --check
python3 Demo/GlassComparison/NativePassProbe/analyze-native-face.py --check
python3 Demo/GlassComparison/NativePassProbe/verify-native-accent-equations.py
```

Earlier Objective-C setter-hook attempts produced no callbacks for native glass
construction. Those negative results did not establish an absent conversion;
the subsequent shipped AIR and DesignLibrary CPU inspection resolved the face
and accent equations without further application launches.
