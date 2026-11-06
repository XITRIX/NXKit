# Glass rendering optimization — 2026-11-05

The later [scrolling follow-up](../scroll-followup/README.md) addresses the
user's remaining lag report after this build was installed, with frame-deadline
handling and another measured reduction in glass rendering work.

Working tree after `50a7240`. Final validation in this report runs **inside iOS
Simulator**, as requested. No optimized build was installed on the iPhone.
The preceding physical-device investigation is documented in the parent report;
its FPS numbers are not measurements of this implementation.

## Changes

- Bound the sharp backdrop separately from blur-atlas addressing. A runtime
  image filter has one maximum sampling radius for all inputs. Atlas addressing
  inflated capture of the unfiltered scene even though optical sampling stayed
  near the glass. The sharp input now retains only the required source domain,
  including group transforms, material reach and merging expansion.
- Explicitly clip standalone native exterior glass to its declared output.
  `saveLayer` bounds are allocation hints. Without an output clip, replacing
  pixels with `kSrc` depended on preserving the entire surrounding scene. The
  bounded capture exposed this on raster surfaces; the clip fixes that dependency
  and matches the existing container renderer.
- Specialize capture loops for their actual 1–4 tap count. Skip neighbouring
  cells only when their interpolation weight is exactly zero. Fractional
  reconstruction and sample arithmetic remain unchanged.
- Replace the integer mip-address loop with its exact geometric sum, and avoid
  fetching a second mip when its interpolation weight is zero.

Blur kernels, resolution, color recipes, optics, geometry, animation curves and
normal idle rendering policy are preserved. Glass material resolution also keeps
the existing one-point fallback for invalid display-scale traits; the private
native-profile validator still rejects invalid direct inputs.

Simulator support now selects arm64 simulator dependencies, propagates Skia's
`trivial_abi` calling convention to external consumers, and uses Graphite's
existing destination-copy path where Simulator Metal rejects framebuffer reads.
Simulator binary-archive creation is disabled because that backend has no device
architecture for archives. It can still load offline IR and prepare pipeline
objects at launch. These backend conditions do not change the phone's Metal path.

## Measurements

Apple M1 Max host, iOS 27.0 Simulator / iPhone 17 Pro Max, SDK 27.1,
arm64 `RelWithDebInfo`, Metal validation disabled. Both versions use the same
simulator-compatible Skia build and benchmark. The baseline preserves the exact
pre-optimization renderer sources. Its executables were frozen before changes.

The offscreen viewport is 402×874 points at 3×. Navigation exercises all eight
Demo item arrangements with real controller push/pop animations; the stress
scene has 96 live cards, vertical scrolling and eight horizontal rails. Frames
are unpaced and explicitly synchronize GPU completion. **This measures completed
rendering work, not delivered FPS.** The simulator's presentation timestamp API
is unavailable, so no presentation rate is inferred from CPU time or submissions.

Two runs per renderer use before/after then after/before order. Only the final,
fully warm pass is included below; no pipeline counter increases during those
passes. Initial compilation, snapshots and builds are excluded. No other test
or build ran during the measured workloads. Medians combine equal-sized runs.

| Workload | Baseline | Optimized | Time reduction |
| --- | ---: | ---: | ---: |
| Navigation, CPU + completed GPU | 25.708 ms | 17.578 ms | 31.6% |
| Nested glass scrolling, CPU + completed GPU | 65.601 ms | 41.176 ms | 37.2% |
| Navigation CPU recording/submission | 4.972 ms | 3.892 ms | 21.7% |

Each navigation renderer contributes 992 measured frames. Each stress renderer
contributes 48 scrolling frames spanning the complete up/down trajectory.
Individual stress-run medians are 65.834/65.459 ms before and 41.195/41.072 ms
after. [Measurements](measurements.json) retain all run medians and stage timings;
compressed CSVs are in `raw/`.

The finite, actual full-screen app replay also completes on the simulator with
scrolling, eight navigation push/pop pairs, a second scroll and idle. Both
versions render zero frames during the idle phase. Its independent summaries
are `replay-before.json` and `replay-after.json`. Rendered-frame submission rates
are labelled separately from presentation FPS. These are secondary controls,
not a matched physical-device FPS or thermal experiment.

## Fidelity and regressions

- Twenty capture controls on **each** simulator backend (Metal and CPU raster):
  five reduction scales, four fractional offsets, colored translucent input.
  Every RGBA byte matches the frozen pre-optimization capture equation.
- Standalone-glass output checks at 1×/2×/3× on both backends preserve remote
  background pixels exactly. The isolated build without the new output clip
  fails this check; `missing-clip-control.log` records that negative control.
- All 64 independent native blur controls retain the baseline errors: step
  MAE 0.406494/255, stripe MAE 0.704753/255. Existing thresholds remain unchanged.
- Sixteen resting navigation captures (eight arrangements, light 2×/dark 3×)
  are byte-identical. Thirty-two push/pop midpoint controls have a maximum
  whole-image MAE of 0.128/255. These use per-transition relative timers and
  are visual controls, not a claim of byte-exact animation timing parity.
- Fifteen existing rendering, navigation, interactive-pop, tab, frame-loop and
  Core-boundary suites pass in the simulator. Scroll pockets include 213
  checks; material tests include 480 color controls with maximum error 1/255.
  The explicit uncapped pacing case also passes with its required environment.

`regressions.json`, `blur-tests.log`, `uncapped-frame.log` and
`image-differences.json` retain the results. Intermediate failing experiments
are excluded from the timing data. The scroll-pocket failure caused by the
initial bounded capture was fixed; no thresholds were relaxed or tests removed.

## Reproduction

Build Skia for `target_os="ios"`, `target_cpu="arm64"`,
`ios_use_simulator=true`, `ios_min_target="15.0"`,
`skia_enable_graphite=true`, `skia_enable_ganesh=false`, `skia_use_gl=false`,
`skia_use_metal=true`, `is_trivial_abi=true`, `is_debug=false` into
`Submodules/skia/out/ios-simulator-arm64`, with the repository's patches.
This workstation used `is_official_build=false` and line-table debug information.

```sh
cmake -S Demo -B Demo/build/ios-sim-ninja -G Ninja \
  -DPLATFORM_IOS=ON -DPLATFORM=SIMULATORARM64 \
  -DVCPKG_TARGET_TRIPLET=arm64-ios-simulator \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo -DNXKIT_BUILD_METAL_SHADERS=OFF \
  -DNXKIT_BUILD_SIMULATOR_BENCHMARKS=ON \
  -DUIKIT_BUILD_TESTS=ON -DUIKITCORE_BUILD_TESTS=ON
cmake --build Demo/build/ios-sim-ninja --target \
  ThorVGApp MetalNavigationBenchmark MetalGlassStressPerformance SimulatorGlassBlurTests -j 8
```

With a booted simulator, run each executable using its absolute path:

```sh
xcrun simctl spawn "$SIMULATOR_UDID" "$BUILD/SimulatorGlassBlurTests"
SIMCTL_CHILD_MTL_DEBUG_LAYER=0 xcrun simctl spawn "$SIMULATOR_UDID" \
  "$BUILD/MetalNavigationBenchmark" /tmp/navigation.csv --full --catalog --quick
SIMCTL_CHILD_MTL_DEBUG_LAYER=0 xcrun simctl spawn "$SIMULATOR_UDID" \
  "$BUILD/MetalGlassStressPerformance" /tmp/stress.csv --frames 32 --passes 3
python3 Demo/scripts/measure_simulator_demo.py --simulator "$SIMULATOR_UDID" \
  --app "$BUILD/ThorVGApp.app" --output /tmp/demo-replay
```

Add `--snapshots /tmp/frame` to the navigation benchmark for comparison PPMs.
Regression executables under `Thirdparty/UIKit/<module>/<test>.app/<test>` also
run with `simctl spawn`; they need not be launched as UIKit applications.
The uncapped pacing test requires `SIMCTL_CHILD_NXKIT_UNCAPPED=1` and `--uncapped`.

## Packaged build verification

Fresh simulator capture covers 15,414 frames: both themes/scales, all interactive
arrangements, large-title push/pop and gesture completion/cancellation, all 128
icon-blur kernel samples, and the stress scene's glass-setting sweep. Every final
pass is warm. The merged raw catalog retains earlier states with explicit
provenance and contains 217 libraries / 165 pipeline descriptions. No old GPU
binary was relabelled. `shader-coverage.json` records the captures and hashes.

The simulator bundle contains 102 offline libraries and prepares 78 pipeline
objects at launch. A further 4,052 bundled navigation/gesture/large-title/ink
frames compile **zero new libraries or pipelines during interaction**. These
are prepared-object hits, not simulator binary-archive hits. The packaged
full-screen replay also completes and returns to normal Demo mode. Its timing
is excluded from the comparison table; a phone build was running concurrently.

The optimized, symbolized iPhone build succeeds and is signed with the existing
development identity. Its offline iOS bundle contains 217 libraries and a newly
compiled 165-pipeline GPU archive under the same source ABI. Four unreferenced
libraries left by earlier build experiments were removed, the app was signed
again with Xcode's exact identity/entitlements, and every sealed resource hash
was checked. `iphone-build.json` records artifact and archive hashes.

Ready-to-install artifact: `Demo/build/ios/RelWithDebInfo-iphoneos/ThorVGApp.app`.
It has **not** been installed or executed on the iPhone in this final validation.

## Limits

Simulator GPU work, host memory traffic and its destination-copy path differ
from physical iPhone execution. The results establish less rendering work and
preserved tested output; they do not establish sustained 60/120 FPS, temperature,
power consumption or complete native UIKit pixel parity. Those require the
user's next phone test. Normal launches retain idle skips and frame pacing.
