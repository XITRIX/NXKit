# Floating tabs and navigation-item interaction — 2026-11-03

The production tab component now retains the measured four-point selection inset
on all sides. Two/three tabs use natural centered widths; four/five fill the
available phone width. Cell centers, captions and selection geometry share the
same layout. Narrow layouts allocate different widths to longer titles, and the
lens interpolates width as well as position between those cells. Font-dependent
metrics are cached when items/traits change, outside the interactive frame path.

The 62-point platter overlaps a 34-point home-indicator safe area by 13 points,
ending 21 points above the window edge. The previous placement ended 46 points
above it. Child content receives an 83-point bottom safe area, including the
device inset once; application additional insets remain independent.

Navigation buttons and morphing glass are siblings. A passive observer on their
common bar now routes the complete contact lifetime to the relevant material.
The observer preserves control and gesture delivery. Routed regressions cover
plain/shared and prominent capsules, dragging, release, cancellation, disabled
items, and exactly one action per completed press.

## Native evidence

`TabVariantsReference.m` uses a real public UIKit controller, including both
themes and first/last selections, in 390- and 402-point viewports on iOS 27.0.
Its ordinary UIView traversal records private class names as observations;
it never invokes private APIs. `TabGeometryReference.m` independently measures
explicit-width native UITabBar views. Sources and source hashes are retained.
`UITabBarView.frame` describes the floating platter; the comparison measures
that platter inside native UIKit's full-width UITabBar wrapper.

At 402 points wide, with a 34-point device bottom inset:

| Tabs | Platter width | Selection width | Bottom gap |
| --- | --- | --- | --- |
| 2 | 188 pt | 94 pt | 21 pt |
| 3 | 274 pt | 94 pt | 21 pt |
| 4 | 360 pt | 95 pt | 21 pt |
| 5 | 360 pt | 79.33 pt | 21 pt |

At a 390-point viewport, five tabs use content-weighted compression: the four
shorter-title cells measure 83.22 points and Favorites measures 87.12 points.
The selected lens uses that cell's actual width. At 430 points, the four-tab
platter measures 388 points and the selection measures 102 points.

[Light comparison](variants-light.png) · [Dark comparison](variants-dark.png)
· [Corrected Demo](dark.png) · [Geometry results](comparison.json)

The portable suite checks every selection for 2/3/4/5 tabs, both themes, and
320/360/390/402/430-point widths: **140 passing comparisons**, maximum lens/frame
error **0.23 points**, below one 3× pixel. Controller placement and child safe
areas are compared directly at 390/402 points. Other widths use the explicit-width
bar oracle for horizontal geometry and lens padding; their vertical controller
placement is not claimed as an independent native sample. These are geometry
checks, not full-frame optical/pixel parity. Dynamic Type, long-title policy,
overflow/More and general tablet presentation remain incomplete.

## Reproduction

```sh
python3 Demo/NavigationComparison/build-tab-variants.py /tmp/nxkit-tab-variants-native
xcodebuild -project /tmp/nxkit-tab-variants-native/Interaction.xcodeproj \
  -scheme Interaction -configuration Debug -sdk iphonesimulator \
  -destination 'generic/platform=iOS Simulator' \
  -derivedDataPath /tmp/nxkit-tab-variants-native/build CODE_SIGNING_ALLOWED=NO build
```

Install/launch the resulting app with simctl, using bundle identifier
`org.nxkit.TabVariantsReference`. It writes `Documents/tab-variants.json` and
screenshots after about 17 seconds. Build the standalone probe with the iOS
Simulator clang SDK and run it using `simctl spawn` to produce `standalone-widths.json`.

`UIKitTabTests` accepts optional `REPORT_JSON IMAGE_DIRECTORY` arguments and
writes PPM screenshots. Compare recorded geometry with:

```sh
python3 Demo/NavigationComparison/compare-tab-variants.py \
  docs/navigation/layout-fixes/native/tab-variants.json \
  docs/navigation/layout-fixes/native/standalone-widths.json \
  /tmp/portable-tabs.json /tmp/tab-comparison.json
```

All five focused suites pass against current Skia: UIKit navigation, UIKit tabs,
the migrated Demo screen, lens/tab interaction, and gesture/material interaction.
The original navigation oracle also retains all 10 state and 46 lifecycle/delegate
matches. Both the macOS Demo and unsigned iOS Xcode Debug Demo compile/link with
the corrections. Physical-device execution of this patch is not claimed.
