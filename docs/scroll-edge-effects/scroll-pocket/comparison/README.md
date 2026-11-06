# Scroll-edge comparison with iOS 26

This is the requested fixed ScrollPocket appearance. Native iOS 26 is the API
and geometry oracle; its content-adaptive optics are intentionally not matched.
See [the fixed policy and before/after report](../README.md).

Compared 49 matching static cases from 49 native and 49 portable trace entries. The native oracle is iOS 26.0 release 23A343; see the [native audit](../../native/README.md).

[Light comparison](light-comparison.png), [dark comparison](dark-comparison.png), [measurements and trace hashes](comparison.json), [portable state trace](portable-states.json.gz). Native UIKit is on the left of every pair; portable is on the right. Individual pairs are under `pairs/`.

## Scope and interpretation

These are measurements of a reproducible fixture, not a claim of pixel or behavioral parity. The primary image region excludes the OS shell; independent glyph rendering, tab/navigation glass and semantic colors remain in broad image differences. Per-row soft/hidden fits exclude title, trailing button and tab-glass samples where practical. They describe combined blur, tint and mask behavior and must not be read as the native alpha mask or Gaussian sigma.

Carrier geometry: 76 scored bands; coordinate RMS 0.10002740852456846 points; maximum coordinate difference 0.5000000000000568 points. Portable frame provenance: renderer snapshot extent. Frames derived from renderer snapshot extents are explicitly labelled; geometry scores do not independently verify raster output. Empty inactive portable bands are excluded, since UIKit retains dormant carrier objects that have no corresponding active rendered band.

Missing portable trace cases: none. Extra portable cases: none. Partial-input mode: False.

## Optical profiles

The table records the red-channel slope and intercept of `soft ≈ slope × hidden + intercept` at selected rows. Full RGB fits and per-row differences are in the JSON. A high positive intercept usually indicates a light wash, while a reduced slope with near-zero intercept indicates darkening; blur and color mixing also affect both.

| Trait | y (pt) | Native red slope / intercept | Portable red slope / intercept |
|---|---:|---:|---:|
| light | 20 | 0.735 / 2.44 | 0.295 / 165.77 |
| light | 116 | 0.921 / 0.99 | 0.749 / 60.30 |
| light | 126 | 0.969 / -0.07 | 0.908 / 21.77 |
| light | 156 | 1.000 / 0.00 | 1.000 / 0.00 |
| light | 860 | 0.752 / -0.10 | 0.376 / 154.29 |
| dark | 20 | 0.396 / 0.00 | 0.302 / 14.45 |
| dark | 116 | 0.823 / -0.04 | 0.749 / 14.20 |
| dark | 126 | 0.928 / -0.40 | 0.907 / 2.96 |
| dark | 156 | 1.000 / 0.00 | 1.000 / 0.00 |
| dark | 860 | 0.424 / 0.02 | 0.368 / 7.17 |

## Private variable-blur probes

| Mask | Mean absolute RGB difference in effect ROI |
|---|---:|
| mask-zero | 3.769 |
| mask-one | 6.806 |
| mask-ramp | 7.425 |
| mask-nil | 3.028 |

Transparent/opaque/ramp masks exercise the real variable-blur renderer; nil is an argument-rejection case. Optical measurements include source raster differences and do not prove a universal private radius/kernel mapping.

## Transparent content

[Sparse light/dark comparison](sparse/sparse-comparison.png), [gap and colored-cell measurements](sparse/sparse-measurements.json). The sparse fixture skips alternate tile fills/text and keeps the drawing background clear; the screen background stays systemBackground.

Maximum per-row mean gap soft-versus-hidden RGB difference: {'native': {'light': 3.0, 'dark': 0.0}, 'portable': {'light': 17.166666666666668, 'dark': 23.333333333333336}}. These are gap-preservation checks, not whole-scene equivalence scores.

## Remaining differences

The paired sheets still show differences in adaptive bar/title legibility over dense light content, dark bottom/tab optics, and hard-edge material/color mixing. The native coverage-dependent adaptation algorithm is unrecovered; the portable coverage policy and private radius mapping remain approximations. Arbitrarily transformed or frame-animated scroll/chrome geometry is outside this fixture.

No timing, animation, physical-device, accessibility or alternate-runtime verification is inferred from this static comparison. Nil-mask exception behavior is recorded separately from optical mask images.

## Reproduction

```sh
python3 Demo/ScrollEdgeComparison/compare-portable.py /Users/daniilvinogradov/Documents/Dev/NXKit/docs/scroll-edge-effects/native /private/tmp/nxkit-scroll-pocket-b6d-raster /Users/daniilvinogradov/Documents/Dev/NXKit/docs/scroll-edge-effects/scroll-pocket/comparison
```
