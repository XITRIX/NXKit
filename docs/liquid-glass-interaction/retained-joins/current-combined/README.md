# Combined material and shadow follow-up

Recomputed 28 October 2026 against the same96 native UIKit27.0 retained-join checkpoints described in the parent report. Native embedded DisplayP3 profiles are converted to sRGB. Presentation rectangles come from native traces; no alignment, contrast or color adjustment is fitted. Both adapters use explicit2× effect backing resolution.

The frozen old baseline is the matched-2× archive/output used by the parent report (overall11.860705). The current core combines the later material, clear-glass, chroma and native-calibrated shadow corrections. Waist MAE decreases to **4.896651**; the earlier integrated result was5.127541. These are complete rendering comparisons, not an isolated measure of the shadow algorithm.

| Palette / appearance | Initial and settled | Held | Pulled |
| --- | ---: | ---: | ---: |
| Neutral/blue, light | 3.265 | 7.921 | 8.279 |
| Neutral/blue, dark | 2.176 | 4.890 | 4.784 |
| Blue/coral, light | 4.773 | 8.435 | 8.217 |
| Blue/coral, dark | 3.224 | 4.629 | 4.316 |

One checkpoint remains slightly worse than the frozen baseline: dark blue/coral, reversed order, gap4, held, **8.868→9.025**. This improves on the earlier final9.119 but is not pixel parity. Every image file here contains native / frozen old / current / amplified absolute difference, with initial / held / pulled / released rows. `metrics.json` retains every checkpoint's body controls, geometry time difference, mean residual RGB, MAE and P95. Motion is bypassed by injecting the observed native presentation rectangles.

The separately measured neutral shared-size resolution correction is byte-identical across all96 of these equal-size fixtures. `provenance.json` retains both current-core archive identifiers and every before/after full-raster SHA256. Mixed-size shared defaults are validated in [their separate report](../../../liquid-glass-shadows/shared-material/README.md).

![Neutral/blue dark, reversed order, retained join](join-nb-dark-g2-reverse.png)

![Blue/coral dark, reversed order, retained join](join-bc-dark-g2-reverse.png)

Reproduce with the parent report's native24 cases and `compare-merge-matrix.py`, using the matched-2× frozen baseline outputs and the exact core archive identified here. Full raster intermediates remain at `/private/tmp/nxkit-group-body/merge-results` and the shared-size candidate at `/private/tmp/nxkit-group-body/merge-candidate-results` in this session.
