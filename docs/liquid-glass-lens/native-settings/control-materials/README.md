# Control-specific lens backdrop and blur

Audit: 2026-11-06, retained iOS/iPadOS27.0 Simulator24A434 captures.
This is an archive/graph investigation, not a comparison inferred from pixels.
The earlier manual-lens profile extraction retained lifted optical settings
but did not translate the actual controls' resting backgrounds or the
small-variant transition backdrop endpoints.

## Evidence

Original native read-only settings driver:
`ev_00804c3e2e408ea042163c6889f41fa905db7c931b6708550354b3b1cb0865b7`.
The original record reports truncated overall capture coverage; the retained
144 compressed responses have request identities and individual hashes.

The new bounded [REA audit](rea-audit.json),
`ev_f4dd55e7d34af944445c3a603ff2446182aa43b34be89588d7a03d84dcfc859a`,
decodes all144 responses and checks432 actual-control rows. Exit0,
`truncated:false`. This new process executes only a plist/JSON decoder;
it does **not** execute UIKit or observe a new control transition. The
[normalized native values](settled-controls.json) retain every source hash,
six representative actual-control profiles, exact filter archive hashes,
filter order, color matrices, backdrop scale, and semantic control paths.
Archives are interpreted as data without instantiating archived classes.

## Confirmed control differences

| Control | Native lens path | Style | `warpsContentBelow` | Resting filter observation |
| --- | --- | --- | --- | --- |
| UITabBar | `stage.1.0.1` | Large1 | true | Caller-owned `_UITabSelectionView`: Gaussian2 then color matrix |
| UISwitch | `stage.2.0.2` | Small0 | true | Cold initial opaque white background; no attached backdrop filter |
| UISlider | `stage.3.0.1` | Small0 | false | Cold initial opaque white background; no attached backdrop filter |

These roles are established by the fixture's explicit UITabBar, UISwitch and
UISlider construction and the recorded native hierarchy, not by guessing
from filter names. All actual controls are settled/resting in this144-case
matrix: changing the manual lens's lift request does not lift them. Their
normalized values are identical within each theme across the tested manual
styles, sizes and glass-preference values0,.5,1.

The tab's resting blur lives at lens-relative `.0.0`, on the
`_UITabSelectionView`'s CABackdropLayer. Its ordered filters are:

1. `gaussianBlur`: `inputRadius=2`, `inputNormalizeEdges=true`.
2. `colorMatrix`: exact20 coefficients in the normalized JSON.

The layer reports `scale=.25`, `marginWidth=0`. The sibling ClearGlass
selected-content wrapper has displacement0 at rest. The generic manually
created resting lens does not have the tab's blur. Therefore this is a
control-supplied resting material, not evidence that every resting selection
lens must blur. Radius2 is the archived filter input, not a measured physical
point-space Gaussian sigma; native downsampling/compositor conventions must
be preserved separately.

The actual small variants report `unliftedDisplacement=50`,
`unliftedBlurRadius=6`, `liftedDisplacement=9`. Large variants report0,0,9.
These are native variant settings. They establish the two endpoint values,
but this settled matrix does not establish their per-frame interpolation or
when the initially absent backdrop view is created or removed. The new
stationary native-control filter traces are required for those lifecycle
claims. Subsequent [direct-key slider traces](../../../slider/lift-motion/README-native.md#blur-and-displacement-are-independent)
resolve an important qualification: `warpsContentBelow=false` does **not**
disable the complete below-glass pass. The Gaussian remains active, following
`max(0,6*(1-q))` on first lift and a geometry-spring return toward6. Cold first
contact displacement is `50*(1-q)`; settled lift and the captured retirement
use displacement0. Here `q=(presentationWidth-37)/20.35`. The older endpoint
matrix alone could not establish these distinct timelines.

## Portable mapping required

- Give the lens a separate below-glass warp/blur pass, distinct from its
  ClearGlass material and selected-content source. Carry the native
  `warpsContentBelow` policy independently of lift opacity. UISlider sets it
  false, but its Gaussian still operates; settled/releasing displacement is0
  rather than the generic lifted9. Do not use this boolean to disable blur.
- Keep native small-variant50/6 and lifted9/0 as transition endpoint settings
  instead of applying the lifted+9 displacement at every positive progress.
  The direct slider probe establishes independent Gaussian, first-contact
  displacement, and selected-wrapper timelines; generic switch interpolation
  remains unverified. Cold lazy attachment is not equivalent to opacity fading.
- Supply the tab-specific resting backdrop material with Gaussian2 and the
  exact light/dark color matrices. A fitted point-color matrix with zero
  blur omits a native filter stage.
- Keep below-glass blur separate from glassBackground's own blur inputs.
  The retained **lifted manual** glassBackground has zero blur radius and
  zero blur-fill/bleed radii. That does not justify disabling a control's
  resting material blur or the small variant's transition blur.

These findings do not establish arbitrary switch transitions, physical
device/HDR color packing, a point-space blur-kernel conversion, or a full
native compositor implementation. In particular, generic lifted lens
parameters alone are insufficient to infer slider track sampling.

## Reproduce

```sh
/usr/bin/python3 docs/liquid-glass-lens/native-settings/control-materials/audit.py
```

This checks every retained source and deterministically regenerates the
normalized JSON. No Simulator, application launch, private setter, or device
connection is required for this archive audit.
