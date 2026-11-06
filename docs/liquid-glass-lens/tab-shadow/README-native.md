# Native tab selection lens shadow retirement

The bounded probe presses the already selected Home button in a native
UITabBarController for one second, then releases it, once per theme. The iPhone
Simulator runs iOS 27.0 (24A434) at 3×. No private lens setter, authored lift state,
or physical device is involved. `native` retains exact fixture sources, the
successful XCTest log, raw compressed model/presentation traces and passive
native video. The UITabBar root is not UIControl; its generic control flags are
explicitly unavailable rather than treated as actual tracking state.

## Observed material changes

The actual selection lens is layer identity 27. It reparents during contact;
the animated glassBackground material is identity 71. Match identities and the
current recorded path rather than assuming a fixed original hierarchy path.
Each theme has one delivered down/up pair. Light has 154 total/134 material
samples; dark has 153 total/133 material samples. Every material sample includes
all 69 known inputs plus the optional inputAmount query.

| Input | Lifted | Retired |
| --- | ---: | ---: |
| Face opacity | 1 | 0 |
| Inner refraction amount / height | −10.5 / 7 | 0 / 0 |
| Ring shadow opacity / blur / stroke / offset | .1 / 3 / 4 / 8 | 0 / 1 / 0 / 0 |
| Ring shadow mask | 0 | 1 |
| Shadow amount / radius | 17.5 / 8 | 0 / 0 |
| Shadow opacity | .001 | 0 |
| Key-fill highlight amount / height / offset | .5 / 1 / −2/3 | .4 / 0 / 0 |
| Key-fill highlight spread / SDR spread | π/2 / 2π/3 | 5π/12 / 5π/12 |
| Maximum headroom | 9999 | 1.2 |

The shadow fill is black with alpha 1 while lifted and retires toward
transparent/nil. Face black remains zero; outer-refraction amount/height,
refractionOpacity and blurRadius also remain zero. Shadow opacity switches its
model target to zero at release, without an interpolated scalar ramp. Container
opacity stays one while these shader parameters change. Keeping a fixed lifted
profile until the last frame therefore fails to reproduce retirement.

## Normalization and slider differences

While the return is in its interior, faceOpacity, innerRefractionAmount/−10.5,
innerRefractionHeight/7 and shadowAmount/17.5 share the same normalized phase q
within 2.3e−16. Ring opacity follows .1*q until its independent completion clamp.
The resting selection view opacity is 1−q, subject to float precision and its
completion. Root opacityPair is clamp(4*q,0,1) while faceOpacity remains active;
its small tail continues past the face completion cutoff.

The held material coefficients use a 70pt reference height: −.15*70, .1*70 and
.25*70. During retirement they scale by q while the live lens height changes
70→54. They do not recompute the coefficients from each shrinking live height.
The SDF geometry still follows those live bounds. Lifted lens bounds are
120.5×70; resting bounds are 104.5×54.

Unlike the slider's slightly different inner-material follower, the tab's
normalized inner and face clocks agree. A .4-response critically damped return
with zero initial decay velocity reconstructs this native phase. The per-theme
least-squares alignment absorbs CA commit versus touch-dispatch phase:

| Theme | Interior samples | Alignment | Maximum q residual | RMS q residual |
| --- | ---: | ---: | ---: | ---: |
| Light | 28 | 16.37960ms | .00027975 | .00006068 |
| Dark | 27 | 16.38372ms | .00007043 | .00001909 |

This alignment is a measurement nuisance, not a delay to add to production.
`normalized-curve.csv`, `inputs.csv`, `endpoints.json` and `clock-fit.json` retain
the actual finger-up-relative samples, complete parameters, fit scope and source
hashes. Completion clamps are excluded from the interior fit. The analytic
follower is a reconstruction supported by measurements, not recovered source
for Apple's integrator.

## Evidence and limits

The native test and traces establish this stationary selected-tab interaction
in both themes. Arbitrary tab counts, moving selection, interruption, repeated
regrabs and system tint changes are outside this bounded probe. The existing
large-lens profile supplies a useful baseline, but the observed control-specific
retirement above is stronger evidence than assuming a settled profile persists.

Selected crops and `contact-sheet.png` are derived from the retained native
video and correlated using the visible frame marker. They are compressed
Display-P3 video explicitly converted to sRGB. Marker commit and presentation
sampling can differ by about one compositor frame; these are visual evidence,
not lossless pixel-error ground truth. `selected-frames.json` records the actual
timestamps and hashes.

[REA artifact verification](native/rea-artifact-verification.json), Evidence
`ev_cd4c4d267a3ea53417a9de9b319bb47c5fff6ebce9710a75fe0026881268d004`,
exited 0 with untruncated output. It checks both raw trace hashes, compiled
source hashes, all 267 complete material samples, mutual normalized phases and
the ten selected video crops. This verifies retained artifacts; the independent
XCTest logs and traces are the native runtime observation.

Reproduce the native contact capture with:

```
/usr/bin/python3 Demo/SliderComparison/capture-lift.py DEVICE OUTPUT Demo/SliderComparison/tab-shadow-cases.json --video
/usr/bin/python3 Demo/SliderComparison/verify-tab-shadow.py
```

The capture uses an already booted Simulator, does not change system settings,
and does not install on a physical device.
