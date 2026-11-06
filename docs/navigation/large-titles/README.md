# Collapsible navigation accessories and large titles

The public surface follows Apple's `UINavigationBar.prefersLargeTitles` and
`UINavigationItem.largeTitleDisplayMode`. Defaults are false and Automatic.
Automatic inherits the preceding item's request, including when that predecessor
is currently collapsed by scrolling. Always still permits scroll collapse; it
requests the large-title presentation, rather than pinning it open. Never uses
the inline bar. Compact-height traits suppress the out-of-line row.

```cpp
navigationController->navigationBar()->setPrefersLargeTitles(true);
navigationItem()->setLargeTitleDisplayMode(
    UINavigationItemLargeTitleDisplayMode::always);
```

The new Full-screen iOS layout Demo entry is **Navigation → Large titles**. Scroll
slowly and release midway; push Automatic, Always or Never, then use Back or the
edge gesture. Tabs retain independent navigation stacks. Existing collection and
navigation-glass playground destinations explicitly request Never.

## Structure

- Core's navigation-item metadata is appearance independent. Core also supplies
  weak content-scroll and scroll-motion observation; UIKit never replaces the
  application's `UIScrollViewDelegate`.
- `NavigationBarAccessoryLayout` allocates measured out-of-line rows, consumes a
  scroll distance, and supplies resting boundaries. Large-title text is one row
  in the title carrier, separately clipped from the stationary button/glass row.
- The navigation controller retains each screen's collapse state, binds its
  first-chain/explicit content scroll view, and reserves the row's current
  extent in the child's container safe area. The full expanded travel range
  remains available to the scroll physics, independently of that visible inset.
  Resolving changed insets preserves content position and active motion, including
  changes made inside an existing layout callback.
- Core's public `scrollViewWillEndDragging` callback receives points/ms and a
  mutable content-coordinate destination. Container settling is proposed first;
  the application can replace that destination. An app override also removes the
  container's fitted settling response. Unchanged axes retain ordinary physics.
- The large-title row travels inside the same title carrier and reversible clock
  as content on push, pop, gesture completion and cancellation. Its bar extent
  uses that clock too. Back retains its existing identity.

Title-carrier clipping uses the carrier's own visible accessory extent, separately
from the bar's safe-area reservation. This fixes two order-dependent pop bugs:
the button-pop reveal previously captured the source inline bar height and hid
the incoming large row until completion; a retained gesture carrier kept clipping
enabled and its next layout shrank to the inline destination height, hiding the
outgoing row during a second swipe. Both paths now retain enough vertical space
for their own content throughout the transition.

`InteractiveNavigationTests::successiveLargeTitlePops` reproduces the exact
inline → large → inline pop sequence after a real animated push. Its 32 cases
combine button/swipe first pop, committed/cancelled second swipe, expanded/half
collapsed titles, both appearances and 2×/3× scales. It samples presentation
clipping at 120 Hz and forces layout during held gestures. These checks failed
before the clipping correction; checking final bar heights alone missed them.

Future search and palette implementations should supply accessory rows and
metrics to this layout/coordinator, rather than add another scroll delegate,
safe-area override, timer or navigation-transition clock. Search/palette public
APIs are deliberately deferred until those components exist.

## Native reference and limits

`Demo/NavigationComparison/LargeTitles/build.py` generates a standalone public
UIKit app and public XCUITest touch runner. Capture uses iPhone 17 Pro simulator,
iOS 27.0, built with SDK 27.1. No private UIKit selector is called. The reference
reads public UIView/UILabel geometry at display-link intervals.

`native/programmatic.json` records opt-in, offsets and Automatic/Always/Never
stack behavior. `native/touch-{24,40,140}.json` retain three OS-generated drags,
including their tracking and release frames. The shorter gestures held at bar
heights 92 and 76 points, then settled to 106 and 54 respectively. Intermediate
tracking geometry satisfies `barHeight + contentOffset.y + safeTop = 0` exactly
for this full-frame scroll view (safeTop = 62). This is an observed reference
configuration, not a production layout constant.

Fitting exponential release responses **after aligning the first post-release
sample** gives maximum errors of 0.36pt expanding and 0.62pt collapsing in the
first 250ms. These compare the retained native response with the fitted model;
they do not measure input latency or establish full animation/pixel parity.
`native-summary.json` retains frame counts, hashes, fit rates and errors.

NXKit's row height derives from the 34-point bold line metrics and its spacing
grid; safe areas and container placement derive from live bounds. Long titles
are fitted with an ellipsis at UTF-8 character boundaries. Full Unicode grapheme
truncation, Dynamic Type, RTL, the newer Inline large-title mode, overscroll
magnification and pixel-identical native push/pop title choreography remain
follow-up work. The basic out-of-line title and its collapse/settling/containment
path are implemented. Physical-device large-title verification remains pending.

Native UIKit programmatic changes differ from finger tracking: offset changes
can defer collapse until a threshold, and live title-policy changes may wait for
another presentation. NXKit applies its retained layout immediately. Do not use
a programmatic native offset sweep as the finger-tracking oracle.

References: Apple's [prefersLargeTitles](https://developer.apple.com/documentation/uikit/uinavigationbar/preferslargetitles),
[largeTitleDisplayMode](https://developer.apple.com/documentation/uikit/uinavigationitem/largetitledisplaymode-swift.property),
and [scrollViewWillEndDragging](https://developer.apple.com/documentation/uikit/uiscrollviewdelegate/scrollviewwillenddragging(_:withvelocity:targetcontentoffset:)).

Expanded native/portable screenshots are retained in `native/expanded.png` and
`portable/expanded.png`; portable half/collapsed frames are alongside them. The
portable harness omits the platform status-bar overlay. Expanded title glyphs
have equal measured width in this fixture; their current raster origin differs
by 1pt vertically and one display pixel horizontally. These are recorded visual
residuals, not a claim of pixel identity.

## Final verification

The focused Core, UIKit, NXKit, scroll physics, scroll discovery, layout, Demo,
scroll-edge and Metal-format regressions pass. Scroll-edge rendering retains all
213 checks. The native vertex-format round-trip test covers all 27 formats used
by Graphite, including the UShort4Normalized path exercised by large text.
Configuration performed during `loadView`/`viewDidLoad` is resolved before
navigation-transition snapshots; loading cannot install a ScrollPocket overlay
behind its content.

macOS Demo, iOS RelWithDebInfo Performance and Switch NRO builds succeed. Both
Apple app bundles contain the matching final ABI, 162 libraries and 129 pipeline
descriptions. All GPU binaries are freshly built from the validated raw catalog.
The final archive-only replay renders **21,018 frames** with **zero runtime
library or pipeline compilations**: 12,938 large-title frames plus 7,310 normal
navigation/scroll-edge frames and 770 deterministic ink/alpha-mask blur frames,
including both appearances and 1×/2×/3× edge
profiles. Large-title navigation uses 3× dark and 2× light. This is Mac Graphite
correctness/cache evidence, not an iPhone thermal or performance claim.

`verification.json` records the ABI, bundle executable hashes and profile hashes.
`metal/` retains compressed frame CSVs, edge summaries and runtime logs. The final
CPU loading-order corrections reuse the exact content-addressed raw descriptor
superset from the preceding full capture, with explicit provenance; no previous
GPU binary is relabelled. A fresh final-ABI navigation harvest and the full final
strict replay validate this catalog. Small-radius alpha-mask blur variants found
during the first strict replay are also compiled offline. Their exact emitted
MSL uses the same neighboring fragment signature with a different kernel loop
bound; the full strict replay validates their pipeline descriptors. The additional
`MetalNavigationBenchmark --ink-blur --full` sweep covers blur radii independently
of wall-clock animation sampling. The strict replay can be repeated with
`MetalNavigationBenchmark --large-titles --full` and
`NXKIT_METAL_REQUIRE_ARCHIVE_HIT=1`, pointing `NXKIT_METAL_BUNDLE_DIR` at the built
macOS app's `Contents/Resources/NXKitMetalShaders` directory.
