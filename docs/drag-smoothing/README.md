# Drag presentation smoothing

The recognizer continues reporting measured translation and velocity. A small,
shared presentation follower smooths the motion of the consumer: UIScrollView
owns its content offset and release-to-inertia handoff; a draggable view owns
its frame. This keeps animation policy out of recognition thresholds, contact
membership, state delivery and release-velocity estimation.

This division follows the purpose of Apple's
[UIPanGestureRecognizer](https://developer.apple.com/documentation/uikit/uipangesturerecognizer),
which exposes continuous recognition, translation and velocity. Smoothing is an
explicit NXKit extension, not a claimed UIKit recognizer property.

## Input delivery

The checked-in SDL Switch driver reads integer touch positions on a 1280×720
coordinate grid and reports changes when it polls. This alone does not establish
the physical sensor's effective resolution or sample rate. Quantization, sample
cadence and delivery can each cause visible steps.

NXKit previously updated each touch's position and receipt timestamp, then
silently discarded motion notifications less than 5 ms apart. A burst drained
from the SDL queue could lose most of its samples. The adapter now delivers
all motion packets, retaining their sampling intervals by mapping
[SDL's nanosecond event timestamps](https://wiki.libsdl.org/SDL3/SDL_TouchFingerEvent)
into the existing Timer clock domain. Mouse-to-touch events carry their source
timestamps too. Missing/future timestamps fall back to receipt time.

## Presentation policy

`NXKit::DragSmoother` is a reusable Core utility in `tools/DragSmoother.hpp`.
Reset it to the displayed position, feed measured targets and velocities, and
call `advance(seconds)` once per display frame. It uses exponential following
with an adaptive 8–32 ms time constant: stronger smoothing for slow movement,
less lag for fast movement. It preserves subpixel accumulation, never predicts
past the latest target, and settles when the finger stops. Double precision
accumulation avoids stalls at large content offsets. Nonfinite values and
negative frame durations are rejected.

This trades a small amount of visual lag for reduced stepping. It cannot recover
movement the sensor did not measure, eliminate rendering stalls, or guarantee
that the subjective touch feel is better before physical-device comparison.

`UIScrollView::setDragSmoothingEnabled(bool)` enables this presentation policy;
`isDragSmoothingEnabled()` reads it. The default is true on Switch and false on
other platforms. XML exposes `dragSmoothingEnabled`. UIKit-calibrated recognition,
rubber-banding and inertia remain the underlying behavior.

During a smoothed drag, scroll content uses fractional offsets rather than
pixel-rounded motion. A slow release smoothly finishes the measured pending
distance; its final didScroll callbacks may therefore follow didEndDragging.
A fast release cancels the follower and starts inertia from the displayed offset
with the existing measured velocity. Cancellation, policy changes, disabled
scrolling, removal, viewport changes and programmatic offsets cannot retain a
stale target. Reentrant didScroll replacements win over the old callback.

The Demo's blur/glass drag examples, Yoga drag example and glass gallery use
`DragPresentation.hpp` to apply the same utility to frame/layout positions on
Switch. Their recognition callbacks stay immediate. Other platforms retain
immediate presentation for these examples.

## Verification

The retained `tests.log`, `scroll-tests.log` and `scale-tests.log` cover:

- Existing native-calibrated pan/scroll physics and gesture arbitration.
- A real queued SDL burst with three motion packets 4 ms apart, all delivered
  in one drain, preserving sampling intervals.
- Subpixel following, frame subdivision, direction reversal, no overshoot,
  stationary convergence, fast/slow response and large-coordinate precision.
- Slow release settlement, fast inertia from the visible offset, unchanged
  gesture velocity, programmatic rebasing, reentrant replacement, scroll-disable,
  removal, viewport changes, indicator cleanup and XML policy.
- Existing 2×/3× startup, coordinate scaling and touch routing.

Switch builds succeed. The final cold and bundled shader-cache/device smoke
checks and subjective touch comparison are recorded after deployment below.

The final build completes both 28-stage hardware passes with 51 cached keys and
a 2,453,320-byte native cache. `bake.log` and `bundled-validation.log` record
initialization, presentation and normal fixture completion. The installed NRO
at `/switch/ThorVGApp.nro` matches the package by FTP SHA-256 readback; see
`deployment.json` and `shader-manifest.json`. Subjective slow-drag comparison
and a normal Demo exit are being checked separately from the bake fixture.

An additional report, `01793652287_03db12780bd84000.log`, has module ID
`1f1c38b0cafa9d899e21f7792cc32dbfde4838e6` and predates the final smoothing
build. Its retained stack indicates view/text teardown. A matching debug ELF
was not retained, so offsets are not assigned conclusively using another build.
The final smoothing build's ELF is preserved locally for an exact comparison
if normal exit reproduces it.

Normal Demo startup also loads 51 saved keys and presents at least 2,400 frames.
The final ELF build ID is `3fb5dbe8cd149b138144ba431f801aaae484b2b6`.
On 2026-11-02, the user confirmed the physical-device movement is
“smoother, with acceptable lag.” The subsequent normal Demo exit crashed.
Report `01793653260_03db12780bd84000.log` matches this exact ELF and faults in
`_malloc_r` while copying the animated-layer registry during cleanup. This
indicates earlier heap corruption, subsequently traced to the press-release
iterator lifetime described below.


### B-button exit repair

The exit fault was unrelated to touch-coordinate filtering. On gamepad release,
`UIApplication` held an iterator into `_gamepadButtonPresses` while delivering
`pressesEnded`. The Demo's B action called `handleSDLQuit`, which cleared that
map. Release handling then erased the freed node, corrupting allocator state.
Direct `handleSDLQuit` exit tests and shader-bake exits did not cover this path.

The fix detaches source entries and completed events before callback delivery,
retains the delivered event by value, and prevents finished events from being
redelivered during nested cancellation. Axis reversal also stops when its
release callback quits or replaces the source. Queued events stop draining once
quit is requested. Keyboard release uses the same completion path.

`UIApplicationPressLifecycleTests.cpp` covers button/axis/keyboard release
callbacks that quit, nested cancellation and replacement of the same source.
`press-lifecycle-before-asan.log` reproduces the invalid map erase with the old
button-release ordering; `press-lifecycle-after-asan.log` records the fixed path.
The sanitizer run instruments the application event dispatcher and regression
fixture; linked raster dependencies are not sanitizer builds. The ordinary
headless gesture, scroll, application and press-lifecycle regressions also pass.

The automatic `--nxkit-exit-test` fixture visits all eight normal Demo tabs,
queues quantized slow touch drags, then queues actual SDL B-button events.
On 2026-11-03 the fixed NRO installed successfully. The fixture visited all
eight screens, rendered more than 1,200 frames, sent quantized touch drags,
and exited on the first SDL B-button release. The direct-quit failure fallback
was not reached. The renderer saved an 824,788-byte native cache and the user
confirmed Sphaira was open. `exit-fixed-device.log` records the run; the exact
ELF build ID is `58d3ee079fb7a483c67ccc64da568c8053d6f341` and shader ABI is
`9fd11c965619aeab8412a2fba913dae4e10d692801f422d7962764fc331a8828`.
Final crash-report readback and refreshed bundled shaders await FTP restart.
All temporary routing changes used during reconnection have been removed.


An extra run of `NXKit/tests/UIPressRepeatTests.cpp` passes its button, trigger,
repeat and disconnect checks but fails three long-`UITextView` focus-scroll
assertions in the headless environment. Repeating it with the original HEAD
application event dispatcher gives the same three failures
(`press-repeat-baseline.log`); these are not introduced by the exit repair.
