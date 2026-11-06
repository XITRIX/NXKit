# Full-screen Demo scrolling — 2026-11-06

The iPhone feedback narrows the remaining choppiness to the iOS full-screen
Demo. This round measures that actual scene, retains native material resolution
and optics, and changes only the generated grouped-material program.

## Change

Each group member now reads its own material uniforms and backdrop sampler using
constant indices. Previously every shading lane first copied the entire material
into mutable shader globals, then selected its sampler through a shared active
material variable. The new program gives each lane its own functions and leaves
shared geometry, SDF joining, color ownership and touch lighting intact.

This removes a large mutable material state from the generated fragment program.
It does not change layout, blur kernels, glass settings, foreground rendering,
frame-rate requests, idle skips or the global renderer.

## Measurements and limits

An offscreen Graphite/Metal harness constructs `GlassScreenViewController` with
its actual home list, navigation items, tab bar, lens and scroll pockets at
402×874 points and 3× density. `--scroll` traverses the full content range and
returns using 241 deterministic positions per pass. `--live-appearance` also
polls async backdrop reads and delivers main-queue appearance updates. Pending
entry animations are completed before this steady-scroll control.

Four separate launches run before / after / after / before, three passes each.
Only pass 2 is retained (241 frames per run). Table values average the two run
medians for each variant. Source and pipeline compilation are absent in warm
passes. No build or other test runs concurrently with these measurements.

| Live appearance, warm run | Before | After | Reduction |
| --- | ---: | ---: | ---: |
| CPU recording/submission | 3.945 ms | 3.455 ms | 12.4% |
| Synchronized completed work | 15.149 ms | 13.672 ms | 9.7% |
| Metal command-buffer execution | 1.40973 ms | 1.40969 ms | Effectively unchanged |

The simpler control without repeated appearance delivery improves completed
work by 10.9%; its small GPU-time difference is not reproduced by the live
control. The demonstrated gain is CPU/submission-side work, not a material GPU
speedup. See [individual live runs](live-paired.json),
[aggregate measurements](measurement-summary.json) and compressed frame CSVs in
`raw/`. Metal timing sums valid `GPUStartTime`/`GPUEndTime` intervals for command
buffers issued during each frame; zero indicates unavailable timing if no buffer
exposes valid timestamps.

These are **Simulator offscreen measurements, not delivered iPhone FPS**.
The synchronous completion latency includes Simulator/host scheduling and wait
costs well beyond Metal execution. It must not be inverted to claim display FPS
or interpreted as a phone's GPU frame budget. Host frequencies are not locked.
No physical device benchmarks or installation were performed in this round.
Sustained 120 FPS and thermal improvement on the iPhone remain unverified.

## Isolation and rejected experiments

A first diagnostic removes navigation glass, tab glass, pockets, or all effects
independently. Both bars and the pockets contribute to the full scene's cost;
the effects are not additive. These ordered short runs are diagnostic only,
with no thermal inference. See [isolation measurements](isolation.json).

The following prototypes were removed because they did not produce a reliable
benefit in longer comparisons: one-material shader batches, immutable filter
graph reuse, a neutral-only standalone program, conditional highlight work,
explicit per-frame atlas materialization and 2× glass-only shading. No resolution
or visual-quality tradeoff from those experiments remains. Earlier exploratory
numbers are retained separately from the final live-appearance measurements.

## Reproduction

Build `MetalNavigationBenchmark` with the existing Simulator CMake configuration,
then run sequentially with Metal validation disabled and a nonexistent bundle
override when comparing source programs:

```sh
SIMCTL_CHILD_MTL_DEBUG_LAYER=0 \
SIMCTL_CHILD_NXKIT_METAL_BUNDLE_DIR=/tmp/no-bundle \
xcrun simctl spawn SIMULATOR_UDID /absolute/path/MetalNavigationBenchmark \
  /tmp/fullscreen-scroll.csv --scroll --live-appearance
```

`--quick` takes every fourth position for diagnosis. `--isolation` accepts
`none`, `navigation`, `tabs`, `pockets`, `glass` or `effects`; these bypasses are
benchmark-only. `--full --catalog --interactive` retains the existing transition
and interactive-pop workload. Keep GPU workloads and builds sequential.

## Verification

All 12 retained Simulator suites plus the explicit uncapped control pass. See
[results](regressions.json) and logs in `raw/`. The new material-lane test adds
240 channel checks across two, three, four and six members, alternating
clear/regular styles and distinct explicit colors, standalone reference samples,
1×/3× rendering and reversed insertion order. Existing tests cover joining,
press feedback, shared neutral sizing, transforms and sampler-limited batches,
plus scroll pockets, navigation, interactive pop and tabs.

Eight static baseline/final raster layout images are byte-identical (zero maximum
and mean channel difference), spanning both themes, three gap offsets and a
smaller item size. See [pixel results](comparison.json),
[light](group-light.png) and [dark](group-dark.png); baseline is left, final right.

The 16 Metal navigation-region comparisons (eight arrangements in dark 3× and
light 2×) are also byte-identical. See [Metal results](metal-comparison.json) and
[dark comparison](metal-dark-5.png). A few whole-frame samples differ only in the
untouched bottom tab lens, outside the navigation output; those timing-dependent
frames are not counted as whole-screen equivalence.

## Packaging

Fresh capture covers 15,134 Simulator frames, including live-appearance scrolling,
all navigation arrangements, push/pop, interactive commit/cancel, large titles
and the stress scene's glass-setting sweep in dark 3× and light 2× profiles.
The light live-appearance capture runs five passes so a late appearance variant
is included; its final three passes require no new pipeline compilation.

The additive raw-source catalog contains 232 libraries and 176 pipeline
descriptions, retaining unchanged earlier states with provenance. All packaged
binaries are rebuilt under source ABI
`4080b09816dd31ece98eab6102a5e2c88d36b8078da3b2c9374b4e143b52a92e`.
The Simulator bundle uses its fresh 99 libraries / 76 pipelines. Another 1,513
cold-cache packaged navigation/gesture frames compile no new libraries or
interaction-time pipelines. Its 76 startup pipeline preparations are expected
on Simulator, where native GPU binary archives are unavailable. See
[bundle coverage](bundled-shader-check.json) and [capture coverage](shader-coverage.json).
Capture/replay timing is excluded from the paired performance results; the
iPhone build ran concurrently during part of this functional verification.

The packaged Simulator app completes both scroll phases and navigation, reaches
idle with zero rendered idle frames, and restores a normal Demo launch.
[Replay results](packaged-replay.json) are functional evidence only; their timing
is excluded from the performance comparison. The retained independent material
controls also pass on the final executable.

The iPhone app builds successfully, embeds 232 freshly compiled libraries / 176
pipeline descriptions and a 139,125,408-byte GPU archive, and is
signed with the existing development identity. All 242 sealed resources
verify. [Artifact hashes](iphone-build.json) identify the ready-to-install app at
`Demo/build/ios/RelWithDebInfo-iphoneos/ThorVGApp.app`. No iPhone installation or
execution was performed during this round.
