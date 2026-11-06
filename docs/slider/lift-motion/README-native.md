# Native slider lift and retirement reference

The isolated `Demo/SliderComparison/LiftReference.m` fixture observes Apple's
UISlider responding to actual XCTest OS touches. It never calls private
setLifted methods, moves the native lens, or manufactures tracking callbacks.
Both light and dark captures use iOS 27.0 (24A434), iPhone Simulator, 3×. A
120Hz display-link preference yielded approximately 60Hz in the measured runs;
raw timestamps and dropped samples are retained.

## Stationary contact

The model thumb changes from 37×24pt to 57.35×37.20pt. Presentation bounds
overshoot to approximately 59.85×38.82pt, then settle. Stationary presentation
transforms remain identity, so this enlargement is independent of moving-drag
flex. Release undershoots to approximately 36.04×23.38pt.

The native white resting child's presentation opacity changes with the lift
curve, including a raw negative overshoot of approximately −.123. The external
highlight SDF opacity is its complement, reaching approximately 1.123. These
raw presentation values should not be confused with unrestricted framebuffer
alpha. Every sampled glass/backdrop container stays at opacity 1, including
after the resting white fill returns. Native glass is not globally crossfaded
with the same scalar as the resting fill.

| Phase / time from window touch dispatch | Width (pt) | White presentation opacity |
| --- | ---: | ---: |
| Press +.0906s | 47.8838 | .465167 |
| Press +.1239s | 55.5979 | .086100 |
| Press peak +.1905s | 59.8529 | −.122993 |
| Release +.0497s | 54.6293 | .097443 |
| Release +.1003s | 48.8135 | .376361 |
| Release +.1997s | 39.5246 | .782164 |
| Release minimum +.3497s | 36.0413 | .966815 |
| Release +.5000s | 37 | 1 |

These are actual nearest captured samples, not authored animation times or a
claim that UI event dispatch and animation commit are simultaneous.

The separately recovered small-lens configuration specifies lift response .27,
damping .625, retarget impulse .032; retirement response .5, damping .7,
retarget impulse .016. An analytic second-order reconstruction with normalized
initial velocity `ω² × impulse` matches the stationary curves after accounting
for the capture's commit phase. The return of white opacity follows a separate
approximately .4-response critically damped curve. This is a reconstruction
validated against samples, not recovered source for Apple's private integrator.
The production comparison records its fit residuals separately.

Native size completion is observably property-specific. In the light run,
width changes from 57.5818 to its exact 57.35 target between .2509 and .2676s
after public beginTracking returns; white opacity continues until approximately
.3509s. On retirement width changes from 36.5454 to its exact 37 target between
.4670 and .4830s after touch-end dispatch; white reaches exactly 1 at .5000s.
The raw curves retain these completion discontinuities. A fixed 60Hz delay is
not a portable animation rule.

## Ordering and lifecycle

During contact the native lens container's ordered children are:

1. The dedicated warp backdrop.
2. The white resting fill.
3. ClearGlass, including the material, source and foreground passes.

Their zPositions are zero. This establishes ordering, but does not alone
establish private backdrop sampling exclusions. A single combined optical
effect sampling a fading white fill is not the same composition.

Stationary endTracking, touchUpOutside (event 128), and final valueChanged occur
immediately after native touch-end dispatch, while the visual retirement
continues. The callback reports touchInside=true despite event 128; this is
retained as native output rather than silently renamed. Moving/decelerating
slider retirement has a different control timeline, documented in the earlier
[tracking reference](../README.md).

Short taps complete the lift before returning. Approximately 62ms and 45ms
delivered contacts still reach the full lift overshoot; model size first
returns toward rest around .230s and .245s from the initial dispatch. This is
consistent with the recovered .22s minimum lens hang time. The exact internal
deadline origin is not identified: public tracking callbacks, native gesture
initialization, commits and display callbacks have different clocks. The
filter-instrumented timing run dropped samples and is excluded from fitting
that deadline.

## Blur and displacement are independent

The final bounded direct-key probe is retained in
`native/direct-filter/direct-filter-slider`, including `filters.csv`. Earlier
`filters` and `filter-inputs` runs exposed filter names but no numeric inputs:
CAFilter's inputKeys getter returned nil. The final fixture therefore read only
the known inputRadius, inputAmount and inputSourceSublayerName keys, each guarded
against exceptions. This established a material difference that filter names or
the native warpsContentBelow=false getter alone could not establish.

**Observed:** the dedicated backdrop at `slider.0.1.0.0` applies displacementMap
and gaussianBlur. During first lift, its radius equals `max(0, 6*(1-q))`, where
`q=(presentationWidth-37)/20.35`, to floating-point precision while geometry is
still animating. Retirement radius rises from 0 toward 6 with the geometry
spring, including an overshoot to 6.21445 at +.29974s after touch-end dispatch.
It snaps to 6 by +.31639s, earlier than bounds finish. At +.04973s retirement,
radius is .80217; +.09974s: 2.51691; +.19971s: 5.25564. Blur remains despite
warpsContentBelow=false. Removing this pass makes the blue progress line
incorrectly sharp during lift and retirement.

First-contact backdrop displacement equals `50*(1-q)` initially, including a
negative −6.16571 overshoot. Its oscillation continues after bounds snap to
target. After touch-end its inputAmount is exactly 0 throughout the captured
retirement. The separate ClearGlass wrapper displacement lifts from 0 to −17.5,
then returns with the white-fill/highlight opacity clock: while white opacity
is strictly between 0 and 1, `amount=-17.5*(1-whiteOpacity)` has maximum residual
5.21e−7. Its tiny tail survives the white-opacity completion cutoff.

**Inferred:** independent optical followers reproduce these scalar traces;
they should not share the bounded geometry or white-opacity completion clock.
The presence of cold first-contact displacement does not establish that it
recurs on later contacts. **Unknown:** subsequent-lift behavior, CAFilter's
enabled flag, exact private render-source exclusions and the native shader's
kernel implementation. Switch's numeric interpolation was not captured by this
final slider-only probe; the earlier switch traces establish layer/filter
presence, not scalar timing.

## Interruption limits

Two requested short-contact paths delivered two real contacts, but XCTest
compressed the requested 60ms gap to approximately 1ms in one run. That trace
cannot establish the authored idle interval. A later single-pointer path
attempt with the runtime-checked pressDownAtOffset selector produced only one
native down/up contact, despite requesting two. Those `regrab-*` files are
negative input-coverage evidence, not a successful native regrab test.

Preserving velocity and cancelling pending retirement when an application
regrabs a portable slider remain reconstruction choices with portable
regression coverage. No exact native interrupted-solver equivalence is claimed.

## Resting-fill composition ablation

`native/fill-ablation` retains a separate, explicitly mutated native experiment.
During a real stationary hold, the fixture leaves the lifted glass and layer
order untouched, then sets only the resting child's background red, green and
white at opacity 1. [The contact sheet](native/fill-ablation/contact-sheet.png)
and eight selected crops come from passive native video, with marker IDs and
timestamps in `selected-frames.json`. The native opaque white fill develops a
darker internal contour under the still fully lifted glass. Colored fills also
interact with its appearance. Therefore the actual native order cannot be
replaced by a claim that resting content is always excluded from optical
composition. This artificial fully lifted/opaque combination does not establish
the appearance or routing of an unmodified retirement.

Before the color mutation, root opacityPair's inputAmount is observed to equal
`clamp(4*(1-whitePresentationOpacity),0,1)` exactly at captured press samples.
During release, while the native inner-shadow child's shadowOpacity has not
completed, `min(1,4*shadowOpacity/.06)` matches that input within 2.1e−8. Both
properties change while their container layer opacities stay 1. These distinct
filter and shadow clocks matter when recreating retirement; a constant glass
shader plus only a fading white fill is insufficient. This observation does
not recover the compositor's private offscreen-group routing.

The initial two screenshot-watcher attempts were discarded: Simulator capture
requests took multiple seconds and the labels no longer described the visible
phase. The retained third run uses passive video and the actual frame markers.
Its P3-to-sRGB conversion and compressed-video limitations are the same as the
motion crops. The ablation source snapshots distinguish these authored color
mutations from the unmodified native motion tests above.

## Material parameters during retirement

The final single light-mode, unmodified hold/release probe explicitly reads the
69 known glassBackground input keys plus the optional inputAmount (nil). Its
119 material samples, full endpoint table and per-frame CSV are retained under
`native/material-inputs/material-inputs-slider`. Container opacity 1 does not
mean the material is constant: faceOpacity, inner-refraction geometry, ring
shadow and highlight inputs change internally during retirement.

| Input | Lifted model | Retired model |
| --- | ---: | ---: |
| Face opacity / black | 1 / .1 | 0 / 0 |
| Inner refraction amount / height | −11.1 / 6.66 | 0 / 0 |
| Ring shadow opacity / blur / stroke / offset | .15 / 3 / 4 / 8 | 0 / 1 / 0 / 0 |
| Ring shadow mask | 0 | 1 |
| Shadow amount / radius | 9.25 / 8 | 0 / 0 |
| Key-fill highlight amount / height / offset | .5 / 1 / −2/3 | .4 / 0 / 0 |
| Key-fill highlight spread / SDR spread | π/2 / 2π/3 | 5π/12 / 5π/12 |
| Maximum headroom | 9999 | 1.2 |

Outer-refraction amount/height, refractionOpacity and blurRadius stay 0 in this
probe. That does not independently prove every internal refraction branch is
disabled. The shadow fill changes from RGB(.3375,.4575,.5625), alpha 1 toward
transparent/nil; shadowOpacity changes from .0018 to 0 immediately at release.
The complete table records every queried value, including nil/default cases.

Observed affine relationships show most material inputs following the returning
white-fill critical clock, with independent completion/clamping. The inner
refraction amount/height and shadowAmount share a slightly different follower.
A .4-response critically damped reconstruction of their normalized retirement
uses an initial decay velocity .88773090396824/s, compared with zero for face
opacity; this value is constant across the interior captured samples within
3e−14 under that fit. It is a measured reconstruction law, not recovered native
source. Applying one fixed lifted profile beneath a fading white fill misses
these material changes and creates an excessive internal contour.

Only the light-mode material endpoint transition was freshly sampled here.
Dark-mode color parameters, arbitrary interruptions and the compositor's
offscreen-group routing remain outside this bounded probe.

## Retained evidence

`native/stationary` and `native/interruption` contain gzipped complete layer
traces, model/presentation curves, touch/action streams, animation metadata,
raw source hashes and exact compiled fixture sources. Public layer animation
inspection found only private match-property/match-move placeholders on a
material SDF child, with nonfinite durations. It did not expose public
CASpringAnimation coefficients for thumb bounds or resting opacity.

[Video extraction](video/README.md) retains 593 distinct native crops and six
precontact crops, with video marker IDs mapped to trace frames. The source
videos are variable-rate compressed recordings. Explicit Display-P3-to-sRGB
conversion makes the crops useful for visual comparison; they are not native
lossless P3 screenshot evidence. Marker commit versus presentation-layer
sampling can differ by a compositor frame. Missing marker IDs and duplicate
encoded frames are recorded; no frames are synthesized.

The original XCTest runs and raw logs are independent native runtime evidence.
[Final REA artifact verification](native/rea-final-artifact-verification.json),
Evidence `ev_825fcfa0344a2e6f95296b0b650a2430fe7f2d5c84da86bc28c37ac1cff6ceab`,
exited 0 with untruncated capture. It checks all 19 retained traces, original
motion video/crops, eight selected ablation crops and their video, marker
references, and the complete 119 material samples. Its coverage is retained
artifact verification, not a new UIKit execution.

[REA artifact verification](native/rea-artifact-verification.json), Evidence
`ev_95c1a6bc5986502d67115fc33a6d6a4083d7a84ec5f896d6dbf276508769d55c`,
checks all 16 retained trace hashes, 599 crop hashes, both source videos and
marker references, plus the direct-filter relationships above. The command
exited 0. REA reports its overall capture truncated; the complete verifier
result is present in the retained output frames. This verification does not
retroactively observe UIKit execution or establish the unresolved regrab case.

```sh
/usr/bin/python3 Demo/SliderComparison/capture-lift.py DEVICE OUTPUT Demo/SliderComparison/lift-stationary-cases.json --video
/usr/bin/python3 Demo/SliderComparison/capture-lift.py DEVICE OUTPUT Demo/SliderComparison/lift-interruption-cases.json --video
/usr/bin/python3 Demo/SliderComparison/analyze-lift.py TRACE_JSON_OR_GZ OUTPUT
/usr/bin/python3 Demo/SliderComparison/verify-lift.py
/usr/bin/python3 Demo/SliderComparison/analyze-material-inputs.py TRACE_JSON_OR_GZ OUTPUT
```

The fixture reads an already booted Simulator. It does not reset services,
change persistent system settings, or touch a physical device.
