# Navigation glass grouping — 2026-11-03

Navigation controls now live **inside their glass capsule**, replacing the
sibling-background workaround. A capsule owns each visual group, so its contact
affine and morph timing carry both the material and its content. UIBarButtonItem
remains style-neutral metadata in Core; UIKit creates the presentation hierarchy.

## Native observations and implemented rules

The public UIKit fixture exercises 19 configurations on both sides and in both
themes (76 states), plus ten push/pop pairs (20 endpoints), on an iPhone 17 Pro
simulator with iOS 27.0. Default symbols and explicit 20-point regular symbols
are retained. Private class names are read-only hierarchy observations; the
fixture and production implementation invoke no private APIs.

| Configuration | Native / implemented grouping |
| --- | --- |
| Two or three image items | One capsule |
| Different plain foreground tints | One capsule; only ink is tinted |
| Prominent plus plain | Separate capsules; prominent material receives tint |
| Two prominent items / prominent middle item | Prominent items stand alone |
| Text plus text / text plus image | Text items stand alone |
| Fixed spaces of 0, 12, or 30 points | Split groups; the tested phone bar uses a 12-point inter-capsule gap for all three |
| Flexible space | Splits groups in this navigation bar |
| `sharesBackground = false` | Own capsule |
| `hidesSharedBackground = true` | Independent item with no glass material |
| Disabled image item | Remains grouped; no action or press response for that item |
| Custom view plus image | Shares; the custom control receives its own action |
| System Done / automatic Back | Independent capsule |

These rules agree with Apple's [navigation/toolbar guidance](https://developer.apple.com/videos/play/wwdc2025/284/),
[`sharesBackground`](https://developer.apple.com/documentation/uikit/uibarbuttonitem/sharesbackground)
and [`hidesSharedBackground`](https://developer.apple.com/documentation/uikit/uibarbuttonitem/hidessharedbackground).
Explicit UIBarButtonItemGroup APIs remain unsupported; native item groups can
override these per-item rules.

## Transitions and contact

Carriers match by surviving items and visual order. The new `identifier` property
matches controls across distinct metadata instances. Splits retain a carrier,
create additional capsules with a spring scale, and apply a bounded surface pulse.
Joins expand a receiving carrier and retire redundant ones. Joining ink enters
inside its receiving material. Capsule bounds and item positions use the same
spring clock, including delay and overshoot. Interrupted changes start from
presented geometry, including the visible pulse transform.

Four native XCUITest holds/drags cover shared, prominent, fixed-space-separated
and explicitly unshared cases. Each delivers exactly one action. Native glyph
and capsule affines agree below 0.001 points. An independent portable raster test
checks a marker's rendered centroid against its capsule's contact affine, within
0.4 points. Routed tests also cover release, cancellation and disabled items.

The animation engine now completes by elapsed duration and delay. A timing
curve crossing one in mid-animation no longer completes early; a pulse returning
to zero at its endpoint no longer leaves its presentation layer alive. Core
regressions verify both phases and cleanup.

## Results and limits

[Light comparison](grouping-light.png) · [Dark comparison](grouping-dark.png)
· [Machine-readable results](comparison.json)

All 76 grouping decisions and 20 transition endpoints match. The 72 scored
resting-frame cases differ by at most **1 point**, using equivalent explicit
symbols and a 1.1-point tolerance. AppKit and UIKit raster symbol metrics differ;
the original default-symbol measurements are retained too. Four custom UIButton
cases check grouping and actions, while intrinsic sizing is reported without
scoring because the two UIButton implementations have different intrinsic layout.

The later [frame-by-frame motion audit](../motion/README.md) supersedes the
initial split/birth approximation with overlapping SDF surfaces, fitted geometry
and pulse springs, absorption and foreground blur. Exact native pixel parity
is still not claimed. Transient clipping, arbitrary custom-view
sizing, item groups, automatic matching heuristics without identifiers, large
titles, overflow, Dynamic Type, RTL and tablet presentation need further work.
The split pulse and birth spring approximate the observed motion. Physical-device
execution is not claimed.

Seven focused suites cover Core boundaries, navigation, layout/animation timing,
Demo composition, tabs, lens/tab interaction and gesture/material interaction.
Navigation cases include in-flight glyph containment, identifier reordering and
interrupted split/join continuity. macOS and unsigned iOS builds are checked too.

## Reproduction

For manual testing, open **Full-screen iOS layout → Navigation → Navigation
morphing** in Demo. Use **Push next** to visit all eight arrangements, or use
the arrangement list to test any pair. The standard Back button reverses the
transition; **Pop to collection** restores the tab's original list. Navigation
item taps update the screen's status line. Switching tabs preserves each stack.
The leading-items case keeps automatic Back alongside a favorite action.

[Shared screen](demo-shared.png) · [Text actions](demo-text.png)
· [Leading actions](demo-leading.png)

The headless Demo harness can render a destination, with arrangements numbered
0–7: `GlassScreenTests --navigation /tmp/shared.ppm 0`.

```sh
python3 Demo/NavigationComparison/build-grouping.py /tmp/navigation-grouping
xcodebuild -project /tmp/navigation-grouping/Interaction.xcodeproj \
  -scheme Interaction -destination 'platform=iOS Simulator,id=DEVICE_ID' \
  -derivedDataPath /tmp/navigation-grouping/build -parallel-testing-enabled NO test
```

The runner uses public coordinate holds/drags. Launch without `HOLD_CASE` for
the matrix; pass `--explicit-images` for equivalent symbol inputs. Documents
receives grouping.json and screenshots after about 50 seconds, or hold-CASE.json
for a contact trace. Source hashes and compressed traces accompany this audit.

```sh
UIKitNavigationTests /tmp/navigation.json /tmp/images light \
  /tmp/grouping.json /tmp/images
python3 Demo/NavigationComparison/compare-grouping.py \
  docs/navigation/grouping/native/explicit/grouping.json.gz \
  /tmp/grouping.json /tmp/grouping-comparison.json
```

The final image-directory argument is optional. Create image directories first.
Static geometry checks compare group frames directly, independently of crops.
