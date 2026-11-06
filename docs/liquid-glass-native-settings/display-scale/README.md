# Native glass display-scale recipe

The shipped AIR equations exposed two missing host inputs: the packed direction
is `(sin(angle),-cos(angle))`, and stroke mode is selected when the negative
highlight offset cancels its height to within.001. After translating these,
the original2× native side-rim controls still differed by approximately16RGB
levels. This report records the subsequent native experiments; no equation or
image alignment was fitted to make those controls pass.

All completed captures use iPadOS27.0 build24A434. The2× device is an existing
iPadPro11-inch(M5,16GB) simulator. Glass tint is process-local through the
argument domain; no global setting is changed. Fresh runIDs, effective tint,
screen scale, filter archives and requested changes are retained. Existing
simulators were restored to their initial shutdown state. A newly created2×
simulator stalled on its first app installation and was removed; it produced
no accepted capture.

## Independent exterior controls

An untouched public-UIKit scene reproduced the original52/80/120pt dark/white
side samples exactly:116/90/57 at.25pt outside. This establishes that the old
native reference remains valid. The four knockout scenes contain three copies
of each size on an entirely white framebuffer. Every accepted acknowledgement
has nine glassBackground records and nine external-highlight records.

| Native knockout |52pt|80pt|120pt|
|---|---:|---:|---:|
| Baseline |116|90|57|
| External CASDF highlight opacity0 |116|90|57|
| Internal filter highlight amount0 |253|250|241|
| Elevation shadow opacity0 |117|93|61|

The external CASDF highlight does not explain the exterior difference. The
internal filter highlight does. At.75pt outside, the baseline shadow gives
253/251/242; disabling elevation gives255 at all three sizes. These samples
must distinguish shadow-enabled and shadow-disabled tests.

`knockouts/samples.json` contains raw PNG component readbacks. Sampling uses
pixel centers from the observed integer frames, without registration or color
adjustment. The separate52/80/120 scene and its archived settings are retained
as `2x-interactive.png/json`. These results concern finite exterior controls,
not whole-image optical equivalence.

## Recipe inputs and causal controls

Six processes cover light/dark × tint0/.5/1, each with56 surfaces: four styles
(regular,clear,tinted,clearTinted), seven sizes, circles and wide capsules.
All336 archived filter inputs were decoded and matched against the existing
3× profiles. Exactly three scalar fields differ; every other scalar is equal.

Public child-controller trait overrides then varied display scale and idiom on
the same2× iPad screen. Each process reports the actual view traits as well as
the physical screen scale. All four glass styles agree:

| View display scale | View idiom | Height | Offset | Color bias |
|---:|---|---:|---:|---:|
|1|pad|1|−1|−.25|
|2|pad|.5|−.5|−.25|
|2|phone|.5|−.5|−.25|
|2.5|pad|.64|−.64|−.3|
|3|pad|.533333|−.533333|−.3|
|3|phone|.533333|−.533333|−.3|

This isolates the view's display-scale trait from screen hardware and idiom.
The observed lower-scale recipe has a one-device-pixel height and bias−.25;
the higher-scale recipe has a1.6-device-pixel height and bias−.3. Offset is the
negative height. The exact native selection threshold between2and2.5 has not
been recovered. Any portable selection policy outside measured points must
remain explicit rather than being described as a recovered CPU equation.

The existing3× interactive/no-interactive check also decoded42 actual
`glassBackground` archives per process and found identical inputs. Filter
validation must traverse the complete window layer tree: a public effect
view's own layer tree may contain zero filters even while its private material
layers render elsewhere. Zero-filter comparisons are not accepted evidence.

## Reproduction and limits

The recorded fixture and driver sources accompany the raw data. `TraitProbe.m`
uses public, deprecated-but-supported controller trait overrides; it records
actual traits to verify their application. `ContextProbe.m` captures all
styles/sizes. `RimLightingProbe.m` changes only the named native effect inputs
or external-highlight layer opacity. The parser resolves keyed archives;
it does not decompile or copy Apple shader payloads.

Validate the checked-in archive counts and causal settings with:

```sh
python3 Demo/GlassComparison/NativePassProbe/analyze-display-scale.py \
  docs/liquid-glass-native-settings/display-scale
```

`verification.json` records source, raw-capture and executable hashes. The
2× profiles and public trait experiments justify the three-setting translation;
other scalar equations retain their separate AIR and framebuffer evidence.
The historic1×/3× integrated-rim-width assertion was a portable scale-invariance
assumption, not an independent native framebuffer measurement. It must not
force a fitted change to the observed native scale recipe. The original2×
native byte controls remain unchanged.
