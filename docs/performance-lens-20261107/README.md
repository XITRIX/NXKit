# LiquidLens movement and state performance — 2026-11-07

Working tree after `b895e97`. This review covers lifting, held motion, lowering,
and idle in the full-screen iOS Demo. It also adds a repeatable 2/3/4/5-item
Metal workload. Normal application rendering still skips unchanged frames.

## Findings and changes

The physical iPhone profile attributes the largest cost to drawing and Metal
render-pass setup/submission, rather than the spring calculation. Of 6,724
main-thread running samples after the first five seconds, inclusive stacks
include 2,434 in recording/submission, 1,418 in Metal render-pass setup, 258 in
lens source rendering, 81 in lens advancement, and 13 in tab-row arrangement.
These are overlapping statistical samples, not additive elapsed timers.
The retained XML and attribution JSON contain the complete search boundary.

Lens source capture and both optical stages previously created client surfaces
and took image snapshots. On Graphite these snapshots copied their textures.
They now use immutable child devices and no-copy special-image views when the
backend supports them. The old surface route remains a backend fallback and a
private regression control (`NXKIT_LENS_SURFACE_INPUTS=1`).

The pointwise vibrant foreground compositor now reads a frozen backdrop prefix
and draws its complete result directly, avoiding the zero-radius runtime
image-filter isolation/restore pass. Recording or unsupported devices retain
the filter path (`NXKIT_LENS_FILTER_FOREGROUND=1` forces the control).
Both paths replace their already-composed scene with Src. The previous filter
restore added a translucent scene twice; an empty-foreground identity regression
now verifies unchanged transparent backdrop RGBA on Raster and Metal.

Tab dragging updates only the moving lens when bounds, items, appearance and
lifted layout are unchanged. Position-only lens changes no longer reapply all
internal child frames. Geometry and appearance invalidation still rebuild the
row; this does not cache arbitrary application drawing.

A larger lifted-lens composition group was tested and removed. It preserved
pixels but did not establish a repeatable CPU/GPU improvement. Its measurements
are retained separately. No optical radius, spectral sampling, resolution,
shadow support, or spring curve is reduced by the retained optimizations.

## Simulator before/after

Alternating Original / Optimized / Optimized / Original runs use the same
executable, scene, layout and 2/3/4/5 tabs at 3×. Each run includes an excluded
warm pass and a measured pass. Metal validation is off. Both original-input
controls are enabled together for the original cases; normal pacing/readback
is absent in this offscreen workload. CPU preparation excludes synchronized GPU
completion; `update_ms` measures UI mutation separately. GPU intervals are
completed command-buffer execution time, not inverse frame rate.

| Steady dragging | Original trial medians | Optimized trial medians |
| --- | --- | --- |
| CPU preparation | 3.878 / 4.037 ms | 2.935 / 3.083 ms |
| GPU command buffer | 1.340 / 1.267 ms | 1.068 / 1.103 ms |

The mean of the trial medians falls approximately **24% for CPU preparation**
and **17% for GPU execution**. This does not establish physical-device FPS or
thermal improvement. Raw CSVs and per-phase summaries are retained. Each final
measured pass compiles zero new pipelines. The motion path uses the same
three frozen input stages and native optical equations.

## Physical baseline and remaining limit

A separate clean real-run-loop baseline on the connected iPhone 15 Pro Max
(A17 Pro, iOS 27.2/24B5110a) starts in Nominal thermal state and reaches Serious
within one minute while connected to power (reported battery state Full).
The six four-second steady drag windows deliver approximately 120 presented
FPS. Warm drag command buffers have about 5.6–5.7 ms median GPU execution;
lift windows are around 6.6 ms. UI drawing and submission are each about
1.5 ms during dragging. Total frame wall time includes display pacing and
must not be interpreted as pure CPU work.

This first profiling-only package allows runtime compilation. Startup and a
later lowering transition compile new variants; timing summaries retain those
counters. Warm drag cycles after the first have no compilation. These results
are a workload baseline, not a bundled-only shader pass.

The final optimized device comparison is pending: the iPhone's trusted
CoreDevice connection disappeared during installation. The Simulator gain and
this baseline do **not** prove sustained 120 Hz under thermal throttling or a
cooler phone. The report deliberately leaves that target unverified.

## Reproduction

Build the optimized Demo, then install it on an unlocked iPhone:

```sh
python3 Demo/scripts/measure_ios_lens.py --device <UDID> --output /tmp/lens-original --original-inputs
python3 Demo/scripts/measure_ios_lens.py --device <UDID> --output /tmp/lens-optimized
python3 Demo/scripts/measure_ios_lens.py --device <UDID> --output /tmp/lens-strict --seconds 120 --require-archive
```

The live replay repeats rest, lift, drag and lower for the requested finite
duration, then idles. The analyzer splits contiguous cycles before calculating
presented FPS; it never bridges unrelated phase gaps. The tool requires every
drag phase and terminal idle and restores ordinary Demo launch in `finally`.
A separate `--gpu-passes` run adds sparse pass instrumentation for attribution;
keep it separate from clean timing runs.

The Simulator target `MetalNavigationBenchmark` accepts
`output.csv --lens-motion --quick --passes 2`. `SimulatorGlassBlurTests`
provides `--lens-only`, `--slider-only`, `--switch-only`, and
`--lens-state-output output.rgba` for cross-process complete-frame controls.
The new control gallery has a navigation/settings smoke replay through
`MetalNavigationBenchmark output.csv --controls-gallery --passes 1`.

## Final regression coverage

The corrected filter control and immutable/direct-input path agree across
128 complete-frame cases, including translucent backdrops and rotated/clipped
owners: maximum channel error 1/255, mean absolute error 1.81641e−7/255.
The independent empty-foreground identity case has zero RGBA difference on
Raster and Metal. Cross-backend lens, slider and switch suites pass; these
allow documented edge/coverage differences and do not establish native pixel
parity. Switch regressions also cover 96 attached-trait color cases, independent
native motion traces, tap/drag delivery, cancellation and reentrant callbacks.

The new route is **iOS fullscreen Demo → System UI elements → Slider / Switch**.
Both detail screens provide a live preview, configurable public properties,
backgrounds and an event log. Navigation and every settings action have a
repeatable smoke replay. Full-resolution snapshots are retained as
[Slider](gallery-slider.png) and [Switch](gallery-switch.png).

The source ABI for the performance measurements and package verification above is
`0cb95436b7c436a838a69299ea7620d3b256a1370d11b432a6212fd699292910`.
The additive raw catalog contains 259 content-addressed Metal libraries and
219 complete render-pipeline descriptors. Old GPU binaries are not relabelled:
normal Simulator and iPhone builds compile SDK-specific libraries and GPU
archives offline from these raw inputs. Native well artwork, interrupted
switch release velocity, full accessibility bridging and exact optical pixel
parity remain documented compatibility limits.


The final Simulator build and signed iPhone build succeed. Independent cold-cache,
bundled-only replays pass lens, slider, switch, gallery navigation and tab motion
with zero library/pipeline compilations **during rendering**. Simulator context
creation prepares pipeline objects from the bundled libraries/descriptors; this
is outside those rendering counters and is not a claim of zero startup work.
The final iPhone package embeds the matching ABI and its signature verifies.
Installation and optimized physical measurements remain pending its unavailable
trusted development connection. Logs and assertions are retained in `verification/`.

The later [motion activation follow-up](../liquid-glass-lens/relative-motion/README.md)
fixes resting motion sampling after these measurements. Its native comparisons,
regressions and final package ABI are recorded separately; the timings here
are not a new measurement of that follow-up.
