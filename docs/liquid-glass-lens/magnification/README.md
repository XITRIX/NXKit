# Lifted lens content magnification

The tab bar now enlarges each selected-content item to **1.16×** while lifted and animates back to unit scale on release. Every item has its own pivot, including the two items visible when the lens is between tabs. The platter's independent 1.039272× press growth and the phone lens's 95×54 → 111×70 bounds remain separate.

[Aligned overlays and matching-pixel removal](comparison.html) · [Measurements](metrics.json) · [Standalone native source measurements](standalone-measurements.json) · [Provenance](provenance.json)

![UIKit, previous NXKit, current NXKit](overview.png)

## Native observations

These measurements describe UIKit **27.0, build 24A434, in Simulator**. They are observations rather than a documented Apple magnification contract.

- **N1 — actual phone UITabBar:** ten independent retained light/dark holds over neutral, blue, brown, coral and green backgrounds. The selected-content `_UITabButton` layers, paths `stage.1.0.0.0.{0,1,2,3}`, all change their presentation transform from 1 to **1.16**, then return to 1. Ordinary item transforms stay at 1. The enclosing platter separately scales to 1.039272. Trace hashes and exact scales are retained in `provenance.json` and `metrics.json`; original captures/traces are in `../tab-press/native`.
- **N2 — compact UITabBar:** `../native/motion/motion-tabs-snapshots/trace.json.gz` independently shows the three selected-content items reaching **1.16**, with ordinary items staying at 1. The observed compact selection has 99.5×36 resting bounds. This supports the same enlargement in the compact Demo layout.
- **N3 — freshly instantiated standalone private lens:** four scored captures cover both themes and regular/selection styles at 116×56 with `liftedContentMode=1`. Four saturated markers away from the rim retain their requested 36pt horizontal and 12pt vertical spacing: measured scale differs from 1 by less than .001. See `native-standalone`, compressed metadata, `standalone-cases.json` and `standalone-measurements.json`. The separate source-only control requests are retained for reproduction, but are not used to fit the zoom. A manually supplied selected source is not automatically enlarged by this native configuration.

**Inference:** the missing tab-bar enlargement belongs to the selected source items. Scaling the entire row about the moving lens center would displace adjacent items when dragging between tabs. Applying both source-item enlargement and a global optical enlargement would count it twice. This change reproduces the observed source ownership instead.

## Implementation

`LensTabBarView` groups each selected icon and caption under a separate source item. Lift/release animates a 1.16× transform about the measured item center. The pivot translation is exactly zero at rest; no fractional anchor division is involved. Resize updates the pivots, and dragging leaves the selected source stationary beneath the moving lens.

The portable `LensView` extension additionally exposes `liftedContentMagnification`, finite and at least 1, including XML support. It uniformly samples its selected source about the lens center and interpolates the enlargement with lift progress. The magnified original and refracted samples share one map, avoiding overlapping enlarged and original-size glyphs during partial lift. Source footprint width is adjusted for the zoom. The default stays 1 to preserve N3; standalone Lens demo samples explicitly choose 1.16. The tab bar uses per-item source transforms and leaves this optical multiplier at 1. The ordinary backdrop remains governed by its independent refraction field.

## Pixel comparison

Both sides render at 2× with sRGB color conversion. Native crops are inherited from stage/window geometry. Demo holds use the existing `--fixed-optics` fixture to settle the platter and fix lens placement; no image translation, image scaling, color fitting, or glyph fitting is performed. Both old and new Demo captures use the same pose. These fixed holds test source enlargement, not complete touch-latency or motion parity.

The selected-content score uses the same fixed pixel rectangle `[444,54,533,154]` in every 804×200 crop. It includes the glyph, caption and nearby material; it is not a segmented text-only score. Absolute differences are shown at 4× gain. Matching-pixel removal replaces pixels whose maximum channel difference is at most 2/255 with gray.

| Mean RGB absolute error, 0–255 | Previous | Current |
| --- | ---: | ---: |
| Selected-content rectangle, ten held scenes | 36.406 | 23.583 |
| Entire held crop, ten scenes | 6.034 | 5.324 |

The selected-source symbol dimensions now agree with native presentation dimensions within **0.001pt**, separately from their position/material errors. All ten resting before/after images are **pixel-identical**. Nine selected-content scores improve; dark-blue increases from 38.788 to 39.762/255. Remaining selected vibrancy, chromatic footprint, dark contour, light rim and native hold placement differences are deliberately visible in the overlays. This is not a claim of full optical parity.

## Reproduction and regression coverage

```sh
python3 Demo/GlassComparison/compare-lens-magnification.py \
  /path/to/TabPressRenderer /path/to/frozen-before-TabPressRenderer \
  docs/liquid-glass-lens/magnification

python3 Demo/GlassComparison/build-lens.py /tmp/lens-source-oracle
python3 Demo/GlassComparison/capture-lens.py SIMULATOR_UDID \
  /tmp/lens-source-oracle docs/liquid-glass-lens/magnification/standalone-cases.json
python3 Demo/GlassComparison/measure-lens-source-scale.py \
  /tmp/lens-source-oracle/native docs/liquid-glass-lens/magnification
```

The frozen executable is identified by its SHA-256 in provenance. It is a local before-change baseline; native inputs and repeatable capture/measurement code are retained in the repository.

Verification completed:

- `LensViewTests`: defaults, invalid/nonfinite input, XML and existing source/lifecycle/presentation behavior.
- `LensRefractionTests`: twelve partial/full zoom comparisons against independently transformed ordinary source content in both styles/themes; all existing center, perimeter and white/tinted chromatic regressions pass, including the 48 unit-scale center texture cases.
- `LensTabBarTests`: all selected items scale 1.16 about the native pivots; between-item dragging, resize, animated lift, interrupted release and exact rest are covered alongside routed-touch regressions.
- `GlassScreenTests`: full-screen routed touch and scroll/chrome regressions pass.
- iOS `ThorVGApp` Debug build succeeds with code signing disabled.

Exact native enlargement timing is not newly fitted here; the existing Demo lift/layout animation drives the item transforms. Physical iOS 27.2, other Dynamic Type sizes and Metal/Dawn pixel/performance parity remain unverified. Production uses no private UIKit APIs; they stay in the independent comparison fixture.
