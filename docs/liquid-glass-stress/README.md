# Nested Liquid Glass stress test — 2026-11-02

The Demo's **Glass → Glass stress · 96 cards / nested scrolls** entry contains one vertical `UIScrollView`, eight horizontal `UIScrollView`s and 96 interactive regular/clear glass cards. The fixed glass toolbar samples the moving feed; its Backdrop action changes live content. Tap cards to update their labels. Drag through the cards vertically or horizontally. Direct launch uses the environment variable `NXKIT_GLASS_STRESS=1` (also usable in an Xcode scheme).

The application now renders and presents **every paced frame**, including a clean retained tree. The old clean-tree framebuffer skip is removed globally. Ordinary pacing is retained; the stress controller requests 60 Hz while visible. View/layer content regeneration still honors `setNeedsDisplay()`, so continuous compositing does not repair an omitted content invalidation.

## Measured paths and changes

1. **Clipped subtrees were fully traversed.** The baseline prepared 1,683 layers and 97 glass layers every frame, even though only a handful intersected the viewport. `CALayer::skiaRender` now rejects a subtree before corner clipping, filters and callbacks only when `masksToBounds` proves all output is bounded. Unclipped parents still traverse overflowing descendants, shadows and effects. The render matrix includes presentation transforms and scroll offsets; no persistent visibility cache is introduced. The null-canvas replay's median drops to 168 layer visits and seven glass preparations. The layout walk still visits 1,665 views.
2. **Every layer copied and stable-sorted its children.** Already ordered children now avoid stable-sort scratch work. The ownership snapshot is retained because a custom draw callback can remove or reparent a sibling. Changed z-order still uses stable sorting. This is a small allocation improvement, not the main measured bottleneck.
3. **Glass shading recomputed continuous geometry.** Distance and normal previously evaluated the same superellipse powers and roots separately. The material, exterior contour and merged-container geometry now share one evaluation. Inactive press lighting is skipped at zero intensity; an unused sharp backdrop sample is avoided when the host supplies full shape coverage. No resolution, blur, refraction, sampling-radius or material-quality reduction is used.
4. **Nested rails swallowed vertical drags.** Scroll pans now reject travel along a non-scrollable axis before they prevent an enclosing orthogonal pan. The existing general pan threshold and velocity behavior remain. The replay fails if a rail does not move, the feed does not move, or a card/toolbar action is not delivered exactly once. Immediate capture of an already-decelerating nested rail before direction is known remains an arbitration limitation; this work does not claim complete UIKit nested-gesture parity.

Apple explicitly permits orthogonal nested scroll views in its [scroll-view guidance](https://developer.apple.com/design/human-interface-guidelines/scroll-views). Rejection uses Skia's conservative [SkCanvas::quickReject](https://api.skia.org/classSkCanvas.html), with clipping ownership checked by NXKit.

## Results

Raw per-frame reports, aggregate medians, pixel comparisons and source hashes are retained in [metrics.json](metrics.json), [pixels.json](pixels.json) and [provenance.json](provenance.json).

| Workload | Before median CPU frame | After median CPU frame | Reduction |
| --- | ---: | ---: | ---: |
| 64-frame SDL replay, null canvas (preparation only) | 26.63 ms | 6.60 ms | 75.2% |
| Light, 320×640 points at 1×, raster snapshots | 1,582.74 ms | 1,378.29 ms | 12.9% |
| Dark, 320×640 points at 2×, raster snapshots | 5,889.04 ms | 5,179.88 ms | 12.0% |

These are CPU wall measurements on a shared host: NXKit is Release (`-O3`), but the available CPU Skia archive is a **Debug raster build**. They are **not physical-device GPU frame times or application FPS**. Before and after run sequentially; one cold frame is excluded. Each raster snapshot workload has five warm frames, so timings are indicative rather than a statistical performance guarantee. The null canvas maintains transforms/clips but discards raster shading; its report backend is named `null-recording`. Inclusive glass/render timers overlap and must not be added together.

Visible glass shading remains the heaviest measured raster path: after optimization it accounts for about 98% of light raster frame time. Skipping invisible work has a much larger relative effect on CPU preparation. Full-tree view layout, lifted-lens rendering, physical Metal/Dawn shader cost, energy and thermal behavior remain future profiling work.

Initial and scrolled screenshots are **byte-identical before/after** in both tested configurations (zero changed RGB channels). This proves those retained raster cases, not universal GPU pixel parity. Inspect [light initial](after-light-initial.png), [light scrolled](after-light-scrolled.png), [dark initial](after-dark-initial.png) and [dark scrolled](after-dark-scrolled.png); matching before images are retained alongside them.

The complete raster replay also runs 64 unclipped application frames through real SDL down/move/up routing: an outer vertical drag, every horizontal rail, a card press/release, toolbar interaction and simultaneous offsets. All 64 frames present and both actions run exactly once. Its per-frame data is retained as `after-replay.jsonl`.

## Reproduction

Use matching raster Skia headers/archives and SDL3. This machine reused `/tmp/nxkit-container/raster-tests`, configured with `NXKIT_SKIA_DEBUG=ON`, `NXKIT_SKIA_RASTER_HEADERS=/tmp/nxkit-skia-raster-headers`, and the `Submodules/skia/out/mac-arm64-angle` archives. The normal source headers must match whichever archive you use.

```sh
cmake -S Demo/GlassComparison/RasterTests -B /tmp/nxkit-stress-tests \
  -DNXKIT_SKIA_RASTER_HEADERS=/path/to/matching/skia/headers \
  -DNXKIT_SKIA_RASTER_ARCHIVE=/path/to/matching/raster/archives \
  -DNXKIT_SKIA_DEBUG=ON
cmake --build /tmp/nxkit-stress-tests --target GlassStressPerformance RenderTraversalTests -j 6

# Fast input/traversal regression; no raster/GPU throughput claim.
/tmp/nxkit-stress-tests/GlassStressPerformance /tmp/stress-preparation --record-only
# Full optical rendering and interaction replay; slow on Debug CPU Skia.
/tmp/nxkit-stress-tests/GlassStressPerformance /tmp/stress-raster
# Deterministic comparison snapshots without time-dependent touch deformation.
/tmp/nxkit-stress-tests/GlassStressPerformance /tmp/stress-dark --snapshots-only --dark --2x

python3 Demo/GlassComparison/summarize_glass_stress.py BEFORE.jsonl AFTER.jsonl
ctest --test-dir /tmp/nxkit-stress-tests -R 'GlassStressReplay|RenderTraversalTests' --output-on-failure
```

`baseline-renderer.patch` reverses only the five renderer optimizations against this working tree, retaining the new screen, instrumentation, continuous rendering and scroll arbitration fix. To reproduce the before renderer, apply it with `git apply --unidiff-zero` in a disposable checkout, rebuild, capture the same workload, then discard the checkout. It is an evidence artifact, not a patch to apply to normal development. The baseline used for the retained measurements was linked from frozen source objects without modifying the working tree. Source hashes identify both renderers.

## Validation and platform limits

All 19 focused checks pass: material, container, corner, appearance, rim, shadow, lens, refraction, lens motion, layer sampling, glass interaction, gallery, glass screen, render traversal, application scale/pacing, gestures, scroll physics, scroll indicators and the SDL stress replay. The traversal harness checks actual overflowing-child pixels, transformed clips, scroll origins, reveal/reparenting, stable equal-z order, changed z-order, balanced canvas saves and callback-driven hierarchy removal. The startup harness checks clean-frame presentation at 2×/3× and display-link pacing. See [validation.json](validation.json).

The **iOS Debug Demo compiles and links successfully with signing disabled**. The macOS Demo cannot configure because the expected `Submodules/skia/out/mac-arm64/libskia.a` Metal archive is absent. Physical iPhones/iPad reported unavailable during the final device check. No physical GPU/thermal run or Switch run is claimed. The installed device app was not replaced.
