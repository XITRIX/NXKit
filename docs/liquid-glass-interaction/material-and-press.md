# Container optics and press lighting

**Color-profile correction, 2026-10-27:** the historical figures and numeric tables below compared native Display P3 channel values directly with untagged sRGB raster output. They are retained as the original experiment record, not current color-error evidence. In particular, the large resting-blue discrepancy reported below is mostly a profile mismatch and must not guide shader fitting. New comparison tools convert the embedded native ICC profile to sRGB before measuring or displaying paired pixels; originals remain untouched. The [expanded retained-join report](retained-joins/README.md) supersedes these color metrics.

Verified against native UIKit 27.0 on 2026-10-27. These comparisons complement the [touch-motion replay report](README.md). They do not establish pixel-identical glass.

## Container correction

The old container interpolated every shader uniform before shading. Mixing untinted regular glass (`tintMode=0`) with explicit regular tint (`tintMode=2`) crossed discrete shader branches at intermediate values. That created artificial crescent bands. It also mixed item centres, dimensions and touch locations into a fictitious optical rectangle.

The corrected renderer shades each participating material with its own categorical mode and local geometry, then blends the finished colors. Refraction and touch-light samples map through that item's affine transform. Single-item containers use the ordinary renderer, preserving its output exactly.

The existing smooth-union field and its zero contour are retained. Where shapes blend, four neighbouring field evaluations recover the gradient, including the changing blend width. Dividing the field by its gradient magnitude supplies a local physical distance for the bevel and edge coverage. Normalizing only the old direction had left the narrow neck with an excessively wide optical rim. Medial-axis denominators are bounded; this is a local distance correction rather than an exact global distance solve.

![UIKit, previous NXKit and corrected NXKit with measured native geometry](group-native-before-after.png)

The fixture contains neutral and blue 52-point circles, a 12-point gap and container spacing 8. It captures held, pulled and release states in both appearances over gray. The NXKit checkpoint adapter injects the measured native presentation rectangles, deliberately bypassing touch dynamics. No fitted image alignment or color normalization is applied.

Mean absolute RGB error on the fixed seam region, on a 0–255 channel scale:

| Checkpoints | Previous | Current |
| --- | ---: | ---: |
| All 30 | 16.55 | 14.95 |
| Held | 19.30 | 11.84 |
| Pulled | 20.99 | 13.50 |
| Settled | 7.38 | 7.25 |

[Full group metrics](group-material-metrics.json) include native geometry and sampling-time offsets. [Container-only metrics](group-container-only-metrics.json) isolate the geometry/material-mode correction before press-light calibration: overall seam error changes 16.55 → 16.19. The additional improvement above includes the measured lighting changes below.

The original paired fixture supplies dynamic `systemBlueColor`; its first raster adapter supplies explicit RGB(0,122,255). This affects absolute color error. The separate fixed-input lighting fixture below removes that ambiguity. Native join shading remains darker and more detailed than the current handoff, even after the duplicated bands are removed.

## Isolated join-shading check

A follow-up diagnostic maps each renderer's local neutral/blue body controls to its corresponding native controls before measuring only the waist. This removes resting/pressed body-color bias for analysis; the mapping is never applied to the figures or counted as renderer parity.

A separate experimental binary retained the union contour/coverage and used the nearer own-item optical boundary, `max(unionDistance, itemDistance)`, only where geometry blends. It kept the merged normal, isolating the distance hypothesis. This **worsened all six held/pulled checkpoints** in both raw and body-controlled error:

| Body-controlled waist MAE | Current | Local-distance experiment |
| --- | ---: | ---: |
| Light held, pull case | 16.31 | 16.81 |
| Light pulled | 20.74 | 22.00 |
| Dark held, pull case | 17.98 | 18.76 |
| Dark pulled | 19.69 | 21.34 |

A uniform 10% waist attenuation was also inconsistent: light held error increased 16.31 → 19.08, whereas dark held improved 17.98 → 12.70. Least-squares scalar fits varied roughly 6–11% across states and appearances. No extra absorption or local-distance change was adopted. This negative check does not rule out a fuller model of native internal normals/reflections; it rules out the tested distance-only substitution and a universal small darkening from this evidence.

[Diagnostic data](join-absorption-diagnostic.json) and [analysis script](../../Demo/GlassComparison/analyze-join-absorption.py) preserve the method and results. All fresh paired captures put blue glass in front. Their settled state is disconnected, so it supplies no evidence about a settled join. Earlier [two-tint comparisons](../liquid-glass-gallery/README.md#two-different-tints) cover both orders, but do not establish the missing neutral/blue cases here. These limits prevent a safe general fit from this waist sample alone.

## Press lighting with matched inputs

Four independent standalone cases use regular glass with either no tint or explicit RGB(0,122,255), in light and dark appearance. Sixteen equally weighted body samples use local diagonal offsets of 8 or 10 points from the centre, avoiding glyphs, the seam and the outer rim. Native held and settled samples use their measured presentation rectangles.

White-equivalent opacity is the least-squares projection of held-minus-settled RGB onto the available white headroom. It describes the observed response; it does not identify UIKit's private shader implementation. The overlay-only error applies each rendered opacity to the **native settled color**, keeping the separate resting-material error out of the press fit.

![Matched fixed-input native and NXKit press-light comparisons](press-light-native-before-after.png)

| Regular material | Native opacity | Previous | Current | Overlay-only MAE, previous → current |
| --- | ---: | ---: | ---: | ---: |
| Neutral, light | 0.5015 | 0.2723 | 0.5000 | 13.99 → 0.11 |
| Neutral, dark | 0.4392 | 0.2104 | 0.4390 | 27.85 → 0.10 |
| Blue, light | 0.3707 | 0.2778 | 0.3610 | 17.06 → 10.78 |
| Blue, dark | 0.3614 | 0.2118 | 0.3481 | 18.71 → 6.41 |

Regular neutral glass now uses an ambient white-opacity interpolation of 0.385–0.47 through the existing appearance parameter, plus its existing touch-local sheen. Explicit regular tint uses 0.30 ambient plus that sheen. Clear glass retains its previous response; these regular-glass captures do not calibrate clear interaction lighting.

Absolute held-body error improves 10.33 → 2.42 for neutral light, 28.85 → 0.73 for neutral dark, 24.48 → 12.73 for blue light and 30.42 → 13.75 for blue dark. [Machine-readable lighting metrics](material-press-metrics.json) retain both held and settled RGB values.

Resting blue-glass error remains **19.83 in light and 18.33 in dark**, despite matching the supplied tint. Native tinted changes are not exactly a white mix, and changing the press overlay does not fix the resting material. Tint optics and internal seam shading therefore remain partial. No resting-tint adjustment was made from these interaction captures.

The runtime resolves dynamic `systemBlueColor` to RGB(0,136,255) in light appearance and RGB(0,145,255) in dark. The renderer accepts the logged resolved color when available; fixed-tint captures explicitly use RGB(0,122,255) on both sides.

## Regression coverage and reproduction

`ContainerTests` checks continuous untinted/blue material handoff in both orders and appearances, exact single-item equivalence at 1×/2×/3×, affine press transforms with off-centre lighting, fractional neck coverage, regular neutral/tinted response, and unchanged clear response. Existing containment, clipping, presentation geometry, reparenting and ownership tests continue to pass.

Use an up-to-date Ninja raster build configured as described in the [container harness instructions](../liquid-glass-container/README.md#reproduce). The helper writes only its output directory and reuses that build's compiler/linker flags:

```sh
python3 Demo/GlassComparison/build-merge-checkpoints.py RASTER_BUILD CHECKPOINT_BUILD \
  --baseline-press --baseline-ref a25895be95f9ff3f84b3708c1eb83033870fcade
python3 Demo/GlassComparison/compare-merge-checkpoints.py PAIRED_CAPTURES GROUP_REPORT \
  CHECKPOINT_BUILD/MergeCheckpointBefore CHECKPOINT_BUILD/MergeCheckpointAfter
python3 Demo/GlassComparison/measure-press-light.py FIXED_LIGHTING_CAPTURES LIGHTING_BEFORE \
  CHECKPOINT_BUILD/MergeCheckpointBefore
python3 Demo/GlassComparison/measure-press-light.py FIXED_LIGHTING_CAPTURES LIGHTING_AFTER \
  CHECKPOINT_BUILD/MergeCheckpointAfter
```

The comparison scripts require Pillow and NumPy. The native sources are [InteractionReference.m](../../Demo/GlassComparison/InteractionReference.m) and [InteractionUITests.m](../../Demo/GlassComparison/InteractionUITests.m). Build provenance records the baseline revision and shader-source hashes.

Only stable held/settled snapshots determine the lighting fit. A nearest logged presentation sample may differ from image capture by one display frame; transient release comparisons retain that uncertainty. Gray-underlay captures do not prove artwork-refraction parity. General rotated anisotropic/sheared blur covariance, three-item smooth-union order dependence, and end-to-end Metal/Dawn performance remain outside these optical checks. Shader cost is one geometry evaluation normally, five where the field blends, and material shading only for nonzero owners.
