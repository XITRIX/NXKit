# Native UISlider endpoint reference

The bounded native fixture uses iOS 27.0 (24A434), iPhone Simulator at 3×.
`native/snapped` contains 26 stable pages across both themes; `native/continuous`
contains five additional light-mode configured pages with allowsTickValuesOnly
explicitly disabled. Each case retains complete model/presentation layer and
mask geometry, the actual OS screenshot, and SHA-256 provenance. The compiled
Objective-C source and capture manifest are retained with each group. No native
app binaries are included.

The 22 standard input fractions concentrate on the first and last two percent.
Neutral-center pages also include .49, .499, .4999, .5, .5001, .501 and .51.
Cases cover default, thumbless, custom thumb, ticks, neutral .5, endpoint images,
disabled, width 180, range −10…10, and custom min/max track images. The screenshot
pages stay unchanged until capture; they use Simulator OS screenshots rather
than drawViewHierarchy. Their PNGs report sRGB metadata and are 1320×2868.

## Observed spatial law

The native fluid track follows the statically recovered mapping:

```
C(u) = T * clamp(min(u/.02, (1-u)/.02), 0, 1)
E(u) = C(u)/2 + u*(W-C(u))
```

`W` is the actual track width, `T` is the resting thumb width (37pt normally,
30pt for the custom image, zero for thumbless), and `u` is its normalized visual
fraction. Thus width begins at exactly zero, changes quadratically in the first
2%, is linear inside the interval, and reaches full width smoothly. The filled
view's opacity stays 1; this is spatial geometry, not a fade at the endpoint.
The thumbless rendering uses a 7pt track and `E(u)=W*u`, although its public
trackRectForBounds still reports a 6pt rect and public thumbRect reports 37pt.

For W=300, T=37, native filled widths at fractions .0001, .001, .005, .01 and
.02 are approximately .1224815, 1.2231501, 6.0787499, 12.0649997 and 23.7599995pt.
Endpoint images reduce the track to x=29,width=234; the same mapping applies to
that live width. Width 180, disabled and both themes preserve the geometry.

Custom min/max track images select the separate native `_UISlideriOSVisualElement`
provider, with a 31×31 thumb and its own insets/integer split. That provider does
not obey the LiquidLens 2% law. Its source geometry is retained as an explicit
exception, not evidence that the fluid mapper covers legacy track images.

## Configured ticks and neutral

The public header defaults allowsTickValuesOnly to YES. With that default, the
getter and rendered fill snap to the five configured tick positions. With it
explicitly NO, the fill follows the requested continuous fraction, while the
native public value getter still reports the nearest tick in these captures.
This getter/visual distinction must be preserved in interpreting the dataset:
using the reported value to predict the continuous visual fraction is wrong.

Neutral .5 introduces no bipolar fill or separate neutral anchor in the native
hierarchy. All captured blue fills start at local x=0, including around .5 and
with a −10…10 range. The neutral metadata's broader application semantics are
not established by these endpoint captures.

`ordering-summary.json` compares fresh rows against immediate forward/reverse
setValue sweeps. All 26 original pages agree exactly. The five continuous
configured pages reveal a setter/getter quirk: after requesting .9999 (getter
already 1), setting 1 leaves the prior 299.877498pt visual width; setting 0 after
.0001 similarly leaves .1224815pt. Around neutral, .4999→.5 retains 149.973703pt.
The maximum fresh/ordered difference is .122502pt. This is bounded evidence of
native configured-state behavior, not an additional temporal endpoint easing.

## Real touch and overdrag

`native/drag` retains one actual continuous OS touch sweep per theme, with
passive native video, raw touch/action/layer traces, per-frame `track.csv`, ten
selected crops and a [contact sheet](native/drag/contact-sheet.png). XCTest's
event-path method signature was checked at runtime. The delivered streams each
contain one down/up and 233–240 moves, span screen x=35…425, and reach value 0
and 1. No private lens state was authored.

During the moving interior, the fill continues to use resting T=37 even though
the raised lens bounds are 57.35pt wide; the observed mapping residual against
the public float value is at most 3.3e−5pt. Overdrag additionally deforms the
native track container. Beyond maximum its width reaches 303.75031pt and height
4.28753pt, with local x=2.08065; the filled view spans that live width at value 1.
Returning inside restores the track. At the lower endpoint the filled width
stays zero. This separate elastic track deformation is recorded rather than
claimed as implemented by the endpoint-mapping fix.

Video crops are compressed Display-P3 video explicitly converted to sRGB;
`selected-frames.json` preserves video frame time, native marker ID and actual
trace timestamp. Marker commit and presentation sampling can differ by about
one compositor frame. These crops are visual motion evidence, not lossless
pixel-error ground truth. The static PNG screenshots remain independent,
lossless OS captures. The real-touch sweep is runtime observation; the analytic
law is corroborated by the retained geometry and the separate static REA audit.

## Reproduction

```
/usr/bin/python3 Demo/SliderComparison/capture-endpoints.py DEVICE OUTPUT
/usr/bin/python3 Demo/SliderComparison/capture-endpoints.py DEVICE OUTPUT --continuous-ticks
```

`EndpointReference.m` implements the pages. The endpoint drag cases reuse the
actual-touch `LiftReference.m`/`LiftUITests.m` fixture; exact compiled snapshots
are retained under `native/drag`. No device installation, persistent system
setting change, or further lens/shader investigation belongs to this probe.
