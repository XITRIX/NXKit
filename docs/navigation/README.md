# UIKit navigation bar and controller

The current [complete playground motion audit](cycle/README.md) adds the native
eight-layout Demo, all 288 layout pairs and dense push/Back cycle comparisons.

The [leading-material follow-up](leading-material/README.md) fixes the dark hole
formed when supplemental glass overlaps Back, with independent ink reveal and
light/dark push/pop comparisons.

The [Metal pipeline follow-up](pipeline-cache/README.md) removes navigation's
large compilation stalls with stable sampling graphs, build-time shader/pipeline
archives, native preparation before interaction, and persistent recovery caches.

Audit date: 2026-11-03. The API surface is a usable inline-navigation subset,
tracked as **Partial** in UIKIT_API_PROGRESS.md.

The [navigation grouping audit](grouping/README.md) supersedes the earlier
sibling-background workaround. Controls now belong to their glass groups, with
native grouping, contact, split/join and interruption regressions.

The [frame-by-frame motion audit](motion/README.md) adds overlapping SDF necks,
absorption, fitted surface/birth springs and foreground blur, with 20 light/dark
native clips and before/after filmstrips. It supersedes the original width-only
onset-aligned motion experiment below.

The subsequent [layout and interaction regression audit](layout-fixes/README.md)
covers corrected tab placement/lens padding, 2/3/4/5-tab native comparisons, and
routed navigation-item glass press/drag feedback.

Core owns `UIBarButtonItem` and `UINavigationItem`, alongside lazy, read-only
`UIViewController::navigationItem()`. UIKit owns `UINavigationBar` and
`UINavigationController`. Neither Core nor NXKit links UIKit. The existing NX
navigation controller retains its own Nintendo presentation.

Public method names follow Apple's Objective-C headers: `pushNavigationItem`,
`popNavigationItem`, `setItems`, `topItem`/`backItem`; controller stack methods,
`navigationBar`, weak delegates and `setNavigationBarHidden`; navigation title,
custom title view, left/right single items and arrays, hidden/supplemental Back;
bar button title/image, style, custom view, primary action, width, tint and enabled
state. C++ `UIAction` closures replace Objective-C targets/selectors. Unsupported
APIs are omitted rather than exposed as empty methods.
The compatibility references are Apple's [UINavigationBar](https://developer.apple.com/documentation/uikit/uinavigationbar),
[UINavigationController](https://developer.apple.com/documentation/uikit/uinavigationcontroller),
[UINavigationItem](https://developer.apple.com/documentation/uikit/uinavigationitem) and
[UIBarButtonItem](https://developer.apple.com/documentation/uikit/uibarbuttonitem) documentation and installed SDK headers.

## Behavior

- Containers adopt children before loading their views. Only visible content loads.
- Animated stack operations serialize; queued pops return their reserved result
  while the active stack stays stable until the current transition finishes.
- `show` routes from a child to this navigation container. Automatic Back and a
  Core semantic menu/Back action pop the nearest stack and fall through at root.
- Content animates in container-owned hosts, preserving application transforms.
- Bar and content share one controller stack. Managed bar item/delegate mutation
  is guarded; applications configure the controller and its navigation items.
- Right button arrays are ordered from the trailing edge inward. Identical button
  items retain controls across updates. Leading/trailing glass carriers retain
  identity and interpolate visible bounds/position during push/pop and item edits.
  Incoming/outgoing content fades separately. Retargeting starts at the current
  presentation. The damped capsule interpolation is an empirical approximation.
- Plain content follows modern UIKit's label color, with explicit item tint
  overriding its ink. Bar/ancestor accent colors apply to prominent glass fills;
  prominent buttons keep white ink. See the [color follow-up](cycle/README.md#navigation-ink-color-follow-up).
- Portrait inline bars reserve 54 points with 44-point controls at their top;
  compact-height bars reserve 44. Device and application additional safe areas
  remain independent. Hiding/showing removes/restores only the bar reservation.
- A standalone navigation bar may pop its last item. A veto returns the attempted
  item while leaving its stack unchanged. A navigation controller preserves its
  root. These behaviors were measured rather than inferred from shared names.

The Full-screen iOS layout Demo is now:

```
UITabBarController
  ├─ UINavigationController → Home list root
  ├─ UINavigationController → Favorites list root
  ├─ UINavigationController → Library list root
  └─ UINavigationController → Settings list root
```

`GlassScreenView` contains the existing collection/list only. Each root retains
its own list state. Bar button items own Add/More/Close actions. The surrounding
NX Demo navigation is hidden for this destination and restored on exit. Old lens
and optical fixtures remain separate; the full-screen composition uses production
UIKit components.

## Native comparison and reproduction

`Demo/NavigationComparison/NativeReference.m` is a native-only UIKit application.
It records public stack/delegate/lifecycle behavior, UIKit view/layer presentation
geometry and steady-state screenshots. It does not call private UIKit APIs;
private class names in trace rows are observed data from public view traversal.
Steady screenshots are taken separately from the main transition sample window.

Generate/build an unsigned simulator probe:

```sh
python3 Demo/NavigationComparison/build-native.py /tmp/nxkit-navigation-native
xcodebuild -project /tmp/nxkit-navigation-native/Interaction.xcodeproj \
  -scheme Interaction -sdk iphonesimulator -destination 'generic/platform=iOS Simulator' \
  -derivedDataPath /tmp/nxkit-navigation-native/build CODE_SIGNING_ALLOWED=NO build
```

Install/launch `InteractionReference.app` with `xcrun simctl` on an available
simulator. The bundle identifier is `org.nxkit.NavigationReference`; optional
`--dark` selects dark appearance. The probe writes `Documents/native.json` and PNGs
in its simulator data container after about seven seconds. `native/source-manifest.json`
records fixture source hashes. The retained native capture is light-mode iOS 27.0. Dark-mode startup stalled in
CoreSimulator on this run; no dark native capture is claimed. Portable tests and
renders exercise both themes. Retained captures identify the simulator OS.

Build/run `UIKitNavigationTests` using the raster harness. Its optional arguments
are `REPORT_JSON SCREENSHOT_DIRECTORY [dark]`; screenshots are PPM. Then run:

```sh
python3 Demo/NavigationComparison/compare.py \
  docs/navigation/native/light.json.gz /tmp/navigation-portable.json \
  docs/navigation/comparison.json
```

Comparison separates exact stack/lifecycle/frame/safe-area checks from glass
geometry. The geometry tolerance is 3 points, accounting for simulator/iOS versus
macOS SF Symbol raster extents and the empirical material deformation. Screenshots
are visual evidence, not a claim of exact full-frame pixel parity. The headless
suite additionally checks live items, disabled actions/focus, custom views,
supplemental/hidden Back, mutable/queued stacks, retained presentation carriers,
weak ownership, content transforms, 1×/2×/3× traits and compact-height layouts.
The migrated Demo test exercises actual nested containers, routed bar/list input,
scroll bounce, independent retained list state, safe areas and outer NX exit.

The retained comparison passes all 10 stack/frame/safe-area cases and the exact
46-event lifecycle/delegate sequence. Settled shared-background geometry differs
by at most 2.34 points. After endpoint normalization and one onset alignment,
push width/center RMS errors are 1.13/0.081 points; pop-to-root errors are
0.46/0.078 points. This alignment does not measure input latency. Vertical strain
and private optical effects are outside those motion checks. See
[`comparison.json`](comparison.json), [`comparison.png`](comparison.png) and
the migrated [`Demo render`](demo-light.png).

The earlier full raster and Core-only suites passed 32 and 13 tests respectively.
After rebuilding Apple Skia from the current revision, a fresh Core/navigation/tab
suite passes 3/3 against the matching headers and release archives. The retained
portable comparison and light/dark bar renders have been refreshed with that build;
all native state, lifecycle and motion checks still pass. Public navigation headers
also compile independently, and the Switch Demo packages an NRO.

Matching Metal/Graphite Skia libraries now exist in `out/ios-arm64` and
`out/mac-arm64`. The iOS Debug Demo builds with Xcode and code signing disabled;
the macOS Demo builds through the documented CMake flow. Both compile and link
against current headers without a fallback. Apple builds disable Skia's optional
Chromium allocator to match NXKit's consumer configuration. The old iOS archives
are preserved in `Submodules/skia/out/recovery/navigation-apple-20261103`.
See [the Apple build manifest](../skia-upgrade/apple-build.json) for revision,
configuration, SDKs and artifact hashes.

## Remaining work

Large titles, search, custom Back-item metadata, bar appearance objects, menus,
full RTL/overflow/flexible-spacer policy, toolbar presentation, full-content/RTL interactive pop,
custom transition-controller APIs and scroll-edge material adaptation remain
unimplemented. The native optics and spring implementation are approximated;
exact pixel/trajectory parity and physical iOS/Switch GPU behavior are not claimed.
Platform build results verify compilation/linking, not device execution.

Interactive edge-pop behavior, cancellation/delegate timing, 32 paired UIKit/portable clips and shader coverage are in [the interactive navigation audit](interactive/README.md).
