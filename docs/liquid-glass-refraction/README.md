# Regular glass refraction: the next comparison

[Open the aligned pixel viewer](comparison.html). Both themes, three tint choices and five sizes use the same native geometry and source pixels. The viewer overlays UIKit/NXKit, shows absolute RGB differences, and removes matching pixels exactly or within an explicitly stated 2/255 tolerance. Images are never translated, recolored or exposure fitted to improve the result.

The baseline is NXKit revision `9345af9631a4e01188c1862bf1a9a31d5efc9cbe`, already rendering at the correct 2× scale. The material scores isolate the subsequent material changes; full images also retain the integrated default shadow changes, [scored separately](../liquid-glass-shadows/README.md) from these interior masks. The [earlier optics report](../liquid-glass-optics/README.md) retains the original scale/material investigation.

## Changes and observations

The regular displacement uses a monotonic quintic envelope, `T(u) = (1−3u²+2u³)(1−u²)`, where `u` runs from the silhouette to the flat face. Both endpoint slopes are zero. It reduces the deep part of the fold while keeping its outer shoulder continuous. Clear glass keeps its prior transfer.

Light regular glass also filters chroma over a wider neighborhood while preserving transmitted luminance and coverage. A positive five-tap stencil uses half the center and one eighth of each cardinal neighbor; its frequency response cannot reverse the phase of a color wave. Constants remain constants, and linear ramps remain linear apart from raster quantization and image boundaries. The existing sample-radius guard bounds every child evaluation. The actual resolved frost bounds the additional kernels; zero frost disables them.

The neutral detail lane previously injected a fully sharp stripe into the broader frost. On the native 120-point circle the white line is smooth; NXKit instead showed a narrow needle. A small separable tent on that detail lane softens larger controls without changing its constant-color response. Small controls retain their sharper response.

These are measured approximations, not an identification of Apple's private filters or a claim of pixel parity. The two-lane filter interpretation is an inference from the controls.

## Coverage and results

The main matrix has 60 scenes / 300 glass items: light and dark, neutral/blue/coral tint, and black/white/gray/red/green/blue/yellow/ramp/lines/colored-wave backgrounds. Each scene contains 52, 80 and 120-point circles, a 132 × 56 pill, and a 240 × 160 continuous-corner card.

Six [pastel controls](pastels/comparison.html) retain three partial-color backgrounds in both themes. Four [independent color-wave controls](holdouts/comparison.html), captured after choosing the initial kernel, change the wave period from 120 points to 60 and 240. They prevented accepting a dark chroma-filter change that improved the original wave but worsened the new frequencies. Production applies the extra chroma filtering to the light regular material only.

| Controls | Body before → after | Rim before → after | Optical band before → after |
| --- | ---: | ---: | ---: |
| Main 60 scenes | 1.305 → 1.163 | 3.065 → 2.944 | 1.850 → 1.671 |
| Colored 120pt wave (all tints) | 4.456 → 3.072 | 7.943 → 6.744 | 5.899 → 4.244 |
| Independent 60/240pt waves | 6.623 → 5.588 | 14.882 → 11.857 | 10.105 → 7.801 |
| Pastel controls | 1.402 → 1.402 | 2.223 → 2.223 | 2.251 → 2.251 |


Errors are mean absolute sRGB channel differences on a 0–255 scale, equally weighted over each item and scene. Body masks exclude the inner edge band; rim masks cover 0.75–2.75 points inside; optical-band masks cover 3–12 points inside. Conservative straight portions exclude the card's corner-curve approximation. **These scores exclude exterior shadows.** Full images and residuals still show them.

Remaining differences are visible. In particular, the 80-point light narrow-line control trades some optical-band fidelity for the smoother general shoulder. The entire image is not pixel matched, and the new wave controls still have substantial residual color error. Real-device iOS 27.2, exact native continuous corners, and production Metal/Dawn performance remain unverified by this matrix.

## Reproduce

```sh
python3 Demo/GlassComparison/prepare-optical.py RUN
python3 Demo/GlassComparison/build-optical.py RASTER_BUILD RUN
python3 Demo/GlassComparison/capture-optical.py DEVICE RUN
python3 Demo/GlassComparison/compare-optical.py RUN AFTER --render=RUN/OpticalRenderer --baseline=BEFORE
```

Capture the independent frequencies into their own run folder using `capture-optical.py DEVICE HOLDOUT_RUN waves60 waves240 --neutral-only`. Capture pastel controls using `mint lavender peach --neutral-only`. Build each native app with that run's generated background assets. Preserve the old renderer and its outputs before editing the core; a rebuilt old source is not an interchangeable baseline when dependencies changed.

`check-optical-regression.py MAIN_METRICS HOLDOUT_METRICS PASTEL_METRICS` validates completeness, uniform/pastel stability, aggregate material improvement, and each independent frequency/theme control. `check-optical-locality.py RENDERER OUTPUT` exercises the production retained tree with equal local pixels and different surrounding colors: zero frost must keep the centers equal, while positive frost must distinguish them, across both themes, all tints and all five sizes. The retained [metrics](metrics.json), [capture metadata](native-metadata.json), [build provenance](provenance.json), and [exploratory controls](experiments.json) distinguish the final matrix from isolated experiments.

## References and limits

The primary implementation reference remains [liquid_glass_widgets' rendering shader](https://github.com/sdegenaar/liquid_glass_widgets/blob/5d2859796ef0f6964e2fdf25962b033eede27814/shaders/liquid_glass_render.frag), under its existing MIT attribution. This iteration adds independently implemented polynomial/filter changes; it does not import new third-party source.

[Kube's optics derivation](https://kube.io/blog/liquid-glass-css-svg/) and [anuero's actual Metal shader](https://github.com/anuero/LiquidGlass/blob/main/Sources/LiquidGlass/Shaders.metal) supply first-principles Snell/height-profile controls. [LiquidGlassTabBar's shader](https://github.com/IchimTeodor/LiquidGlassTabBar/blob/main/Sources/LiquidGlassTabBar/TabBar/Shaders.metal) provides a useful contrasting fold-free model. Our direct sphere/squircle and fold-free controls reduce some colored/ramp errors but worsen narrow-line rim matching; they were not substituted wholesale for the measured transfer.

The [independent iOS 27 calibration study](https://github.com/shayann07/liquidglass/blob/main/docs/research/ios27-calibration-study.md) describes frequency and color probes and distinguishes capture errors from material behavior. Its measurements and claims are not substituted for our own framebuffer evidence or treated as Apple's specification.

NXKit images use the production retained tree on Skia’s raster backend, with the same material shader and resolved parameters. They do not establish numerical equivalence on Graphite Metal or Dawn.

Native controls here are iPadOS 27.0, build 24A434. Original simulator PNGs declare sRGB; cropped controls retain that color space. Older hierarchy snapshots in other investigations embed Display P3 and require conversion before subtraction. Uncovered wave pixels in all four new captures match the supplied sRGB source **byte for byte**. The report's source-image checks record that observation separately from any glass fit.
