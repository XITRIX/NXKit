# Sustained iPhone headroom investigation — 2026-11-06

Target: actual drawable presentation near 120 Hz (8.33 ms), with GPU headroom during sustained full-screen Demo scrolling. Device profiling authorized by the user after the earlier Simulator-only work.

Device: iPhone 15 Pro Max, iOS 27.2, maximum display rate 120 Hz, Low Power Mode off.

## Installed baseline (ABI 4080b098)

| Run | Thermal state during first / second scroll | First scroll FPS / GPU p50 | Second scroll FPS / GPU p50 |
| --- | --- | --- | --- |
| Full effects | Nominal / Nominal | 69.66 / 9.25 ms | 78.73 / 9.09 ms |
| Pockets disabled | Fair / Serious | 117.93 / 6.52 ms | 96.30 / 9.28 ms |
| Glass disabled | Serious / Serious | 117.38 / 6.07 ms | 112.24 / 8.52 ms |

These are sequential diagnostic controls, not an additive decomposition or a thermally matched A/B. Both materials and pockets are significant. The later controls reached Serious thermal state; their clocks cannot be assumed equal to the Nominal baseline. GPU values are asynchronous command-buffer execution intervals. FPS comes from drawable presentation callbacks.

## Work in progress

Bounded analytic content masks preserve only their affected prefix regions, draw content once, and restore the prefix with inverse-mask clip coverage and Src blending. The fast path requires a hard rectangular clip and a viewport exactly matching the current device boundary. Other geometry retains initialized viewport isolation, including its antialias and descendant filter boundary semantics. No effect strength or rendering resolution is reduced.

The finite device replay now supports 12–600 seconds in its second scrolling phase. Its analyzer reports 30-second windows with presentation intervals, GPU times and thermal state. Sparse, separately enabled Metal stage counters attribute render-pass cost; counter runs are not clean frame-rate validation.

Reproduction:

```sh
python3 Demo/scripts/measure_ios_demo.py --device <UDID> --output /tmp/ios-sustained --scroll-seconds 300
python3 Demo/scripts/measure_ios_demo.py --device <UDID> --output /tmp/ios-passes --gpu-passes --scroll-seconds 30
```

The harness always restores an ordinary app launch. Production rendering remains event driven.

## Prototype controls and optical checks

The sparse-counter prototype in landscape recorded 91 render passes in sampled steady frames; the explicit-reduction prototype recorded 54 in portrait. Pass-count comparisons identify reduced graph duplication, but these orientation-different controls do not establish a matched GPU speedup. Portrait full-effect runs in Serious thermal state improved from 78.06/73.93 FPS (bounded mask only) to 109.79/98.33 FPS (explicit glass reductions, sparse counters enabled). Neither establishes sustained 120 Hz.

Explicit levels retain their original native stencil, capture density, source lattice, sampling halo and optical values. Aligned 2x/3x image comparisons, including transparent content and long/short shapes, allow bounded intermediate rounding (maximum 11/255, whole-scene mean absolute error 0.00377/255 in Metal); fractional and affine configurations use the original lazy path. Independent native step/stripe controls retain exactly the prior aggregate errors (0.363281 and 0.5625).

Rejected experiment: finishing broad scroll-pocket convolution at one pixel per point reduced measured impulse variance beyond the existing four-point-kernel tolerance. F16 intermediates did not fix the regression. Both variants were reverted; the original progressive kernel remains.

The profiling prototypes used runtime shader fallback during development; final sustained testing must use the freshly baked shader bundle and confirm compilation counters.

## Verified source and regression coverage

All twelve focused Simulator suites and the uncapped control passed, including 213 scroll-pocket checks, 64 aligned/fractional/affine glass-image comparisons on each raster/Metal backend, independent native blur controls, grouped material lanes, navigation, interactive cancellation/commit and tabs.

The full five-pass two-scale capture uses ABI `f6ee5294`. The release ABI `2cbccf6d` adds only a defensive null-canvas condition for an optional prefix; its 46 freshly captured source hashes are a subset of the verified 101-source capture. The merged catalog retains content-addressed raw MSL and complete pipeline descriptors from both and the prior transformed/ink-blur fallback catalog. All distributable libraries and GPU archives are compiled afresh for the release ABI; no old GPU binary is relabelled. Final cold packaged tests still validate the release runtime itself.

Cold packaged Simulator verification completed for both light/dark themes: scrolling, every navigation arrangement, interactive cancellation/commit, and large titles. All 13,028 recorded frames use zero runtime source-library compilations; the 179 pipeline objects are prepared from bundled IR before rendering and the count stays constant throughout. Simulator has no native binary-archive architecture; the physical iPhone test separately requires archive hits with runtime compilation disabled.

## First packaged sustained check (not the target result)

Release ABI `2cbccf6d`: native bundled-only verification succeeds with runtime compilation disabled and zero pipeline failures. The short portrait check reaches 117.42/116.80 FPS in Serious thermal state. A following 180-second scrolling phase settles at 94.77 FPS, with median GPU envelope 10.93 ms; thermal state remains Serious. This fails the sustained 120 Hz/headroom target. No completion claim is made from the short run.

The next optimization bounds each frozen reduction by the finite nonzero support of its decal source, propagating full convolution/reconstruction reach at every level. It removes guaranteed-transparent work outside the actual device, rather than truncating the blur kernel at the viewport.

The finite-support prototype passes the same 64 optical image comparisons on each backend and both independent native blur controls, without changing their reported errors. Support begins at the actual device's decal-source bounds and grows by each complete stencil/reconstruction footprint, so valid blur spilling beyond the source is retained.


## Additional source reductions

Circular SDFs use their exact radial equation away from the original centre singularity. Raster and Metal distance/normal maps at 1x/2x/3x retain a maximum 1/255 difference from the frozen previous shader. Grouped-glass early rejection preserves every possible normal-dependent joining region; it does not assume the blended field is globally a unit-distance field.

Native glass starts sampling at pyramid level one when the initial prefilter is active, even for its nominally sharp sample. Those materials now omit the unused level-zero atlas tile. Capture, stencil, tile positions and all sampled levels remain intact. Generic backdrop programs keep the full atlas. The 64 image comparisons on both backends retain exactly the preceding maximum/mean error, including fractional/affine cases.

Frame pacing now retains its schedule across small wake-up overruns instead of accumulating scheduler jitter. Missed intervals are skipped after a stall. The regression models repeated 0.35ms wake jitter, input bursts, rate changes and uncapped operation.

Rejected investigation: bounding the optional custom foreground-vibrancy isolation did not explain this Demo's cost; the feature is disabled in this screen. The experimental change was removed.

The iOS backend also avoids redundant native Metal-layer configuration and Core Animation geometry transactions when drawable size and scale are unchanged. Rotation/resizing still updates them from the live geometry sample.


## Second packaged sustained check (still short of the target)

ABI `b6efa3d7`, signed package SHA-256 in `iphone-build.json`: bundled-only native execution passes with zero library/pipeline compilations and zero pipeline failures. The cold first scrolling phase reaches 119.60 FPS. Its 180-second sustained phase averages 111.42 FPS: successive windows are 119.97, 116.71, 107.84, 107.24, 107.70 and 109.04 FPS. GPU p50 grows from 5.51 ms to about 9.94 ms, with late p95 about 12 ms. Thermal state changes from Fair to Serious. This still fails sustained 120 Hz and the thermal target; further optimization is required.

All 13,028 packaged Simulator verification frames also have zero source-library compilations and a constant 185 prepared pipeline objects. The actual Simulator app run-loop replay and corner-configuration suite pass. The first ordinary-launch restoration returned a CoreDevice launch-service error; a fresh ordinary launch succeeds and its receipt is retained.


## Settled lens work (device validation pending)

Sparse device counters attribute a late full-screen pass to Skia's MSAA load pipeline. The selected tab's clipped background and destination-dependent foreground restart MSAA over the full drawable. Settled lenses now compose this work in a target bounded by live lens geometry, with a two-device-pixel coverage halo. Rotated, nonuniform, custom-drawing and unsupported clipping configurations retain the original composition.

The source picture is still recorded in full, preserving callbacks and overscan. For plain-layer settled sources, only the sampled raster region is drawn into a temporary texture. Its transform retains the full picture's original integer raster lattice and texture-size cap, plus the bilinear footprint. The actual 3x Demo reports 48,888 source pixels, compared with the captured 1,548 × 570 full source. Lifted/moving optics keep the existing source path.

The 480-case image comparison per backend covers plain image sources and custom callbacks, 1x/2x/3x, fractional placement, light/dark, transparency, rotation, nonuniform transforms and antialiased ancestor clips. Raster has a maximum 1/255 difference and MAE 0.00000105019/255. Metal body pixels remain within one code value. Whole-composition retargeting changes one edge pixel in a 2x case by 23/255 from MSAA sample quantization; aggregate MAE is 0.00000192212/255. The explicit budget limits such differences to at most four pixels within one device pixel of the contour and one 4x-MSAA coverage sample; a broad or displaced fringe fails. The cases include 2,500-point-wide sources to exercise the original raster-size cap. All twelve focused behavior/optical suites, the uncapped frame-pacing control, corner configuration and the metrics-analyzer regressions pass; logs are in `lens-regressions/`.

Simulator ABBA source-crop controls do not establish a stable speedup: full-source GPU p50 is 1.244/1.231 ms and cropped is 1.225/1.008 ms. Device tile bandwidth and sustained thermals remain the decision criteria. See `lens-roi-paired.json` and `pass-attribution.json`.

Two alternatives were rejected: selecting global raster path antialiasing increased Simulator CPU/GPU cost; precomputing progressive-Gaussian weights produced no measurable GPU improvement (1.211 ms versus 1.207 ms). Neither is enabled in production. Progressive blur kernel, progression and resolution remain unchanged.


The settled-lens release ABI is `6f2ef493`. Its fresh eight-workload capture contains 103 raw source programs and 79 pipeline descriptions; the additive fallback catalog contains 249 libraries and 186 pipelines. All libraries are rebuilt for this ABI. Cold packaged Simulator verification passes all 13,028 frames with zero runtime library compilations and a constant 186 prepared pipeline objects (`cold-packaged-lens-summary.json`).


The first strict native run rejected two uncaptured A17 Pro source variants and was stopped; its FPS is invalid. A native replay captured those sources plus one pipeline (`lens-native-raw/`). The merged catalog now has 251 source libraries and 187 pipelines, all rebuilt offline. The five-minute prototype with the same rendered lens code averages 116.95 FPS in its sustained phase, median GPU 7.21 ms. It remains in Serious thermal state while charging. This still fails stable 120 Hz/headroom; its two source compilations and one pipeline compilation occur at startup and remain constant thereafter. It is a development control, not final bundled-only validation (`lens-prototype-sustained-summary.json`).


The complete native package passes strict archive verification: zero runtime source/pipeline compilations and zero failures. Sparse counters confirm the late full-screen lens reload has become a 512 × 256 pass; its source image is 312 × 168. All steady frames use the bounded lens/group and cropped source paths. A separate earlier full-screen MSAA load remains immediately after an empty full-screen clear. This is the next backend investigation, not an implemented speedup (`lens-pass-attribution.json`). The counter run is for attribution, not a clean FPS comparison.


## Known-clear Metal reload (validation in progress)

The command buffer remembers only a full, finite clear with no drawing pipelines and a stored single-sample result. Read-only copies from that exact texture preserve the fact. Rendering, compute, buffer writes, uploads, semaphore handoffs and potentially aliasing copies invalidate it. Parent/buffer-backed views are excluded; copy destinations are checked for parent and shared IOSurface aliases. At a matching-format MSAA reload, a hardware attachment clear replaces the fullscreen load draw. State never crosses command buffers.

The 48-case clear/snapshot/write fixture covers N32/F16, sRGB, transparency, replacement clears and intervening writes. All 4,718,592 output bytes match the original path exactly. The final full optical suite also retains the prior native step/stripe and frozen-pyramid errors. Simulator ABBA GPU medians are effectively unchanged (original 1.1125/1.1661 ms, optimized 1.1760/1.0958 ms); no Simulator speedup is claimed. A full-screen 1206 × 2622 load is elided in its diagnostic workload. Native sustained validation remains required.

Current-ABI captures contain 48 source libraries and 34 pipelines, all exact subsets of the prior verified source/descriptor catalog (`known-clear-raw-subset.json`). Only backend load handling changes. All 251 packaged libraries and 187 pipeline descriptions are rebuilt for the current ABI; the prior raw catalog retains complete navigation and A17 Pro coverage.


ABI `2360ac38` passes all 13,028 cold packaged Simulator frames with zero source compilations and a constant 187 prepared pipeline objects. Twelve focused regressions and the uncapped control pass. The actual Simulator app replay and macOS smoke build pass. A deterministic Simulator descriptor-limit failure in URL-backed loading is fixed by loading bundled IR from owned dispatch data; the complete catalog now starts and renders successfully.

The strict five-minute iPhone run confirms a 1290 × 2796 known-clear reload is elided and has zero runtime source/pipeline compilations. First scrolling reaches 119.97 FPS, sustained scrolling 117.96 FPS with GPU median 6.80 ms. Charging continues throughout, with thermal state rising from Nominal through Fair to Serious. Late windows remain about 117–118 FPS. This still fails the requested sustained headroom; work continues on removing the initial clear/snapshot flush.


The next source-level optimization reads a pending uniform Graphite clear without flushing it. Eligible scroll fades restore its quantized, premultiplied device values through a constant shader instead of forcing a clear/snapshot pass. Pending draws/uploads and unsupported pixel formats retain snapshots. No UI geometry or material values are assumed. Read-only private metadata accessors add no fields or virtual methods.

A 288-case GPU comparison covers 1x/2x/3x, N32/F16, sRGB, translucent clears, group opacity, fractional scroll origins and nested blur. Callbacks run once; maximum difference is 1/255 and MAE 0.00051241/255 over 66,060,288 values. Full optical regressions retain the prior native blur errors. The actual Demo confirms one uniform prefix while scrolled under the bar; the resting endpoints have no active fade and zero prefixes. Device validation is pending.


The uniform-prefix prototype (ABI `5c4279fd`) confirms this path on the physical phone: nearly every sustained scrolling frame uses one uniform prefix. Five-minute scrolling averages 119.29 FPS, GPU median 6.42 ms and p95 8.70 ms; the final window falls to 118.24 FPS. The phone remains charging and reaches Serious thermal state. This still fails the sustained headroom target. The prototype captures 70 source libraries and 56 pipelines at startup; these counts stay constant during the sustained phase. It is a development capture, not a bundled-only release check. The verified offline build was restored afterward.


## Directional progressive-blur support

Runtime image filters can now request independent x/y sample extents. Existing scalar factories retain their semantics. The progressive edge passes use the actual sampling axis; convolution weights, local sigma, prefilter, mask and resolution are unchanged. The extension keeps output unrestricted. Serialization stores the larger scalar radius using the existing format, conservatively widening allocation hints on reload without changing pixels. Invalid or non-finite extents and invalid child lists are rejected.

An initial 96-case comparison per backend covers 1x/2x/3x, horizontal/vertical and reversed gradients, fractional placement, transparency and narrow clips. Raster maximum difference is 1/255, Metal 2/255; mean absolute errors are 0.0000000807 and 0.0000796/255. Extended serialization comparisons are included in the full regression run. Alternating Simulator GPU medians are 1.1547/1.1355 ms for isotropic support and 1.1517/1.1040 ms for directional support; this small noisy difference is not a physical-device speedup claim. Native sustained validation is in progress.


The directional-support prototype (ABI `55c0bcee`) completes five minutes on the phone at 119.72 FPS. GPU median is 6.11 ms, p95 8.29 ms, p99 9.42 ms. The first sustained windows are 119.96/119.97 FPS; late windows range from 119.31 to 119.97 FPS. The phone charges throughout and enters Serious thermal state. Runtime capture counts stabilize at 70 source libraries/56 pipelines before sustained scrolling; this is a development capture, not final archive validation. It improves the observed run but still does not establish the requested thermal headroom. These are sequential runs under changing thermal conditions, not a matched causal A/B estimate.

A separate experiment skipped zero-contribution highlight calculations. Simulator alternating medians (1.122/1.151 ms original, 1.092/1.127 ms experimental) do not establish a sufficiently clear benefit. It was removed; no highlight equation change is retained.


The final full optical suite passes, including original/isotropic, directional and serialized-directional comparisons (96 scene cases per backend). Maximum differences remain 1/255 Raster and 2/255 Metal; aggregate means halve because serialized output matches the isotropic control. All twelve focused suites and the uncapped control pass, along with the metrics analyzer and six offline-bake input-validation tests. Profiling output is now buffered, flushing at reporting windows and before export, to avoid hundreds of tiny writes per second during measurement; ordinary runs do not enable this diagnostic writer.


Final package: ABI `55c0bcee`, 253 source libraries, 189 pipeline descriptions, 165,547,392-byte native GPU archive; signed executable/archive hashes and 263 verified sealed resources are in `iphone-build-directional.json`. Shader baking ran offline for the full package; subsequent diagnostic-only incremental compilation reused that exact current-ABI archive. `NXKIT_BUILD_METAL_SHADERS` is restored to ON. No precompiled output was relabelled. All 13,028 cold packaged Simulator frames use zero source compilations and a constant 189 prepared pipelines. The actual Simulator app replay, standalone UIKitCore link test and macOS smoke build pass.


The signed native package passes its initial strict replay: zero runtime source/pipeline compilations, zero failures, 55 archive hits. The second short scroll reaches 119.97 FPS with GPU median 4.99 ms in Fair thermal state. This is an archive/correctness check, not the sustained target result.


The same alternate raster-path antialiasing control is now also rejected on the A17 Pro: 120-second sustained scrolling averages 110.22 FPS, GPU median 9.17 ms and p95 11.22 ms in Serious thermal state. Its 17 source/9 pipeline fallback compilations occur during setup and stay constant throughout sustained scrolling; it is a diagnostic control, not archive-only validation. Production retains Graphite's default hardware MSAA. The private `--raster-paths` profiling switch is retained for reproducibility, disabled by default and cleared on ordinary launch restoration.


## Final strict sustained result — target still not met

The signed package has zero runtime source/pipeline compilations and zero failures across its five-minute strict run. It starts at 119.97 FPS, then falls to approximately 110–113 FPS in the last windows. The sustained aggregate is **115.19 FPS**, GPU median **7.78 ms**, p95 **10.41 ms**, p99 **11.49 ms**. The phone charges and remains in Serious thermal state throughout. This is not sufficient headroom or a thermal-target pass. The lower-overhead diagnostic writer does not remove the late slowdown. An additional hot-state sparse-counter capture is being used to guide further optimization. Ordinary launch restoration succeeds afterward.


A disabled/identity-uniform shader-specialization experiment was removed after an uncontended Simulator ABBA showed no gain: generic GPU medians 1.1398/1.1317 ms, specialized 1.1419/1.1488 ms. An earlier run overlapped optical verification and is discarded. No specialization helper, material-key change or shader-source variation from this experiment remains in production. Hot-state sparse counters still attribute significant time to the root composition, standalone/grouped glass and progressive edge passes; see `directional-hot-pass-attribution.json`. A system-profiler run is being attempted to check memory/CPU alongside GPU throttling.


## Memory and idle-cache follow-up

A separate strict five-minute run averages 119.56 FPS, GPU median 6.41 ms, p95 8.51 ms and p99 9.65 ms. It has zero runtime shader/pipeline compilations and zero failures. Charging and Serious thermal state continue throughout. This remains short of all-frame 120 Hz headroom. These sequential hot-device runs do not establish a controlled percentage improvement.

Low-frequency memory samples show stable allocations across five minutes: process footprint settles near 451.5 MiB, Metal allocation near 309.3 MiB, Graphite budgeted resources near 147.0 MiB. About 99–102.5 MiB of Graphite resources are purgeable. There is no growing allocation trend in this replay (`memory-strict-sustained-summary.json`). GPU allocation queries run every five seconds only while diagnostics are enabled; ordinary app rendering does not query them.

Idle-only cache reclamation now preserves resources used within ten seconds of the last submitted frame. After two seconds without rendering, deferred cleanup gets 0.5 ms per resource provider, at most once per second; it stops when no further resource bytes are reclaimed. It never runs during drawing. Skia also purges its shared trivial-resource cache without the provider time cap.

The Simulator idle/resume replay completes with zero idle GPU frames, zero source compilation and no new pipeline creation. Graphite budget drops from 151.3 MiB before idle to 47.1 MiB after resuming; purgeable resources drop to 6.2 MiB. The first resumed CPU frame takes 3.44 ms. A later resumed frame has a 17.41 ms wall time (1.96 ms rendering, 1.68 ms submission), so this is not an all-frame latency pass. Device validation is pending: CoreDevice lost its trusted connection before installation. The complete current-ABI signed build is ready (`iphone-build-idle-cache.json`).

Instruments Time Profiler could not resolve the running process; Game Performance produced empty CPU/GPU tables. The signed app has `get-task-allow=true`. No CPU-stack or GPU-frequency inference is made from these empty captures.


A second strict device idle/resume replay completes while charging in Serious thermal state. Both 30-second scrolling before idle and the 12-second resumed phase average 119.97 FPS. Before idle GPU median/p95/p99 are 6.20/7.53/8.01 ms; after idle 5.24/7.59/8.02 ms. Idle submits zero frames. Shader/pipeline compilation and failures remain zero. Process footprint falls from 451.0 MiB to 330.7 MiB, Metal allocation from 306.1 MiB to 267.3 MiB, and Graphite budget from 144.1 MiB to 109.0 MiB. The first resumed frame takes 8.39 ms, then the third has a 25.74 ms drawable wait and 29.24 ms total. A cleanup-disabled control is required before attributing that resume hitch to the cache. Short-phase averages do not establish sustained 120 Hz or all-frame headroom.

Direct launch under Instruments succeeds where process-ID attachment failed. Time Profiler records 15,135 samples. In the initial scrolling window, 3,464 main-thread samples put 42.6% of inclusive sampled work in recording/submission and 25.4% in Metal render-pass setup; these nested percentages are not additive. Dynamic tint lookup accounts for only 0.58%. An attempted weak tint-source cache passes focused ownership/reparenting tests but its alternating benchmark has no reliable CPU gain; it is removed completely. No tint inheritance change is retained.

The Game Performance direct-launch capture also returns real GPU state data. Its available 52.455–61.310 second interval contains 1,658 performance-state records, classified Minimum or Medium under active device conditions. Thermal state is Serious throughout the full 61.31-second capture. This supports variable GPU operating-state evidence, but does not establish physical clock units or isolate charging as the cause. Instrumented captures are for attribution, not clean FPS comparisons.

Two further shader experiments were removed: sinking unused interior shadow distance calculations and CPU precomputation of uniform blur LODs. Their alternating Simulator timings do not establish a reliable benefit. The current source and offline catalog retain ABI `55c0bcee`; material equations, blur resolution and sampling remain unchanged.


The cleanup-disabled phone control reproduces the first-gesture stall: its third resumed frame takes 42.29 ms, including a 37.81 ms drawable wait. Its first two CPU frames cost 8.84/7.11 ms, compared with 8.39/5.28 ms with cleanup. Both controls resume at 119.97 presented FPS with zero runtime compilation and idle submissions. This does not prove identical latency distributions under different GPU states, but provides no evidence that cache reclamation introduced the startup wait. With cleanup disabled, Graphite remains near 144.0 MiB after idle, versus 109.0 MiB with cleanup.

The final idle-cleanup source builds on macOS and Simulator; independent Core linkage, layout/drawing, retained traversal, and paced/uncapped tests pass. One Simulator diagnostic run skipped returning to its list after navigation and therefore rendered zero second-scroll/resume frames; that run is invalid. The replay now explicitly restores the list root before its second scroll, and both harnesses reject missing or unrendered motion phases. No FPS claim is made from the invalid control.


## Pressed glass exterior correction

The subsequent [halo correction](../../glass-press-halo-20261106/README.md)
rebuilds the package for ABI `c8d80683` and replaces superseded native material
programs. The current package has 211 libraries and 163 pipeline descriptions,
with a 90,556,256-byte native archive. Its hashes are retained in the halo report;
the earlier build receipts here remain historical. All 13,028 cold packaged
Simulator frames pass with zero source compilations and constant prepared
pipelines. Strict device scrolling/navigation also has zero source/pipeline
compilations and zero failures. A 30-second scrolling phase averages 119.97 FPS,
GPU median 4.95 ms, p95 6.00 ms and p99 6.52 ms in Fair thermal state while
charging. This shorter and cooler run does not establish sustained 120 Hz or a
controlled performance improvement. The reported press halo is fixed by
limiting interaction lighting to the existing glass surface coverage.
