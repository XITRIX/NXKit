# Earlier ScrollPocket blur progression — 2026-11-04

The supplied Demo screenshots showed readable cyan text behind “Shared”. In the supplied Telegram reference, content at the title was already blurred. The kernel maximum was sufficient; its progression was late. Reusing the content-fade recovery curve left the kernel nearly sharp at the title and completely sharp before the pocket's inner boundary.

The radius has a separate normalized profile now. It grows immediately as content enters the live pocket and retains more blur beneath chrome. The maximum stays **4pt**, and the independent **0.4→1 content fade** is unchanged. Pocket geometry still comes only from live safe areas and associated containers. These are appearance stops within those bounds, not added layout distances.

| Distance inward from outer edge / pocket extent | Previous sigma | Revised sigma |
| --- | --- | --- |
| 0% | 4pt | 4pt |
| 70% | 0.96pt | 3.32pt |
| 80% | 0.24pt | 2.60pt |
| 90% | 0pt | 1.20pt |
| 100% | 0pt | 0pt |

The actual variable-kernel filtering, tap counts, resolution selection and caching are unchanged. This does not introduce a masked uniform blur, an extra blur pass, content analysis or content replay. Hard and NXKit pockets are unchanged.

## Visual regression

The paired pictures render the same mounted navigation/tab/scroll hierarchy, typography, scroll offset and content in each version. A row crosses the centered title and another sits below the live safe boundary. These are NXKit raster captures; the user's supplied Telegram image is the visual target, not an independently captured native test.

![Dark comparison](images/comparison-dark.png)

![Light comparison](images/comparison-light.png)

The mounted impulse regression measures kernel variance at the actual navigation bar center, independent of chosen gradient stops. Before the change it was only **1.5–2.0%** of outer variance at 1×/2×/3×. The corrected path must retain **40–85%**, giving substantial blur beneath the title while preserving progression. A separate check keeps the maximum at 4pt. Both checks run in light and dark mode; the mounted half-radius regression still distinguishes a narrowing kernel from uniform-blur opacity blending.

[Before output](before-tests.log) reproduces all six title-band failures. [After output](test-output.log) passes **187 behavioral checks plus six optional image-encoding checks**. Geometry, sparse content, mask constancy, all four filter directions, zero-radius sharpness and the 3× resolution transition remain covered.

## Build and GPU verification

[Verification](verification.json) records source hashes, app/bundle hashes and retained logs. The fresh six-profile cold/strict edge sweep covers both themes at 1×/2×/3×; two push/pop smoke profiles check navigation integration. Each sweep has 7,310 frames. The strict sweep requires the newly baked archive with zero runtime Metal library/pipeline compilation. macOS and unsigned iOS Demo builds include matching offline bundles.

Reproduce with `ScrollEdgeEffectsTests`; set `NXKIT_SCROLL_POCKET_PICTURES` to export the six mounted text-overlap images. [run-metal.py](run-metal.py) runs `cold` or `strict` using explicit local build paths. Catalog merging uses `../scroll-pocket/merge_catalog.py --navigation-smoke`, followed by configuring and building both ordinary Apple projects before the strict run.

No new physical-device or native UIKit optical capture was performed. The [earlier variable-kernel performance report](../variable-kernel/performance.json) remains historical; it is not a timing measurement of this revised profile. Exact native kernel/progression parity, phone thermals and Switch GPU visuals remain unverified.
