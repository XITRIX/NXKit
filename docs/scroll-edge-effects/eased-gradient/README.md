# Eased ScrollPocket gradients — 2026-11-04

The supplied red-background Demo screenshot exposed a concentrated fade band. The old attenuation profile stayed flat through 45% of the pocket, recovered rapidly through linear segments, and reached full clarity at 86%. Its slope changed abruptly at each stop and at that early finish.

The new content recovery is quintic smootherstep over the interval from 25% to 100% of the **existing live pocket extent**:

```
u = clamp((distance / extent - 0.25) / 0.75, 0, 1)
recovery = 6u^5 - 15u^4 + 10u^3
opacity = 0.4 + 0.6 * recovery   // fully active pocket
```

This broadens recovery and flattens its ends. The ideal curve's maximum normalized opacity slope drops from 3.0 to 1.5, removing the narrow high-slope band. We bake 33 samples into the existing cached Skia gradient rather than evaluating a new custom shader per pixel. The rendered mask regression checks both top/bottom for excessive adjacent-row contrast change and recovery continuing gently through the inner tail. All four assertions fail on the previous renderer and pass after easing.

Blur remains a true spatially varying Gaussian kernel. Its separate existing anchors are joined with monotone cubic Hermite interpolation and flat endpoint tangents. This removes abrupt kernel-slope changes while preserving the established title-band widths and **4pt outer maximum**. No additional blur/compositing pass, content classification, readback or replay is added. Hard effects and NXKit materials are unchanged. Pocket geometry still comes from live safe areas and associated containers; no extension or reference-device bar height was added.

## iOS 26 comparison

We compared the retained native **iOS 26.0 release (23A343)** reference against freshly rendered matching before/after portable profiles: dense/sparse, light/dark, top/bottom. The native app, runtime metadata and capture provenance are under [the native reference](../native/README.md). This task reused those captures; it did not capture a new native session.

![Dark: native, before, eased](images/dark-comparison.png)

![Light: native, before, eased](images/light-comparison.png)

Eight portable state/geometry traces are identical before/after. `images/comparison.json` records source image hashes. `compare.py` recreates the sheets from native, before, after and output directories.

This addresses the fade's sharpness, not full optical parity. Native dense-content optics adapt to content/traits, while our constant policy is intentionally retained at the user's request. Native carriers in the reference can extend farther inward; our unchanged live safe/container bounds differ. Independent glyphs, glass chrome and colors also differ. A softer boundary should not be interpreted as recovery of Apple's private curve. Apple's public [soft edge style](https://developer.apple.com/documentation/uikit/uiscrolledgeffect/style-swift.class?language=objc) supplies the API contract, not the interpolation formula.

## Validation

The new band regression reproduces four failures before the fix. Afterward, **191 behavioral checks plus six optional image-encoding checks pass**. They include existing mounted title-band impulse-width checks in both themes at 1×/2×/3×, the four-point maximum, true narrowing kernels, all directional filters, sparse transparency, live safe-area/container changes and theme/scale changes. The same six mounted text-over-title pictures are under `images/title/`.

GPU and build verification, source hashes and any paired performance measurements are recorded in `verification.json`. Shader changes require a new ABI and fresh offline bake. The full cold/strict matrix covers six immutable scale/theme profiles and two navigation push/pop smoke profiles, 7,310 frames per sweep. Strict runs require archive hits and forbid runtime library/pipeline compilation. Raw sources are retained in the content-addressed shader catalog; no old binary is relabelled.

The visual test is reproducible with `ScrollEdgeEffectsTests` and the retained public fixture schedules under `../verification`. Set `NXKIT_SCROLL_POCKET_PICTURES` to export mounted title images. `run-metal.py cold` / `strict` reproduce the local Metal matrix using the explicit build directories in that script. Exact native optics, physical phone energy/thermal behavior and Switch rendering remain unverified.

Both macOS and unsigned iOS Debug Demo builds pass and embed the matching new offline bundle (`0d289a6b…`). All **7,310 strict Metal frames pass with zero runtime library/pipeline compilation**. The catalog contains 121 raw library variants and 102 pipeline descriptions, including explicitly retained prior coverage; the fresh coverage is validated before merging.

Three alternating-order paired runs per profile show no total completion-time regression in the measured four-pocket scrolling fixture. At dark 3×, warm combined CPU median is 1.460→1.494 ms (+2.4%) and total through completion is 4.804→4.477 ms. At light 2×, CPU is 1.334→1.300 ms (−2.5%) and total is 3.529→3.414 ms. These small host-dependent differences are observations, not an optimization or phone-power guarantee. Both versions forbid runtime compilation; first passes are excluded and each version/profile contributes 84 selected scrolling frames. Raw measurements and limits are in `performance.json` and `performance/`.
