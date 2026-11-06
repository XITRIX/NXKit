# Actual tab-bar contour validation

[Open the comparison](comparison.html). Select resting or held, then UIKit,
NXKit, the previous NXKit pixels, overlay, absolute difference or transparent
unmatched pixels.

These ten retained native UITabBar cases exercise both themes over neutral,
blue, brown, coral and green backgrounds. The current production Demo is driven
by a routed stationary touch. The before images come from the previous
`liquid-glass-lens/tab-content` report; no fitted registration or color correction
is applied. Geometry comes from the native touch trace, and both sides are
compared at 2× after color management to sRGB.

[Metrics](metrics.json) report full-crop pixel error, geometry and the original
platter feedback probe. The full-crop score includes captions, icon vibrancy,
selection lens and other remaining differences; it is not an isolated border
score. Individual full-tab cases can regress slightly despite better exterior
contour profiles. Press probe colors, selection geometry and touch behavior
are not changed by this contour correction.

[Build/capture hashes](provenance.json) identify the inputs. The independent
[exterior/highlight matrix and method](../README.md) isolate regular-glass edge
behavior across five geometries and ten backgrounds.
