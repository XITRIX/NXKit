# Dark, tint and retained-size comparison

This audit uses Apple's real UIKit compositor on an iPad Pro 11-inch (M5),
iPadOS 26.4.1, scale 2, with light/dark view traits and accessibility contrast
and transparency overrides disabled. NXKit uses its production SkSL material
and effect defaults on the raster backend. No shader tuning is included in this audit.

- [Dark appearance and tints](dark-tints.png): native above, NXKit below.
- [Light appearance and tints](light-tints.png): native above, NXKit below.
- [Neutral size comparison](neutral-sizes.png): five sizes in both appearances.
- [Light resize checkpoints](light-resize.gif) and [dark resize checkpoints](dark-resize.gif).
- [Fresh versus resized light glass](fresh-vs-resized.png).
- [No-animation verification](without-animation.png) with [effective card traits](without-animation-capture.json).

The GIFs show settled target states with illustrative timing. They do not
compare animation frames, latency, or NXKit's GPU/presentation-layer behavior.
The card grows from 120 × 64 through 165 × 88, 210 × 112 and 255 × 136 to
300 × 160, then shrinks back. The same native effect views are retained.
Blue and coral tints are supplied with alpha 1 to both implementations.

The earlier close match applies to freshly created light-mode glass. Resizing
the same native card on this runtime changes its neutral tone despite effective
light traits; returning to its original size retains that tone. The result was
reproduced with and without animation. It is an observation for this runtime,
not a documented rule for every UIKit release or device.

Other gaps are clear in the captures: regular dark glass is too bright and
saturated, explicit tint is lost on clear glass, and regular colored glass has
the wrong tint strength/tone. These remain implementation gaps.

| Interior mean RGB channel error, 0–255, card at 300 × 160 after resizing | Light | Dark |
|---|---:|---:|
| Untinted | 157.128 | 59.380 |
| Blue | 43.262 | 63.730 |
| Coral | 55.096 | 98.924 |

[All 54 checkpoint metrics](metrics.json) exclude four points at each shape's
perimeter, assessing material separately from corner and shadow differences.
The errors apply to these saved scenes, not a broad perceptual score.
[Growth/contraction checks](history.json) compare matching settled states;
the full-scene mean differences are below 0.001/255 after the initial change.

[Reproduction steps and fixture source](../../Demo/GlassComparison/README.md#dark-appearance-tint-and-retained-resizing).


The tone change above is version-specific: [27.0 validation](../liquid-glass-ios27/README.md)
shows stable light/dark material across direct and animated resize round trips.
It must not be used as a generic iOS 27 compatibility requirement.
