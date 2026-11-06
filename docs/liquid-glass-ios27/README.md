# iPadOS 27 theme stability during glass resizing

Validated on **iPadOS 27.0, build 24A434**, using a native UIKit fixture on
an iPad Pro 11-inch (M5, 16 GB) Simulator at scale 2. The same application
binary is built with the iPhoneSimulator 27.1 SDK and also runs on 26.4.1.
Both use the same artwork, logical geometry, `UIGlassEffect(.regular)`, and
explicit light/dark view traits. The resized card's own effective appearance
is recorded in the JSON capture metadata.

[Native fresh/direct/animated comparison](comparison.png) shows the same
300 × 160-point card before resizing and after 120 × 64 → 300 × 160 changes.
The direct path has no UIView animation; the animated path uses 0.3 seconds.
Captures are taken after settling. No correction, recoloring, or alignment
is applied to the source pixels.

| Change from fresh light-mode card, mean absolute RGB channel difference (0–255) | 26.4.1 | 27.0 |
|---|---:|---:|
| Resize down and back without animation | 155.8807 | 0.000051 |
| Resize down and back with animation | 155.8808 | 0.000002 |

The older runtime changed the neutral material to a dark tone while the
card's effective traits remained light. **27.0 retains the light material**
through the same size changes. Dark appearance is stable as well: direct and animated round trips differ from its fresh card by less than 0.001/255 per channel (see [dark measurements](dark-stability.json)). This validates the reported resize-theme
stability on the tested runtime. It does not establish that every form of
backdrop/size-dependent color adjustment has been removed, or that this is
an API guarantee across all versions and devices.

Apple describes Liquid Glass refinements in [iOS 27](https://www.apple.com/os/ios/).
The [iOS 27 release notes](https://developer.apple.com/documentation/ios-ipados-release-notes/ios-ipados-27-release-notes)
do not describe this particular resize behavior; the conclusion above comes
from these captures, not an inferred documentation promise.

The installed 27.1 Simulator runtime (24A94401) reports zero supported device
types, so CoreSimulator rejects creating a comparison device. 27.1 was not
validated. The 27.1 SDK alone is not evidence of 27.1 runtime behavior.

[Original-pixel measurements and source metadata](metrics.json) accompany the
saved crops. Reproduce with the fixture's `probe-resize.py` and
`compare-versions.py` in [Demo/GlassComparison](../../Demo/GlassComparison).
