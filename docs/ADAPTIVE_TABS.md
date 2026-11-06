# Adaptive NXTabBarController

`NXTabBarController` adapts to its inherited **horizontal size class**. No new
configuration is required, and `tabBarWidth()` still controls the regular sidebar.

| Horizontal class | Presentation |
| --- | --- |
| Regular | Existing sidebar beside the selected tab's content. |
| Compact | Full-width tab list. Activating an item opens its controller through `show()`. |
| Unspecified | Regular layout, preserving the previous detached/startup behavior. |

In compact mode, moving controller focus through the list does not open a screen.
Touch or confirm activation still consults the selection delegate. Back returns
to the list and leaves that item selected. The user's `selectionFollowsFocus`
preference is restored when regular width returns.

When compact becomes regular, an open tab becomes the content beside the sidebar.
Its controller, view, navigation item and application state are preserved. The
transfer waits for an in-flight navigation transition and queued stack operations
to settle. A nested screen pushed above the tab remains open; Back then returns
to the regular tab layout. Resizing does not emit user-selection delegate events.

Collapsing regular to compact shows the list and preserves its selection. If an
expansion is reversed while a compact push is still running, the latest size class
wins and the already-open destination remains open. Programmatic selection on a
closed compact list changes its highlighted item; programmatic selection while a
compact destination is open replaces that destination.

The actual selected controller is temporarily transferred from tab containment
to the enclosing `NXNavigationController` stack. Back or expansion returns it to
the tab container. Other configured tabs remain contained. Navigation delegates
and per-screen navigation items are not replaced. If there is no enclosing
navigation controller, ordinary `show()` falls back to modal presentation;
full-screen and overlay dismissal restore tab containment. Overlay cleanup uses
a weak main-queue check while that compact presentation exists, since overlay
dismissal does not call the list's appearance callbacks.

The width class comes from `UITraitCollection`, rather than a controller-specific
pixel cutoff. See [size classes](SIZE_CLASSES.md) for native iOS behavior and the
portable window policy. This remains the NXKit sidebar/list design, not an
implementation of UIKit's adaptive `UITabBarController` appearance.

## Verification

`NXKit/tests/NXAdaptiveTabBarControllerTests.cpp` drives the production application
loop with a headless font context and simulated window sizes. It covers compact
startup without eagerly loading content, layout width, focus versus activation,
selection veto, Back input, reselect/reorder/removal, programmatic switching,
navigation-item identity, selected controller/view/state preservation, balanced
appearances, nested pushes, resize during push/Back, rapidly reversed size-class
changes, and standalone full-screen/overlay presentation.

With the Demo configured using `NXKIT_BUILD_TESTS=ON`:

```sh
cmake --build YOUR_BUILD --target NXKitAdaptiveTabBarControllerTests
```

The existing macOS raster harness also provides a reproducible execution target
without requiring Graphite archives (matching raster Skia headers/ABI and SDL3
are prerequisites):

```sh
cmake -S Demo/GlassComparison/RasterTests -B YOUR_RASTER_BUILD \
  -DNXKIT_RASTER_CONTROLLER_TESTS=ON \
  -DNXKIT_SKIA_RASTER_HEADERS=YOUR_MATCHING_SKIA_HEADERS \
  -DNXKIT_SKIA_RASTER_ARCHIVE=YOUR_RASTER_SKIA_ARCHIVES
cmake --build YOUR_RASTER_BUILD --target AdaptiveTabTests
ctest --test-dir YOUR_RASTER_BUILD -R '^AdaptiveTabTests$' --output-on-failure
```

Set `NXKIT_SKIA_DEBUG=ON` only if those archives were built with Skia's debug ABI.

Verified 2026-10-27, working tree after `7a1273a`: the adaptive harness and existing
tab/navigation regressions pass; the iOS Demo and controller tests compile/link
against the installed SDK. Physical iOS rotation and interaction have not been
executed. Full macOS Graphite and Switch builds require their missing Skia archives.

References: Apple's [`show(_:sender:)`](https://developer.apple.com/documentation/uikit/uiviewcontroller/show(_:sender:))
and [`horizontalSizeClass`](https://developer.apple.com/documentation/uikit/uitraitcollection/horizontalsizeclass).
