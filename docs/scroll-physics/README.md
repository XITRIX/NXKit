# UIKit pan and elastic scrolling comparison

Measured 2026-10-31, working tree after `042ac0c`.

[Follow-up: fast swipes into boundaries](impact/README.md) · [rubber-band reversals at both edges](reversal/README.md) · [12 reversal overlays](reversal/comparison.html)

[All 39 traces and before/after overlays](comparison.html) · [numeric results](metrics.json) · [capture provenance](native/sessions/manifest.json)

The public behavior target is Apple’s [pan recognizer documentation](https://developer.apple.com/documentation/uikit/uipangesturerecognizer) and [scroll-view documentation](https://developer.apple.com/documentation/uikit/uiscrollview). Numerical filters and thresholds below come from the retained runtime measurements.

The oracle is a standalone app containing Apple's public `UIScrollView` or `UIPanGestureRecognizer`, running on an iPhone 17 Pro simulator with iOS 27.0 at 3× scale. XCTest injects actual OS contacts. The recording includes raw contact identities, locations and timestamps, delivery times, public gesture state, display-link samples, viewport geometry and scroll delegate callbacks. There is no NXKit control or physics code in the native oracle.

The XCTest runner uses private **XCTest** pointer-path/event-synthesis plumbing to create overlapping contacts. It does not inspect private UIKit physics. Production code contains no private UIKit calls. All four retained capture batches passed; exact case lists, source hashes and test-result summaries are in `native/sessions/`. The captured reference/test sources are retained alongside the raw recordings.

## What changed

- Pan tracks contact centroids, rebasing membership without adding a jump. It supports minimum/maximum touch counts and keeps a recognized drag alive when fingers leave one by one. Translation accumulates across packets, `setTranslation` preserves velocity, and vector conversion respects affine scale/rotation without adding view or scroll origins.
- Native pan velocity matches a two-sample filter: `0.8 × previous instantaneous velocity + 0.2 × current`. Scroll release velocity applies a second two-sample filter: `0.75 × previous pan velocity + 0.25 × current`. These replace the accumulating average that carried old movement into release.
- The observed fling gate applies to the **pan's** release velocity near 250 pt/s, before the scroll release filter. The 249/250/251 linear-path cases bracket this using recorded velocities, not their nominal path names: the native 249 case stopped, while the other two decelerated, including with filtered scroll velocities below 250 pt/s.
- Free flight uses the configured deceleration rate and ends at 10 pt/s, rather than continuing to its infinite asymptote. Contact release exposes the next display position immediately, matching native inertia catches. Axis collision and stop times are solved analytically, independently of frame subdivision.
- Elastic re-grabs invert the rubber-band function before continuing the drag. The scroll pan's elastic translation is rebased with the visible content. A touch freezes inertia/bounce at its displayed position. A stationary catch cannot resurrect the preceding fling.
- A viewport change clamps an elastic offset and rebases active input. Other limit changes retarget active motion. Completed edge returns preserve exact fractional inset coordinates, preventing a rounded final value from remaining out of bounds.
- Begin/end dragging callbacks occur once per drag. Timer lifetime and generation checks prevent stale callbacks from overwriting a replacement. Completing layer animations snapshots registry membership so callback-driven removals cannot invalidate iteration.

A motionless hold **after an uninterrupted drag** is distinct from catching existing inertia. In the retained native hold case, the public velocity stays cached and UIKit resumes deceleration on release. The fix preserves that behavior. Slow movement before release, or catching a fling and holding still, stops it.

## Replay and error measurement

`Demo/ScrollComparison/Replay.cpp` drives the real NXKit window, recognizer and scroll-view paths with the retained contacts. Contact timestamps drive velocity; delivery timestamps and display-link times drive animation. Both clocks are retained because OS delivery can lag input timestamps. No fitted spatial registration or per-case offset correction is applied.

The before executable was frozen from the pre-change pan/scroll implementation at `042ac0c`, with only a deterministic timer-advance seam and test friendship added. Its hash and the final replay executable hash are in `provenance.json`. The old API had no touch-count bounds; those configurations could not be applied to the baseline. Before/after replay outputs are retained; the baseline executable itself is a temporary build artifact.

Offset RMS is measured over all display-link frames, in points. Translation/velocity errors also include event samples and resting frames. White is UIKit, orange is before, cyan is after. The native scroll view modifies its owned pan translation during elastic interruptions; read offset curves alongside translation curves when comparing those cases.

The critically damped edge-return approximation uses `omega = 12.1 s⁻¹`, calibrated against the normal pull trace. Other bounce/re-grab/resize traces remain separate checks. It is an observed approximation, not a claim about UIKit's private spring equation.

## Reproduce

Capture a fresh simulator oracle:

```sh
python3 Demo/ScrollComparison/capture-native.py SIMULATOR_UDID /tmp/nxkit-native-scroll
```

Build `ScrollReplay` and `ScrollPhysicsTests` with the existing headless raster CMake project at `Demo/GlassComparison/RasterTests`, using matching raster Skia headers/archives and SDL dependencies. The CMake cache must point to those locally built prerequisites.

Run the source regression target and retained native checks:

```sh
ctest --test-dir BUILD_DIRECTORY -R ScrollPhysicsTests --output-on-failure
python3 Demo/ScrollComparison/check-native.py BUILD_DIRECTORY/ScrollReplay
```

The Python scripts require NumPy. To regenerate the full before/after report when both replay executables are available:

```sh
python3 Demo/ScrollComparison/compare.py docs/scroll-physics/native BEFORE_REPLAY AFTER_REPLAY docs/scroll-physics --check
```

The focused routed-touch harness additionally checks a stable 40-contact overlapping relay, count limits, child-control cancellation, affine translation, delegate counts, stationary catches, slow release, spring interruption, viewport/content/inset changes, fractional resting edges, independent axis collisions, frame subdivision and reentrant timer replacement. The existing gesture, indicator, full-screen glass, lens-tab and startup-scale targets are also run. The iOS Demo builds with the installed iPhoneOS SDK.

## Remaining differences and evidence limits

- The long eight-path native FIFO relay contains unexpected all-up gaps and pointer-slot reassignment in XCTest. It is explicitly flagged in the report and is **not** evidence of native relay fidelity. A simple native overlapping handoff and the deterministic NXKit 40-contact relay verify membership continuity; real-device long scribbling still needs validation.
- UIKit defers some actions until its next run-loop pass. Activation/coalescing differences remain in several captures: for example `scroll-release-99` retains an approximately 16-point activation offset. These errors are visible, not registered away. Native short-count limits and dynamic touch-count changes need wider coverage.
- Bounce catches/re-grabs remain approximate, with a few points RMS difference in the retained offset traces. Rotation can also restart native internal bounce state; the timed viewport cases and actual rotation trace are retained separately.
- This validates simulator UIKit and portable NXKit routing. Physical iOS/Switch input, refresh-rate transitions, system interruption/cancellation and perspective transforms remain unverified. The existing axis-specific bounce API/defaults remain NXKit extensions; this does not implement the full UIKit scrolling/zooming/delegate surface.
