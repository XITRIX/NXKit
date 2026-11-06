# Immediate resize and coordinated rotation — 2026-11-02

Desktop window resizing now redraws synchronously from SDL's native main-thread expose callbacks, including AppKit's modal live-resize loop. There is no resize easing or debounce. Frame pacing waits for events and preserves the waking event for normal delivery, so an idle frame interval does not add an unnecessary delay to resize/input handling.

On iOS, logical window size and framebuffer pixels follow UIKit's rotation transition rather than switching directly to the endpoint. The scale remains the native display density. Layout, safe areas and drawable allocation use one geometry snapshot per rendered frame.

## Findings and implementation

- The old loop rendered only after `SDL_PollEvent` returned. AppKit can hold that call inside its live-resize tracking loop. `SkiaCtx_sdlBase` now watches exposed events for its own window on the video thread and invokes a guarded render-only path. That path performs frame preparation, traits, animation advancement, layout, drawing and presentation, without polling events, executing dispatch/display-link callbacks or sleeping recursively. Nested exposes during rendering are ignored safely.
- Apple SDL pacing previously used a blocking thread sleep. It now uses `SDL_WaitEventTimeout(nullptr, remainingTime)`: native events can wake the loop promptly and stay queued. Common frame-budget calculation remains shared; Switch retains its existing pacing implementation.
- Native macOS Metal layer geometry changes disable Core Animation implicit actions. Texture wrapping uses the acquired texture's actual dimensions.
- The old iOS scale calculation divided SDL's possibly final-orientation pixel dimensions by current native point dimensions. Those can describe different moments during rotation. The iOS backend now samples stable native density independently and stops rewriting the native Metal view's frame each render.
- A private transparent child controller is attached to SDL's existing native root controller. It receives `viewWillTransitionToSize:withTransitionCoordinator:` and joins the native animation with a bounds probe. Its presentation bounds supply the real native duration/easing; there is no guessed rotation duration or custom easing curve. The probe has no visible content and does not receive input. SDL/UIKit retain ownership of the Metal view frame and rotation transform.
- Each frame freezes the sampled point dimensions, interpolated safe-area insets and scale. The drawable uses rounded point-size × native-density pixels, with native layer actions disabled. During a transition the application requests the display refresh rate instead of its normal idle rate. Ordinary nonanimated changes use current native bounds immediately. Generation checks prevent a superseded completion from ending a newer transition; declined/nonanimated transitions clear the animated state.

The implementation follows Apple's [size-transition callback](https://developer.apple.com/documentation/uikit/uicontentcontainer/viewwilltransition(to:with:)), [transition coordinator](https://developer.apple.com/documentation/uikit/uiviewcontrollertransitioncoordinator/animate(alongsidetransition:completion:)) and [drawable sizing](https://developer.apple.com/documentation/quartzcore/cametallayer/drawablesize) contracts, plus SDL's [event wait semantics](https://wiki.libsdl.org/SDL3/SDL_WaitEventTimeout). SDL's checked-in event documentation explicitly supports drawing inside OS expose watches during modal resize loops.

## Verification

The native iPhone 17 Pro simulator on iOS 27.0 completes portrait → landscape left → portrait → landscape right → portrait through public XCTest orientation changes. The fixture compiles the production `IOSWindowGeometry.mm` bridge directly, drives real Metal drawables and compares its dimensions with an independent UIKit-animated view.

| Native rotation measurement | Result |
| --- | ---: |
| Completed rotations | 4 |
| Acquired and presented Metal drawables | 383 |
| Samples strictly between orientation endpoints | 67 |
| Drawable-size mismatches | 0 |
| Native scale mismatches | 0 |
| Maximum size error against UIKit reference | 0.0000296 point |

The raw 383-frame trace is retained in [native-rotation.json](native-rotation.json). The native fixture uses no private UIKit or XCTest APIs and needs no simulator Skia archive.

The native Cocoa regression opens a temporary SDL window and verifies that an expose presents and updates the window/root size before returning to the caller; dispatch callbacks are not recursively executed. Synthetic worker-thread exposes cannot mutate UI. After hiding the window to drain incidental window events, a worker event wakes a nominal one-second pacing wait within 15–250 ms and remains queued for normal processing.

Five focused core regressions pass: application startup/scale, glass screen, traits, render traversal and the 64-frame nested-glass SDL replay. The startup harness covers 2×/3× portrait/landscape, immediate resize, a single per-frame preparation snapshot, no resize-path sleep and nested backbuffer-acquisition expose protection. See [validation.json](validation.json) and the retained concise logs.

The iOS Debug Demo builds and links with signing disabled. The macOS Metal backend passes an SDK syntax check, and the native Cocoa integration test runs against the available raster archive. A complete macOS Graphite app build remains blocked by the missing `Submodules/skia/out/mac-arm64/libskia.a` archive.

This validates the native geometry bridge and actual simulator Metal drawable dimensions, plus the portable render path. Full Demo Graphite rotation smoothness on a physical iPhone, rapid/cancelled real-device rotations, iPad multitasking, external displays and Switch hardware behavior remain unverified. Integer drawable sizes naturally change in pixel increments; the geometry probe remains continuous.

## Reproduction

With the existing raster harness configured against matching Skia archives/headers:

```sh
cmake --build /tmp/nxkit-container/raster-tests \
  --target ApplicationScaleTests DesktopResizeTests GlassStressPerformance -j 6
/tmp/nxkit-container/raster-tests/DesktopResizeTests
ctest --test-dir /tmp/nxkit-container/raster-tests \
  -R 'ApplicationScaleTests|GlassScreenTests|TraitCollectionTests|RenderTraversalTests|GlassStressReplay' \
  --output-on-failure
```

The Cocoa test needs window-server access. Generate a disposable native simulator project, then select an available iPhone simulator UUID:

```sh
python3 Demo/ResizeComparison/create_native_project.py /tmp/nxkit-rotation-probe
xcodebuild -project /tmp/nxkit-rotation-probe/Rotation.xcodeproj -scheme Rotation \
  -destination 'platform=iOS Simulator,id=SIMULATOR_UUID' \
  -derivedDataPath /tmp/nxkit-rotation-probe/build test CODE_SIGNING_ALLOWED=NO
```

The fixture writes `Documents/rotation.json` in its simulator data container. For the complete device-SDK Demo:

```sh
cmake -S Demo -B Demo/build/ios
cmake --build Demo/build/ios --target ThorVGApp --config Debug -j 6 \
  -- CODE_SIGNING_ALLOWED=NO CODE_SIGNING_REQUIRED=NO
```
