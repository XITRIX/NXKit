# Opacity-pair filter and resting-fill sampling

Focused static audit: 2026-11-06. The question is whether the native lens's
root `opacityPair` filter itself excludes the white resting child from the
later ClearGlass backdrop, or changes their painting order.

## Confirmed shipped filter operation

REA `ev_9c2e0de9d92e55fec2bea529a391bc7c0e2d1ac5aaefe13453b524a944552912`
retains a read-only LLDB inspection and extracted AIR verification. The
[full capture](rea-opacity-pair.json) exits0, but reports overall truncation;
its terminal result and saved CPU/AIR file hashes are retained. This is
static inspection, not observation of live render uniforms or CA execution.

QuartzCore SHA256:
`5d8010ed6b940cc1e77daa8bcd91675c8bb289e2c2573ed6eca338e9454f8380`.
`CA::OGL::render_opacity_pair_filter` begins at `0x1c398c824`. It reads
the scalar filter amount, clamps it to0…1, binds one input surface and
selects fixed-fragment opcode99. At t>0.998039186 the helper returns without
emitting its filter quads. This is not evidence that the caller passes the
input through unchanged; the caller's destination/capture state matters.

The shipped `fixed_frag_lpf_cpf` AIR (SHA256
`ff057d63d2fdb04b277760d0270def4e32bdd75e6ea2c6ce466ae98e0ee5b145`)
dispatches opcode99 to block2409. Its exact premultiplied operation is:

```
t = clamped inputAmount
outputRGBA = inputRGBA * (1 - t) / (1 - t * inputAlpha)
```

The block reads one already-composed source pixel. It contains no child
index, sibling traversal, white-color test or second source texture. For
alpha1 and t<1 the result is unchanged. Therefore this filter's shader
operation does not itself prove a different white/glass paint order or a
white-branch exclusion. The equation does not describe earlier capture
setup or offscreen-group formation.

The equation has a specific paired-alpha interpretation. If another branch
has alpha `t*a`, drawing it over an under-branch with alpha
`(1-t)*a/(1-t*a)` gives total alpha exactly `a`. Thus it can compensate the
fractional edge coverage of a paired crossfade. This is algebraic inference,
not evidence assigning actual native white/glass surfaces to those branches.
It must not be replaced with a plain global opacity multiplier without
checking the render-surface routing.

## Captured layer context

The [native stationary/direct-filter reference](../../../slider/lift-motion/README-native.md)
orders the dedicated warp backdrop, white resting child, and ClearGlass
under the same lens content wrapper. In the direct-key trace, actual slider
glassBackground id21 reports `groupName=""`, `sourceLayer=nil`, scale1;
foreground id29 reports `groupName=nil`, `sourceLayer=nil`, scale1.
Glass-background margin targets2/displayScale during lift and returns0;
foreground margin stays100. Existing manual-lens archive metadata reports
`groupNamespace=owningContext`, `usesGlobalGroupNamespace=false`, and
`sourceSublayerDefinesBounds=true` for these material backdrops.

The actual-slider wrapper source portal (id16) is nil in the separately
captured lifted-control ablation, both model and presentation. Its parent
wrapper is a sibling of the material provider, not an ancestor of the
glassBackground or highlight. The wrapper's changing displacement therefore
does not warp every ClearGlass output; an empty caller-content portal cannot
explain a refracted white-resting-fill contour.

The ClearGlass root's `_UIMultiLayer` class has nil class/instance baseMethods
in the inspected Objective-C class_ro metadata. This is a bounded metadata
observation. It does not rule out external handling of that marker class,
inherited behavior, filter-group isolation, or private render-list ordering.

## Subsequent native controls

The retained native [colored-fill ablation](../../../slider/lift-motion/native/fill-ablation/contact-sheet.png)
forces an opaque white/red/green resting child while glass stays fully
lifted. Native white then develops the internal contour too; colored fills
also interact with the material. The [marker-associated frames](../../../slider/lift-motion/native/fill-ablation/selected-frames.json)
and [recording](../../../slider/lift-motion/native/fill-ablation/native-ablation.mp4)
therefore reject a blanket claim that resting-fill colors are excluded from
the native optical/material path. Moving white above all optics would conceal
a real material-state mismatch rather than reproduce this control.

The fresh actual-slider [69-input endpoint table](../../../slider/lift-motion/native/material-inputs/material-inputs-slider/endpoints.json)
and [per-frame values](../../../slider/lift-motion/native/material-inputs/material-inputs-slider/inputs.csv)
show that glassBackground itself changes during retirement. Face opacity
falls1→0, inner-refraction amount/height return to0/0, ring opacity falls.15→0,
and its blur/stroke/offset/mask change3/4/8/0→1/0/0/1. A fixed fully lifted
profile beneath a fading white fill cannot reproduce those native states.
The separate material clock is stronger evidence for the natural retirement
appearance than the earlier sampling-isolation hypothesis.

The final [REA artifact verification](../../../slider/lift-motion/native/rea-final-artifact-verification.json),
`ev_825fcfa0344a2e6f95296b0b650a2430fe7f2d5c84da86bc28c37ac1cff6ceab`,
checks19 retained traces and cropped video artifacts, exits0 and reports
untruncated coverage. It verifies retained artifacts; it does not
retroactively observe UIKit execution. The native fixture's OS touch traces
and explicitly mutated fill controls are separate runtime evidence.

## Remaining composition boundary

A root filter can cause offscreen-group construction before its final pixel
equation runs. Exact native capture boundaries still require additional
evidence, but they no longer justify a blanket resting-fill exclusion: the
positive colored-fill control directly contradicts that explanation. The
group-name strings and opacity-pair shader alone do not establish native
render-surface routing.

The actual-control material capture now establishes the missing retirement
state. It does not prove all low-level native offscreen rendering details or
dark-mode endpoint equality; retain those limitations separately.

```sh
/usr/bin/python3 docs/liquid-glass-lens/native-settings/control-materials/opacity-pair-audit.py
```

The script requires the matching extracted native artifact and AIR at the
documented local scratch paths; it does not attach to or launch UIKit.
