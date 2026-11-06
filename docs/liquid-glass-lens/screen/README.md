# Full-screen glass example

Open **Glass → Full-screen iOS layout** in the Demo. This screen hides the
surrounding NX navigation header and footer for its destination only. Back
actually pops the controller and restores the previous destination's chrome.

The screen combines a four-item interactive regular-glass platter with an
interactive selection lens, a centered inline title, leading Back, trailing
More and prominent +. Its scroll view draws inset-grouped sections and reserves
space for the navigation controls, floating tab bar and device safe areas.
Rotation recomputes these placements. Vertical edge pulls use the scroll view's
rubber-band resistance and spring-back on release; navigation and tab glass
remain fixed. Horizontal bouncing stays disabled.

Navigation Back, More and + share a `GlassContainerView` (12-point spacing),
so the trailing materials can merge during interaction. On Apple platforms,
navigation and all four tab icons use the native SF Symbols catalog through
`UIImage::systemImageNamed`, including native symbol weights, template tinting
and display-scale rasterization. This uses the system's symbol resources rather
than embedding a system font. Other platforms retain the portable fallback
artwork. The selected symbol row stays beneath the lens and is refracted and
masked by the existing selection composition.

Touching the tab capsule immediately lifts the lens without committing a tab.
A touch outside the presented lens animates its center toward the finger. A touch
inside its continuous silhouette preserves the grab offset, including while
the platter inflates. Dragging accounts for unequal compact item widths;
release snaps to the nearest tab and cancellation restores the previous one.
The active platter contact owns the lifted state: cancelling a descendant tab's
highlight during pan recognition cannot lower the lens, and pausing a held drag
keeps its refractive appearance lifted until release or cancellation.

The lens grows 8 points per edge when held (95×54→111×70 on the measured phone
layout), independently of the platter's existing inflation. Long regular glass
uses color-preserving press feedback. The [tab press comparison](../tab-press/comparison.html)
includes native light/dark holds over neutral and colored backgrounds. The later
[tab content comparison](../tab-content/comparison.html) corrects the symbol
viewports and fractional bitmap sampling against those retained captures.

## Actions

- Tabs change the title and collection; selection supports taps and lens dragging.
- More toggles item details; + and Add item append up to 30 additional items.
- Background cycles System, Blue and Coral; Accent color cycles Blue, Pink and Mint.
- Sort order reverses the collection; tapping an item toggles Open/Done.
- Reset clears additions, completion, details, order, accent and background.

State is shared across tabs. This is an application composition, not a new
`UITableView`, `UINavigationBar` or UIKit-compatible tab controller API.

## Typography

Point sizes and weights come from Apple's [Typography HIG — iOS/iPadOS,
Large (default)](https://developer.apple.com/design/human-interface-guidelines/typography#Specifications),
read on 28 October 2026. Tab captions instead use the component-specific styles
measured from UIKit, rather than the general Caption 2 preset. The demo
centralizes these styles in `GlassScreenTypography.hpp`.

| Content | Style | Size | Weight |
| --- | --- | --- | --- |
| Inline title and section headings | Headline | 17 pt | Semibold |
| Rows, values and completion summary | Body | 17 pt | Regular |
| Row details and section footnote | Footnote | 13 pt | Regular |
| Four-tab captions | Measured UITabBar style | 10 pt | Semibold normal / medium selected |

The native fixture uses SwiftUI semantic fonts and native bar text. NXKit uses
the current platform font backend with these default-category metrics. The
[text renderer correction](../../text-rendering/README.md) restores the resolved
system font, tracking, grayscale coverage and display-scale baseline rounding;
this report is rerendered with those corrections. Earlier text evidence is
preserved in `typography-before/`. This does
not implement Dynamic Type, a public UIFont facade, or HIG leading controls
for multiline paragraphs. HarfBuzz glyph placement and platform font versions
can still differ. Navigation glyphs are graphical symbols, separately
sized. No Apple font files are embedded.

## Native comparison

[Open the aligned pixel viewer](comparison.html). Six independently captured
states cover initial, blue-background scrolling, and reversed/expanded Library
rows in both light and dark appearances. The reference uses a real SwiftUI
inset-grouped `List`, `NavigationStack`, `TabView` and public glass button styles.
Back removes an actual navigation path entry. Production code never calls
private UIKit APIs. Native hierarchy inspection only measures bar/lens frames.

Reference: iPhone 17 Pro, iOS 27.0 Simulator, 402 × 874 points, safe-area insets
62/0/34/0, 3× native window captures. Native colors are converted from their
embedded profile to sRGB and downsampled to 2×; NXKit draws the actual retained
Demo tree at 2×. No translations, warps or color fitting are used.

Both initial five-row frames, and all six platter/lens frames, align to within
0.001 points. The viewer includes 50% overlay, absolute channel differences,
exact-match removal, and separately labeled 2/255 match removal. Whole-screen
channel MAE includes every pixel; it is not an isolated glass accuracy score.

Remaining visible differences include platform symbol metrics and font metrics,
constant-radius navigation scroll-edge blur versus Apple's variable material,
and existing glass/lens optics. This report establishes reproducible placement
and state evidence; it does not claim pixel parity, native touch timing, or
end-to-end Metal/Dawn equivalence. Landscape safe areas and Back restoration are
covered by the retained regression harness; native screenshots here are portrait.

`metrics.json` contains geometry and pixel scores. `provenance.json` records
source, renderer, native build and capture hashes; raw native pixels/metadata
are in `native/`.

## Reproduce

From the repository root, using an available iOS 27 Simulator:

```sh
python3 Demo/GlassComparison/build-glass-screen.py /private/tmp/nxkit-full-screen-native
xcrun simctl install DEVICE /private/tmp/nxkit-full-screen-native/GlassScreenReference.app
xcrun simctl launch --terminate-running-process DEVICE org.nxkit.GlassScreenReference
python3 Demo/GlassComparison/capture-glass-screen.py DEVICE /private/tmp/nxkit-full-screen-native/captures
cmake --build /private/tmp/nxkit-container/raster-tests --target GlassScreenTests LensTabTests AdaptiveTabTests -j4
ctest --test-dir /private/tmp/nxkit-container/raster-tests -R '^(GlassScreen|LensTab|AdaptiveTab)Tests$' --output-on-failure
python3 Demo/GlassComparison/compare-glass-screen.py /private/tmp/nxkit-full-screen-native/captures /private/tmp/nxkit-container/raster-tests/GlassScreenTests docs/liquid-glass-lens/screen
```

Use the Pillow/NumPy Python environment for the final command. Configure the
retained raster build with `NXKIT_RASTER_CONTROLLER_TESTS=ON` using
`Demo/GlassComparison/RasterTests/CMakeLists.txt` and compatible Skia raster
archives (`NXKIT_SKIA_RASTER_ARCHIVE` and matching `NXKIT_SKIA_RASTER_HEADERS`). `GlassScreenTests` checks real
routed Add/Back touches, actions and shared state, four-tab selection, scroll
extent, portrait/landscape safe areas and restored navigation reservations.


## iOS display-scale correction — 28 October 2026

Physical iPhone 15 Pro Max feedback exposed a startup-only coordinate override
that the earlier retained-tree comparisons bypassed. `UIApplicationMain` forced
an extra factor of 1.3 on iOS: 430 × 932 UIKit points became 559 × 1211.6 layout
units, and the 3× drawable scale became approximately 2.3077. A 44-point control
therefore occupied about 101.5 pixels instead of 132 (23.1% smaller). Device safe
areas still arrived in native points, so content also encroached on native chrome.

Startup now retains the backend's native point size and pixel scale. This follows
Apple's [UIScreen.scale](https://developer.apple.com/documentation/uikit/uiscreen/scale)
contract; the demo's point dimensions and fonts need no device-specific multiplier.
The optional low-level `setExtraScaleFactor` API remains available to callers,
but application startup no longer sets it.

`ApplicationScaleTests` compiles the real iOS startup path with an injected
raster context. It checks 2×/3× portrait and landscape, native bounds/scale/safe
areas, the actual rendered pixel extent of a 44-point control, and normalized
SDL touch delivery in points. Run it with:

```sh
cmake --build /private/tmp/nxkit-container/raster-tests --target ApplicationScaleTests
ctest --test-dir /private/tmp/nxkit-container/raster-tests -R '^ApplicationScaleTests$' --output-on-failure
```

This checks startup and the portable render/input path; it does not exercise
SDL's actual UIKit window or Metal surface. To verify on the iPhone 15 Pro Max,
rebuild and open **Glass → Full-screen iOS layout**: portrait window bounds should
be 430 × 932 points, `displayScale()` should be 3, the 44-point navigation buttons
should span 132 physical pixels, and tapping/scrolling should align with visible
controls. Rotate and return to portrait to check safe-area placement. A fresh
physical-device capture after this correction is still required.
