# Native blur-grid origin check

Three fresh UIKit processes place the same 280×80pt glass at X0, 24 and 25pt
over fixed window-coordinate stripes. Blur radius is 5, capture preference 1;
face/refraction/fill/bleed/shadows are neutralized by fresh filter copies.
Four stripe periods cover 4/8/16/32pt. Every JSON records the glass position,
fresh run identity, original profile archive and mutation readbacks.

All sampled scanlines at fixed physical X300..683, center Y of each row are
byte-identical across the three placements: 9,216 compared RGB components,
maximum difference zero. For this finite case, shifting the glass does not
shift the native blur grid. This rules out a simple per-glass-origin phase
explanation; it does not establish texture dimensions or arbitrary LOD behavior.
PNG dimensions are 1206×2622 at 3×. See `evidence.json` and `comparison.json`.

Reproduce by building/installing `KernelProbe.m`, then:

```
python3 Demo/GlassComparison/NativePassProbe/capture-kernel-origin.py --device DEVICE --output REPORT
```
