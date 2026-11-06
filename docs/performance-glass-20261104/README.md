# Sustained glass performance — 2026-11-04

Working tree after `fb556df`, including the prior eased ScrollPocket gradient.

## Retained changes

- Cache resolved glass materials for the current size and grouped material size.
  Appearance/effect changes invalidate them. Animated sizes continue to resolve
  current presentation geometry.
- Share immutable material descriptions between model/presentation layers and
  grouped drawing. Eliminate shader-string and uniform-map copies in those
  frame paths. Returned public descriptions retain independent value semantics.
- Borrow uniform arrays inside the renderer instead of allocating a vector for
  every read. Each draw retains its immutable owner for the borrowed lifetime.
- Restore idle frame skipping before drawable acquisition/submission/presentation.
  Events, dispatch, animations, layout and pending backdrop completion continue.
  Visual changes, active animations and platform exposes wake rendering; failed
  drawable acquisition leaves the frame pending.
- Generate **ThorVGApp Performance** when building the Xcode Demo target.
  Run/Profile use symbolized `RelWithDebInfo`, disable Metal API validation and
  capture, and default to `NXKIT_UNCAPPED=0`. Rebuilds preserve user launch
  environment/arguments while refreshing generated target IDs. The ordinary
  debugging scheme is retained.
  CMake's own OFF/Disabled scheme output verifies the XML values 1 and 3.

Select **ThorVGApp Performance** for sustained device runs. Enable
`NXKIT_GLASS_STRESS_REPLAY=1` for the repeatable 96-card feed/eight-rail workload;
remove it for ordinary Demo use. `NXKIT_UNCAPPED=1` remains available solely for
throughput measurements; it also forces continuous rendering. Normal use now
skips unchanged frames, as newly requested. `NXKIT_CONTINUOUS_RENDERING=1`
provides paced continuous profiling. The stress replay executable explicitly
sets this flag so its work cannot disappear after idle skipping.

The discarded tighter-input-bound experiment showed no consistent GPU benefit
and a 6% dark idle completion regression. Its measurements are under
`experiments/tight-input-bound`; it is **not** part of the final renderer. The
retained change preserves original optical sampling bounds, kernels and colors.

## Device observations

The initial already-running iPhone 15 Pro Max / iOS 27.2 app was sampled for
30 seconds. Instruments reported **Serious** thermal state throughout its
31.375-second thermal interval. CPU stacks include substantial Metal API
validation (`MTLDebugRenderCommandEncoder`, argument/protocol validation) under
frame submission.

A separate 25-second capture relaunched the installed app with
`MTL_DEBUG_LAYER=0`, `NXKIT_GLASS_STRESS_REPLAY=1`, `NXKIT_UNCAPPED=0`.
Glass drawing accounts for 5,646 ms of the main thread's 18,446 ms sampled
weight after the first four seconds; material resolution accounts for 1,241 ms
inclusively. These overlapping weights cannot be added. The initial screen and
replay differ: they are **not** an A/B measurement of the validation toggle or
this patch. CPU summaries and the original thermal interval are retained here.

Strict device replay also exposed four MSL variants absent from the Mac raw
catalog. `recover-device-sources.py` reconstructs only complete, consecutively
numbered diagnostic blocks using Skia's lossless strict newline formatting.
Each recovered source occurs identically at least 136 times; ten interleaved
blocks are rejected. Hashes and device/build provenance are retained separately.
The final bundle contains 125 libraries and 102 native pipeline descriptions.
A corrected-path device capture using the rebuilt c216 bundle produced only a
manifest (no runtime source/pipeline exports) and no pipeline-compile messages;
physical strict replay verification is recorded separately below.

## Verification

Material snapshot isolation, glass appearance, containers, rim, lens refraction,
ScrollPocket, navigation, interactive pop, tabs and Core module-boundary tests
pass. All twelve frozen/current raster images are byte-identical: light/dark
stress screens at rest and scrolled, plus eight navigation arrangements.

The final frozen shader ABI is
`c216e8e08cf413bd18f2d2478b6a5199c9bd3133b53571161d7460043ef83668`.
Fresh 1×/2×/3× light/dark edge coverage and two full-screen navigation profiles
cover 7,310 frames, encountering 90 library sources and 72 pipeline descriptions.
Retained raw source/descriptor coverage has explicit provenance; GPU archives
are rebuilt under the final ABI, never relabelled from an older ABI.

## Reproduction and limits

Freeze prior release harness executables and their matching shader bundle
before editing production sources. Build the headless harness against matching
Release Skia. Run `run-paired.py --batch <capture-dir> --build <headless-build>
--bundle <fresh-macos-bundle>`. It alternates baseline/current order across three
repetitions of dark 3× and light 2× stress and navigation workloads, requires
archive hits and reports medians of warm-pass medians. Stress traversal/draw
counts must match for every frame. The baseline is `fb556df` plus the preceding
ScrollPocket easing patch, not the older pre-CPU-optimization renderer.

Device CPU capture:

```sh
xcrun xctrace record --template 'Time Profiler' --device <UDID> \
  --time-limit 30s --output device.trace --attach ThorVGApp
xcrun xctrace export --input device.trace \
  --xpath '/trace-toc/run[@number="1"]/data/table[@schema="time-profile"]' \
  --output device.xml
python3 docs/performance-stress-20261104/summarize_profile.py device.xml
```

Mac offscreen stress totals include synchronous GPU completion; navigation CPU
values exclude that wait. Neither measures presentation latency or iPhone power,
battery temperature or sustained thermal improvement. Live glass/filter work
remains substantial during motion. Idle GPU rendering is now skipped in normal
use; CPU event/layout polling still runs.
Switch performance is unmeasured. Raw captures/build logs are in
`/private/tmp/nxkit-glass-sharing` and `/private/tmp/nxkit-sustained-perf`.

## Paired Mac results (material sharing, continuous rendering)

Three alternating baseline/current repetitions, 14,520 total frames, require
archive hits with zero runtime compiles and matching stress workload counts.
Medians of warm-pass medians:

| Stress workload | CPU before → after | Change | Synchronous total change |
| --- | --- | --- | --- |
| Dark 3× idle | 1.508 → 1.461 ms | −3.1% | −7.4% |
| Dark 3× scrolling | 2.407 → 2.363 ms | −1.8% | +2.1% |
| Light 2× idle | 1.369 → 1.319 ms | −3.6% | +0.4% |
| Light 2× scrolling | 2.234 → 2.121 ms | −5.1% | −0.2% |

Navigation CPU changes range from +0.7% to −3.6%; this establishes no broad GPU
completion win. See `paired/summary.json` and compressed raw CSVs. These measure
material sharing with full frame work retained, independently of idle skipping.

## Idle rendering regression coverage

`ApplicationScaleTests`, its continuous/uncapped variants, layout, paced/uncapped
frame timing and the stress input replay pass. `DesktopResizeTests` also passes
on the native macOS window path. The application-loop test explicitly checks
120 clean iterations, mutations, active animation/settling, failed acquisitions,
unchanged-geometry exposes, resize reentrancy, hit testing and 2×/3× portrait/
landscape geometry. Continuous variants acquire/present every iteration.

## Idle profile and physical replay follow-up

Two 20-second native macOS captures use the same unoptimized normal Demo binary
and the same stationary stress screen, with Metal validation disabled. Only
`NXKIT_CONTINUOUS_RENDERING` changes. The stress screen keeps a 120 Hz display
link active in both cases. In the identical 4–20 s sample window:

| Main-thread inclusive sampled weight | Continuous | Normal idle skip |
| --- | --- | --- |
| All main-thread samples | 9,630 ms | 3,347 ms |
| Layer rendering | 4,513 ms | 0 ms |
| Glass drawing (within rendering) | 2,915 ms | 0 ms |
| Frame submission | 3,087 ms | 0 ms |

Main-thread sampled CPU weight falls **65.2%** in this single pair. These are
sampling weights, not frame latency; overlapping groups cannot be added. Normal
idle still traverses layout/display preparation (1,567/1,060 ms respectively),
so further CPU savings remain possible. This unoptimized, deliberately busy
stress display-link configuration is not a release iPhone energy measurement.
Both traces report a corrupt/incomplete **logging** stream; their CPU tables
export successfully and contain samples spanning the complete requested window.
Raw CPU exports and profile issue logs are retained alongside the summaries.

The signed `RelWithDebInfo` iPhone build was rebuilt and installed after idle
skipping. A roughly 31-second archive-only stress replay loads all 102 bundled
native pipelines, reports no missing library/pipeline errors, and exports only
the current c216 manifest into a fresh harvest directory. It uses no writable
runtime shader cache (`NXKIT_METAL_BUNDLED_ONLY=1`). This covers the stress replay,
not every possible navigation/content state.

Post-change iPhone CPU attachment by name/PID failed despite the app being
listed as running. A bounded system recording then stalled while saving and
was stopped; it is excluded from measurements. The earlier active replay trace
(`phone-final-summary.json`, before idle skipping) remains Serious throughout
its 32.787-second thermal interval. Thus **no sustained iPhone thermal improvement
is established**. The phone is returned to the ordinary optimized Demo, without
stress replay or forced continuous rendering. Both normal macOS and signed iOS
Demo builds succeed; focused regressions and `git diff --check` pass.
