# Glass foreground vibrancy and resting color

Glass now has an **opt-in foreground vibrancy** path. Native semantic tab and navigation accents in the Demo use it. Ordinary literal RGB content keeps its existing source-over behavior by default. The Demo's resting selection also uses a measured background transfer instead of the previous label-color alpha fill.

[UIKit / NXKit overlays](comparison.html) · [12-color native palette](palette.html) · [Full-tab scores](metrics.json) · [API pixel results](api-transfer.json)

![Native UIKit, previous NXKit, current NXKit](overview.png)

## Public references and what they establish

- [Apple's UIVibrancyEffect documentation](https://developer.apple.com/documentation/uikit/uivibrancyeffect) describes color-dependent foreground amplification. It does not publish the Liquid Glass foreground shader or a universal Add equation.
- [Apple Design Resources](https://developer.apple.com/design/resources/) links the [official iOS/iPadOS 27 Figma kit](https://www.figma.com/community/file/1651309003795292092/ios-and-ipados-27). The Community file could not be retrieved through the public page reader here; its internal layer/blend settings were **not** used as evidence.
- [liquid_glass_widgets](https://github.com/sdegenaar/liquid_glass_widgets/tree/59118973c0d01b154d35cfc6700b02ff038f7494), MIT: its `_VibrancyFill` is a translucent nested surface, while `glass_glow.dart` uses Plus for specular light. These are distinct from foreground color transfer.
- [flutter_liquid_glass](https://github.com/whynotmake-it/flutter_liquid_glass/tree/8e1e7d649bcc220312ba3d3493cfaf76a91382f2), Apache-2.0 at the inspected revision: its fake-surface highlight uses Plus, and its material shader has separate color/vibrancy controls. Its normal child content is painted above the material. These implementations are useful architectural references, not proof of UIKit's private equation.

No third-party shader code was copied for this change. The foreground and selection coefficients below come from independent public UIKit captures. The existing glass optics retain their earlier attribution.

## Native observations

The new fixture uses **public UIKit APIs only**: `UIGlassEffect`, `UIVisualEffectView`, `UIVibrancyEffect`, `UITabBar`, `UIColor`, `UILabel` and template `UIImageView`. Runtime: **iPadOS 27.0, build 24A434, 2× Simulator**, normal contrast, standard-range captures converted to sRGB.

1. Eighteen literal-color scenes cover both themes, neutral/colored backgrounds, half-alpha input and tinted glass. Twelve RGB swatches, labels and template rectangles retain literal color at full coverage inside regular and clear glass. Applying Add to every glass child would change content this fixture preserves.
2. Eight semantic-color scenes independently establish the newer system palette. For example, `systemBlue` resolves to **0,136,255** in light appearance and **0,145,255** in dark appearance. The old Demo used fixed 0,122,255. `SystemColorReference.m` explicitly resolves every color against each appearance; `system-colors.json` is the output. The portable eleven chromatic system colors are updated to these values; `systemGray` stays unchanged.
3. Twelve palette scenes contain twelve real native tab bars each: twelve semantic tints × six backgrounds × two themes = **144 foreground samples**. Their pixels differ from a flat RGB overlay. In dark appearance, foreground channels receive nonnegative contributions from the material beneath them; in light appearance, their responses are subtractive and channel-dependent. A plain Add operation does not explain both themes.
4. The same native scenes distinguish the resting selection from the surrounding glass. The previous dark Demo pill lightened the material with a white label-color overlay; native selection darkens and adjusts its color. This affected both the resting tint and the brightness fed into foreground vibrancy.

Raw cropped captures and acknowledged requests are retained in `native/{literal,semantic,tabs}` with their executable/source hashes. Each directory also retains the exact fixture source used for that series. The palette fixture's old `sourceRGB` field was resolved through the ambient trait context; fitting instead uses the explicitly resolved independent `system-colors.json` values. Current fixture code resolves through the bar's own trait collection for future captures.

## Reconstruction, not a private implementation claim

The foreground approximation fits a nonnegative 3×3 background transfer for each measured system tint and theme. Dark output adds the transferred background to source color; light output subtracts the transferred background complement. Both are bounded to the SDR range. Positive inverse-distance interpolation supplies a continuous approximation between measured tints. It is not evidence that Apple's private implementation uses those matrices or interpolation.

Green is excluded from fitting. The independent native green palette gives **1.073/255 mean RGB absolute error** for the mathematical transfer, with a worst color mean of 5.664/255. The real portable API pixel harness separately tests both foreground APIs at 2× for every sample and repeats blue at 1×/3×, for **168 renders**. Both API paths agree within 1/255; their 2× held-out color error is **1.007/255**. See `api-transfer.json` for errors by color and density; arbitrary hues, HDR, accessibility contrast and other vibrancy roles remain approximate/unverified.

The Demo selection fill has a separate affine background-color transfer per theme, leaving `LensView`'s caller-owned resting background unchanged. Its held-out green error is **1.452/255 light** and **0.620/255 dark**. This is control styling rather than a new default lens material. The fill follows traits, resizing and lift opacity.

## Portable behavior

- `UIGlassEffect::setContentVibrancyEnabled(bool)` opts a material's subviews into foreground blending. Effects remain copied; reinstall the changed effect. Only children are isolated, so the material itself is not color-blended twice. The default is false.
- `LensView::setContentVibrancyEnabled(bool)` independently opts its selected source into the same transfer. The default is false. Per-ray color treatment retains the existing red/green/blue coverage and refraction geometry, including during lift. External source ownership remains weak.
- Both support XML `contentVibrancyEnabled`, traits and presentation copies. Source opacity is composited once. This extension does not implement all of UIKit's `UIVibrancyEffect` styles or infer semantics automatically for arbitrary views.
- Semantic Demo tab accents enable vibrancy; literal custom accents disable it, including a literal color with the same RGB components as a system tint. Semantic navigation accents use the ordinary glass-child path; prominent white `+` content stays unchanged.
- Lifted lens source capture uses a bounded compatible surface aligned to device pixels, then runs its compositor in device coordinates. Resting vibrant source content now uses a direct foreground blender instead of that capture/filter pass (see `../liquid-glass-performance`). This avoids a second fractional filter on small text and bitmap edges while retaining the backend's GPU images. The normal source path continues using its direct picture shader. Lifted recording canvases without a compatible target retain normal selected content as a fallback; they do not reproduce vibrancy in that recording.

Use vibrancy for semantic/template foregrounds, not photographs or another glass surface. Stacking two opted-in foreground treatments will apply two treatments; callers own that composition.

## Full native-control comparison

Ten previously retained actual UIKit phone tab-bar cases cover both themes over neutral, blue, brown, coral and green. The stage crop and pose are fixed from native geometry; no image translation, scaling, fitted foreground colors or glyph registration is applied. The existing fixed-optics fixture separates color differences from motion. Both sides render at 2× with sRGB conversion.

The fixed selected-content rectangle is `[444,54,533,154]` in every 804×200 crop. It includes nearby material as well as glyphs, so it is not a text-only score. Absolute differences use 4× gain; matching-pixel removal replaces pixels within 2/255 per channel with gray. The `semantic` view uses the corrected source palette and resting fill with normal ink, separating those fixes from foreground blending.

| Mean RGB absolute error, 0–255 | Previous | Current |
| --- | ---: | ---: |
| Selected rectangle, resting | 29.783 | 7.264 |
| Selected rectangle, held | 23.583 | 14.875 |
| Entire crop, resting | 5.283 | 2.652 |
| Entire crop, held | 5.324 | 4.842 |

All ten selected-rectangle scores improve in both phases. Dark neutral resting selection now samples 10,10,10, matching the native probe; light neutral samples 230,230,235 versus native 229,228,234. The geometric and chromatic fields are unchanged. Remaining optical/contour differences, light halo, selected-background transmission and glyph rasterization differences remain visible. Full pixel parity is not claimed.

## Verification and reproduction

Focused tests cover native opaque colors, half-alpha coverage, opt-out restoration, copied effect/XML state, semantic versus literal Demo accents, resting-fill trait changes/fading, fractional Retina texture edges, and the existing 48 center, perimeter, white/tinted chromatic, routed-touch and full-screen regressions. The iOS Demo builds with code signing disabled. Physical-device and Switch/Dawn visual/performance verification remain outstanding.

```sh
python3 Demo/GlassComparison/build-glass-colors.py /tmp/public-glass-colors
python3 Demo/GlassComparison/capture-lens.py SIMULATOR_UDID \
  /tmp/public-glass-colors docs/liquid-glass-colors/tab-palette-cases.json

python3 Demo/GlassComparison/fit-glass-vibrancy.py NATIVE_DIRECTORY \
  docs/liquid-glass-colors/system-colors.json docs/liquid-glass-colors \
  --header UIKitCore/lib/GlassVibrancy.hpp
python3 Demo/GlassComparison/fit-tab-selection-fill.py NATIVE_DIRECTORY \
  docs/liquid-glass-colors \
  Demo/app/Screens/GlassDemoViewController/TabSelectionMaterial.hpp
python3 Demo/GlassComparison/compare-glass-color-transfer.py \
  /path/to/GlassColorRenderer docs/liquid-glass-colors
python3 Demo/GlassComparison/compare-glass-vibrancy.py \
  /path/to/TabPressRenderer /path/to/frozen-before-renderer \
  docs/liquid-glass-colors
python3 Demo/GlassComparison/build-glass-color-report.py docs/liquid-glass-colors
```

Use the retained literal/semantic request files for their corresponding capture series. Newer fixture code handles all three request families. The before executable is identified by hash; its scored before images are retained in this report. Native fixture sources and capture acknowledgements, fitted inputs, renderer outputs and source hashes allow the independent measurement to be inspected separately from the reconstruction.

Final source hashes and focused verification are in [`final-validation.json`](final-validation.json), separate from the scored capture binary. Its dark neutral held frame reproduces the report exactly; the resting re-render has a 0.026/255 mean difference at the selection silhouette (454 pixels exceed 2/255).
