# CASDFOutputEffect support versus shadow coverage

Focused shipped-artifact inspection: 2026-11-06. A radial alpha cutoff at
the captured output maximum8.95046394 can cut the lens's shifted ring shadow
while its Gaussian still has visible coverage. The native output effect does
not implement that radial alpha multiplication.

## Static evidence

REA `ev_c8c1a99a98909a253851a4d21d859319fdebda6157c130a6370cb4c4d41c0a9c`
([complete record](rea-shadow-padding.json)) inspects the matching extracted
QuartzCore SHA256
`5d8010ed6b940cc1e77daa8bcd91675c8bb289e2c2573ed6eca338e9454f8380`.
The audit exits0. REA reports overall truncation; the complete structured
terminal summary and generated disassembly hash are retained. This is static
inspection, not a native allocator trace or runtime shader-uniform capture.

1. `-[CASDFOutputEffect configureLayer:transaction:]`, `0x1c3b365f4`, writes
   effect type1 and the minimum/maximum floats at offsets0x18/0x1c.
2. `CA::OGL::SDFNode::apply` dispatches type1 at `0x1c3a63194` to
   `0x1c3a6350c`. That branch calls `emit_combine(TextureFunction1)` using
   the already-generated SDF surface and exits. It does not load the
   minimum/maximum and construct an iso-distance alpha ramp.
3. `CA::Render::SDFLayer::sdf_padding`, type1 at `0x1c3a80ac4`, reads
   `(-minimum,maximum)` into its padding pair. Common code adds smoothing,
   Gaussian padding and effect-offset contributions. These values participate
   in field-support calculation.
4. `SDFNode::propagate_roi` recognizes type1 at `0x1c3a62db4` and delegates
   the requested region to its child. No type1 radial mask is applied there.

The manual native lens output layer records minimum−10000,
maximum8.950463938720569, effectOffset0, gaussianRadius0. Its sibling
foreground output records maximum1. Those values describe the captured
native field setup; they are not directly opacity-mask endpoints for the
completed glassBackground image.

## Portable consequence

Remove the unsupported `smoothstep(maximum±AA, baseCapsuleSDF)` multiplication
from the lens's completed exterior shading. Evaluate shadow and ring coverage
using their own shifted signed distances and recovered Gaussian equations.
In particular, the ring's vertical offset must be included before deciding
which exterior pixels can contribute.

Allocate or retain sufficient finite sampling support for the translated
effects, instead of clipping their alpha at the unshifted body's maximum.
A portable support bound can take the maximum of native field padding,
ring offset plus stroke and a chosen Gaussian tail margin, and shadow offset
plus its radius tail margin. The chosen finite-tail threshold (for example
three sigma) is a portable error/budget policy, **not** a recovered native
allocator formula. The exact native support rasterization and transformed
offscreen allocation remain unknown. This investigation does not justify
changing the body's native antialiasing or the foreground's optical mask.

The separately configured inner-shadow UIView has black shadow opacity.06,
radius3, offset(0,7). The earlier manual portal captures had no contents,
background, sublayers, mask or explicit shadowPath for that node. A portable
exterior silhouette inferred from appearance must therefore be documented
as a reconstruction unless an actual-control capture establishes the shape.

```sh
/usr/bin/python3 docs/liquid-glass-lens/native-settings/control-materials/shadow-padding-audit.py
```

This reads the matching local extracted native binary without launching UIKit.
