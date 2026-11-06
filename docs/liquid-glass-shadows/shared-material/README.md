# Shared neutral material sizing

The native shadow fixtures revealed that mixed-size neutral regular members also share their largest member's body material response, including disconnected members. The correction re-resolves each copied neutral regular `UIGlassEffect` at the largest eligible member's shortest dimension. Every optional optical override is retained. The actual lens size, local center, corner shape, merged SDF, touch coordinates and transformed geometry remain those of the individual member. Clear glass and explicit colors keep their own material sizing; their cross-size native behavior is not established by these fixtures.

The eight measured groups use neutral regular glass, white/gray, both themes, 64/96/144 circles plus a 132×56 pill and 240×160 card, or two close 52-point circles beside the latter large controls. The preserved native PNGs and requests are in the parent report's `native/` directory. There are no glyphs or touches. The body mask is each shape's observed rounded SDF below −20 logical points, excluding the optical perimeter. Native ICC conversion is retained. No image registration, contrast adjustment or color fitting is applied.

| Body RGB MAE, 0–255 | Separate material sizing | Shared neutral sizing |
| --- | ---: | ---: |
| Light | 2.568 | 0.441 |
| Dark | 20.641 | 0.491 |
| Both, 40 element regions | 11.605 | 0.466 |

Dark-white small 64/96/pill centers previously measured 171/138/180 while native was 121 for every member. The shared resolution produces 121. Light-gray native members measure approximately 203; separate sizing produced 195–198 on the smaller members. Gray-dark native small members measure 115 versus 130–134 previously; the corrected value is 116. These residuals are retained rather than adjusted by per-theme or per-shape offsets.

The light-white controls are already close to the channel ceiling and slightly worsen in aggregate (0.561→0.858 RGB MAE); the smaller native circles retain a roughly 1–2 level local response while shared NXKit resolves near 254. The complete light-theme aggregate improves because the gray controls improve substantially. Larger/default members and shadow geometry remain unchanged. This is broad agreement, not pixel parity, and nonuniform backgrounds in heterogeneous neutral groups have not been validated. Shader detail response still uses the individual lens geometry.

![Dark white group: native / separate sizing / shared sizing / body difference ×8](white-dark1-clear0-group1.png)

![Light gray group](gray-dark0-clear0-group1.png)

The new focused container regression checks shared body response in both themes, immediate size/hidden-member updates, explicit tint and clear exclusions, and fixed optical overrides. Both the complete existing ContainerTests and focused new tests pass against the isolated candidate. The final resize assertion is included in the final validation run.

All **96 existing two-item 52-point retained-join rasters are byte-identical** before and after this additional CPU resolution step. Full-image hashes and exact archive identifiers are in `provenance.json`; this covers both palettes, both themes/orders, initial/held/pulled/settled states and gaps2/4/12. It does not imply that those rasters perfectly match native.

The combined current material/shadow rendering was also rerun against those native checkpoints. Waist MAE is **11.8607→4.8967** against the matched-2× frozen old implementation, versus 5.1275 in the earlier integrated report. Full residuals and all24 four-state montage sets are saved in [the combined retained-join report](../../liquid-glass-interaction/retained-joins/current-combined/metrics.json).

Reproduce the shared-body comparison with the existing ShadowRenderer and `Demo/GlassComparison/compare-shared-material.py MATRIX BEFORE AFTER OUTPUT`. Use native metadata and the same frozen before/candidate core archives identified by `provenance.json`. CPU cost is a second copied effect resolution per eligible member; the shared-size correction adds no shader samples or passes.
