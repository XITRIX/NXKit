# Retained glass joins: color-managed native comparison

Native UIKit 27.0, captured 2026-10-27. The 24 configurations cover neutral/blue and blue/coral, light/dark appearance, both creation orders, and gaps 2/4/12 points with container spacing 8. Every configuration captures initial, held, pulled and settled-after-release states: 96 optical checkpoints. The gap-2 fixture remains visibly joined after release; gap 12 supplies the press-only connection control. Two XCUITest batches completed successfully using public touch gestures.

The original native PNGs contain a Display P3 ICC profile. All figures and metrics here convert that embedded profile to sRGB through LittleCMS before comparison. Raster output is sRGB. Original captures are preserved, with hashes in [provenance.json](provenance.json). Earlier raw-channel comparisons incorrectly attributed much of this profile mismatch to resting tint; their color-error tables are superseded.

## Observations and correction

1. Native explicit blue owns the neutral/blue waist in either creation order. The initial/released central regions differ by less than 0.002 RGB MAE between orders. Two explicit tints retain painter-order ownership instead. NXKit previously allowed a front neutral item to overwrite blue at the join.
2. Native static material ownership has a smooth transition at its endpoints. Projecting native initial waist pixels onto local per-material body controls gives an approximate cubic-smoothstep handoff, with pooled fitted endpoints 9.4 and 1.3 points outside the owning item. The implementation uses rounded 9.5/1.5-point endpoints. This is an empirical material handoff, not a claim about UIKit's private shader. [Samples and fit](native-handoff.json) exclude held/pulled images.
3. A held native item's extruded bridge retains unlit tint outside that item's own geometric silhouette. Grouping had removed the ordinary view clip, allowing NXKit's white touch highlight to leak into the bridge. The correction applies the item's own antialiased SDF coverage to its interaction intensity. The shared union still clips the finished group.

4. When a pressed coral item is behind blue, the native blue waist also lights up where it overlaps the pressed coral silhouette. The feedback is therefore shared by the merged surface, clipped to active source silhouettes. The final renderer maps the active touch from its source through world coordinates into each owning material.

The renderer now resolves untinted ownership before explicit regular tint; equal-priority materials retain painter order. It shades every exact material in its original local geometry, mixes completed colors through the smooth ownership weights, and limits each touch highlight to its source item's silhouette. These container changes leave the merged SDF, zero contour, refraction geometry and resting material unchanged. No scalar waist darkening or theme-specific absorption was added.

## Aligned results

Both implementations receive the same measured native presentation rectangles through affine transforms. The adapter explicitly fixes effect layers to the native 2x scale, including for the older archive that did not inherit the display scale. A fixed stage-space waist region `(344,200)-(376,240)` is measured after masking glyphs. There is no fitted registration or image-color correction beyond ICC conversion.

The first before/after archive comparison isolates the first three join changes above, before shared feedback propagation was added; other core objects are identical. The frozen archive is from revision `ee91d4a99bbdbd9c578349386dc25e70becf41cf`, before the subsequent material/rim improvements.

Mean absolute RGB channel error, 0–255 scale, across both creation orders and all three gaps:

| Palette / appearance | Initial and settled, before → after | Held, before → after | Pulled, before → after |
| --- | ---: | ---: | ---: |
| Neutral/blue, light | 8.49 → 5.22 | 20.89 → 8.08 | 21.33 → 8.34 |
| Neutral/blue, dark | 5.18 → 3.01 | 16.43 → 5.27 | 17.78 → 6.12 |
| Blue/coral, light | 8.02 → 7.04 | 16.53 → 12.28 | 17.00 → 12.34 |
| Blue/coral, dark | 5.33 → 4.51 | 11.41 → 8.68 | 14.37 → 11.50 |

Overall waist MAE improves **11.861 → 7.009**. Every checkpoint improves or is unchanged; disconnected initial/release controls remain unchanged. Held and pulled images were not used to choose the static handoff curve. [Full metrics](metrics.json) include body controls and geometry timestamps.

Isolated before/after montages remain available for [neutral/blue light](join-nb-light-g2-reverse.png), [neutral/blue dark](join-nb-dark-g2-reverse.png), [blue/coral light](join-bc-light-g2-reverse.png) and [blue/coral dark](join-bc-dark-g2-reverse.png).

## Final material, rim and shared-feedback integration

The final comparison combines the container corrections with the independently measured [regular tint transfer](../../liquid-glass-optics/tint-model.md), neutral material and rim changes. It uses the same 96 native checkpoints, the same frozen old baseline, and the same explicit 2x filter backing resolution. This distinguishes the isolated join improvement above from the complete final rendering result.

| Palette / appearance | Initial and settled, before → final | Held, before → final | Pulled, before → final |
| --- | ---: | ---: | ---: |
| Neutral/blue, light | 8.49 → 3.85 | 20.89 → 8.09 | 21.33 → 8.44 |
| Neutral/blue, dark | 5.18 → 2.28 | 16.43 → 4.94 | 17.78 → 4.83 |
| Blue/coral, light | 8.02 → 5.39 | 16.53 → 8.62 | 17.00 → 8.39 |
| Blue/coral, dark | 5.33 → 3.33 | 11.41 → 4.69 | 14.37 → 4.36 |

Overall waist MAE is **11.861 → 5.128**. Before shared feedback propagation, the final material/rim integration scored 5.836; the shared-feedback correction improves it to 5.128 without worsening any checkpoint, and every neutral/blue checkpoint is numerically unchanged by that last step. The strongest correction is behind a front blue material: dark blue/coral reversed gap-2 held improves 13.86 → 5.01, and pulled 16.10 → 5.26.

One final checkpoint is slightly worse than the frozen old baseline: dark blue/coral reversed gap-4 held, 8.868 → 9.119 MAE. The shared-feedback step improves that state from 9.450 → 9.119; no outlier-specific fitting was added. The complete result therefore improves broad measured behavior but is not uniformly closer at every pixel or checkpoint.

[Final metrics](metrics-integrated.json), [before-shared-feedback metrics](metrics-before-shared-lighting.json) and [integration provenance](integrated-provenance.json) retain the full comparison. The following columns show native / old baseline / final NXKit / absolute RGB difference ×4; rows show initial / held / pulled / released. Native images are displayed after ICC conversion.

The final shared core was subsequently relinked after the neutral retained-chroma update. All 16 representative images (both palettes/themes, reversed order, initial/held/pulled/released) are byte-identical to their recorded final-96 images. [Final archive validation](final-archive-validation.json) records the archive and image hashes. This confirms the later neutral change preserves these gray-background checkpoints; it does not replace the separate nonuniform-background validation.

![Final neutral/blue light, reversed creation order](integrated-join-nb-light-g2-reverse.png)

![Final neutral/blue dark, reversed creation order](integrated-join-nb-dark-g2-reverse.png)

![Final blue/coral light, reversed creation order](integrated-join-bc-light-g2-reverse.png)

![Final blue/coral dark, reversed creation order](integrated-join-bc-dark-g2-reverse.png)

## Regression and limits

Focused container tests check exact categorical material shading through the smooth handoff in both insertion orders/appearances, an unlit bridge while the pressed body highlights, rear-item feedback illuminating an overlapping front material, restoration of a retained join after release, single-item equivalence and affine press coordinates. Backing-scale tests cover trait inheritance, active presentation layers, invalid scale fallback, reparenting and preserving explicit overrides during same-screen notifications/layout.

Shader cost remains one geometry field evaluation normally and five where the union blends. Ownership adds a second inexpensive arithmetic pass over items; it adds no texture samples. The cubic weight and own-item highlight coverage use existing distances. Shared feedback adds an arithmetic source-selection pass and touch-coordinate transforms, with no extra texture samples. Materials are still shaded only for nonzero ownership. GPU timing is not established by the raster comparison.

These gray-background, 52-point two-item fixtures demonstrate broad improvement, not pixel-identical UIKit rendering. Blue/coral held/pulled residuals, native internal reflection/rim detail, partially transparent tint ownership, clear-glass priority, multiple simultaneous interaction sources, passive receiving materials, artwork-dependent refraction, three-item topology/order behavior and general rotated/sheared blur covariance remain outside this fit. Motion is bypassed here and is validated separately by the [native replay report](../README.md).

## Reproduce

Generate and run the native UIKit fixture with [cases.json](cases.json):

```sh
python3 Demo/GlassComparison/build-interaction.py NATIVE_BUILD docs/liquid-glass-interaction/retained-joins/cases.json
xcodebuild -project NATIVE_BUILD/Interaction.xcodeproj -scheme Interaction \
  -destination 'platform=iOS Simulator,id=DEVICE' -derivedDataPath NATIVE_BUILD/build test
```

Copy the app's `Documents/join-*` directories without changing the PNG profiles. Build optical adapters from an existing raster Ninja build, then measure. The helper writes into its output directory and can use a frozen archive with `--core-library`.

```sh
python3 Demo/GlassComparison/build-merge-checkpoints.py RASTER_BUILD CHECKPOINT_BUILD \
  --baseline-ref ee91d4a99bbdbd9c578349386dc25e70becf41cf \
  --candidate-container UIKitCore/lib/CAGlassContainerLayer.cpp
python3 Demo/GlassComparison/compare-merge-matrix.py CAPTURES BEFORE CHECKPOINT_BUILD/MergeCheckpointBefore
python3 Demo/GlassComparison/compare-merge-matrix.py CAPTURES AFTER CHECKPOINT_BUILD/MergeCheckpointAfter BEFORE
python3 Demo/GlassComparison/analyze-merge-handoff.py CAPTURES HANDOFF.json
```

Pillow with ImageCms and NumPy are required. Passing `-` as the renderer recomputes metrics and montages from already rendered rasters.

By default both adapters share all non-container objects, isolating the container source change. To reproduce the full material integration, additionally pass `--baseline-core-library FROZEN_OLD_CORE_ARCHIVE --core-library FINAL_CORE_ARCHIVE`; preserve the exact archives identified by the provenance hashes. The baseline container is still compiled from the specified revision. This comparison uses separate before/after material objects and should be labeled integrated rather than container-only.

## 28 October combined follow-up

The later material and shadow corrections were rerun across all96 checkpoints. Waist MAE is now **11.8607→4.8967** against the same matched-2× frozen baseline. The separate mixed-size neutral-container correction leaves all96 full rasters byte-identical. [Current combined metrics, full residual montages and validation hashes](current-combined/README.md) supplement the earlier isolated/integrated results above.
