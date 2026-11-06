# Uncapped glass stress profiling — 2026-11-04

The existing **Glass → Glass stress · 96 cards / nested scrolls** scene is shared by the interactive Demo, SDL input replay and the new native Metal benchmark. No glass-quality settings, shader equations, rendering resolution or continuous frame compositing were changed.

For an Xcode scheme or shell launch:

```sh
NXKIT_GLASS_STRESS_REPLAY=1 NXKIT_UNCAPPED=1 \
  Demo/build/macos/ThorVGApp.app/Contents/MacOS/ThorVGApp
```

`NXKIT_GLASS_STRESS_REPLAY=1` directly launches the screen, waits two seconds, then scrolls the full vertical feed down/up over ten seconds while scrolling all horizontal rails. The cycle repeats; positions derive from each live content/viewport size and elapsed time. Omit it and navigate to the screen for manual input. `NXKIT_GLASS_STRESS=1` still launches it without automated scrolling.

`NXKIT_UNCAPPED=1` disables the common software frame wait, including SDL's event wait. It is read at process startup. Without the flag, regular pacing remains; the stress screen now requests the display refresh rate instead of a fixed 60 Hz. Native drawable acquisition may still limit the displayed app. The offscreen benchmark has no drawable, frame sleep, or display cap and measures actual Graphite/Metal work, unlike the older raster/null-canvas test.

## Measured improvements

Release C++ and matching release Skia, Apple M1 Max / macOS 27.0.1, serial processes, 402×874 logical points. Each configuration has three alternating-order before/after runs, each with three 160-frame passes. The first pass is excluded from timing aggregates. Both binaries are frozen; the baseline links the renderer at `efa4996`. Shader ABI is unchanged: `1d1a294df456de4737a4a3bd0d394ec1ee48892388fb7210a0e79d21428a18b3`. Every frame requires bundled library and native pipeline hits: **5,760 frames, zero runtime shader/pipeline compilations**.

Values below are the median of three warm-run medians. CPU includes layout/display, layer recording and submission. Total includes the explicit completion wait. The wait includes scheduling and synchronization; it is **not a GPU timestamp duration**. Offset changes happen outside the timed drawing block. These are offscreen completion-throughput measurements, not displayed FPS or phone energy measurements.

| Scrolling workload | Before | After | Reduction |
| --- | ---: | ---: | ---: |
| Dark 3× layout/display preparation | 0.848 ms | 0.197 ms | 76.7% |
| Dark 3× combined CPU | 3.012 ms | 2.338 ms | 22.4% |
| Dark 3× total through GPU completion | 9.610 ms | 8.932 ms | 7.0% |
| Light 2× layout/display preparation | 0.847 ms | 0.198 ms | 76.7% |
| Light 2× combined CPU | 2.780 ms | 2.168 ms | 22.0% |
| Light 2× total through GPU completion | 8.502 ms | 8.016 ms | 5.7% |

Stationary CPU medians fall 36.2%/31.1%. Scrolling completion waits remain approximately 6.5 ms at 3× dark and 5.6–5.8 ms at 2× light. Visible glass work remains the largest cost in this artificial scene. Host scheduling/GPU frequency cause variation, especially in stationary completion waits; do not infer a shader speedup from them. Per-run medians/p95 and all raw rows are in [metrics.json](metrics.json) and `data/*.csv.gz`.

## Hot paths and implementation

- **Clean display visits copied drawing environments.** Every one of 1,665 views resolved inherited tint, copied dynamic `UIColor` function objects and swapped traits several times, including views whose retained contents needed no drawing. Drawing environment preparation now happens around actual layer/view display callbacks. Descendants are still visited, redraw-dependent contents still invalidate, and a callback can invalidate later siblings. Each callback scope restores the caller's context.
- **Layout repeated corner walks.** Each recursive layout visit called `updateCornerConfigurationTree()` on its entire subtree, repeating descendant visits at each ancestor depth. Layout now resolves only the visited view's configured corners. Geometry/configuration setters retain their eager subtree propagation; platform and direct-layer changes are refreshed by the layout walk.
- **Weak parent pointers were copied and locked repeatedly.** Tint resolution now locks the stored weak parent once. Layout/display membership checks also use the stored pointer directly. Ownership snapshots remain for callback-driven removal/reparenting.
- **GPU glass work remains.** The workload processes the same visible layers and glass passes before/after. No GPU-filter reduction is claimed; retaining native-resolution glass compositing every frame has a real cost. Future work should profile filter passes/resource bandwidth on the target phone and investigate safe backdrop reuse, with invalidation and glass-over-glass correctness tests.

The initial installed Debug app was captured on an iPhone 15 Pro Max, iOS 27.2, for 15.79 seconds with the stationary stress screen. After the first four seconds, 10,488 ms of main-thread samples include 4,240 ms in display traversal, 2,156 ms in layout, 1,321 ms in tint resolution and 795 ms in recursive corner updates. **Inclusive counts overlap**. Atomic reference-count increments/decrements are the largest exclusive leaves. [Phone stacks](phone-installed-profile.json) identify candidates; they are not a controlled baseline for the Release Mac measurements. Device connectivity was lost before installing the new scrolling fixture; the user explicitly chose to continue with Mac measurements. No phone before/after thermal or power improvement is claimed.

The supplemental Mac baseline Time Profiler export is retained as `mac-before-profile.json` and compressed XML. Instruments reported an ancillary corrupt/incomplete unified-log stream while saving that trace; its time-profile table exported successfully. Timing results come from separate successful benchmark runs, not that capture. Apple's [Metal profiling guidance](https://developer.apple.com/documentation/xcode/analyzing-the-performance-of-your-metal-app/) motivates separating CPU, GPU and scheduling costs.

## Reproduction

Use the matching Skia release archive and headers (this machine: `Submodules/skia/out/mac-arm64`):

```sh
cmake -S Demo/GlassComparison/RasterTests -B /tmp/nxkit-stress-tests \
  -DNXKIT_SKIA_RASTER_ARCHIVE="$PWD/Submodules/skia/out/mac-arm64" \
  -DNXKIT_SKIA_RASTER_HEADERS="$PWD/Submodules/skia" -DNXKIT_SKIA_DEBUG=OFF
cmake --build /tmp/nxkit-stress-tests --target MetalGlassStressPerformance GlassStressPerformance -j6
NXKIT_METAL_BUNDLE_DIR="$PWD/Demo/build/macos/ThorVGApp-metal-macosx" \
NXKIT_METAL_REQUIRE_ARCHIVE_HIT=1 NXKIT_METAL_BUNDLED_ONLY=1 \
  /tmp/nxkit-stress-tests/MetalGlassStressPerformance /tmp/stress.csv --scale 3
/tmp/nxkit-stress-tests/GlassStressPerformance /tmp/stress-input --record-only
```

`measure.py` runs the full paired matrix given frozen `--before`, `--after`, `--bundle` and `--output` paths. Run it without competing builds or GPU workloads. The native benchmark's deterministic scroll offsets complement the separate 64-frame real SDL down/move/up replay; they do not replace input/scroll-physics tests.

## Verification

See `validation.json`, `pixel-validation.json`, `workload-validation.json` and `provenance.json` for completed checks and source/binary hashes. Four CPU-raster images (light 1×/dark 2×, initial/scrolled) are byte-identical. This verifies those retained images, not universal Metal pixel parity. Frame-pacing tests run with the override both disabled and enabled. Drawing regressions cover dirty descendants of clean parents, draw-time sibling invalidation, inherited/explicit tint, context restoration and direct-layer concentric corner changes.

The freshly built native macOS Demo was also launched directly with automated scrolling and the uncapped/strict shader flags. A bounded three-second `sample` capture contains vertical/rail `setContentOffset` calls from the stress replay and normal Metal/glass recording, with no software sleep/wait samples; the process exited cleanly. The native app remains subject to drawable pacing. Its separate Game Performance Instruments recording stalled while finalizing and was terminated; no results from it are used. The direct sample is compressed in `data/mac-demo-direct-sample.txt.gz`.

Both macOS and signed iOS RelWithDebInfo Demo builds pass. The iOS build was not installed. All 13 focused regressions pass; full names/results are in `data/final-tests.log`.
