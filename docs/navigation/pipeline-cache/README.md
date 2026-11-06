# Navigation pipeline compilation — 2026-11-03

This report preserves the initial cache verification (working tree after
`4cd6300`). The source catalog and app artifacts have since been extended for
[interactive navigation](../interactive/README.md); that audit records the
current catalog, source hashes and packaged-archive checks.

The paused iPhone debugger showed main-thread `Recorder::snap()` → Graphite
`MtlGraphicsPipeline::Make` → `newRenderPipelineStateWithDescriptor` → Apple's
Metal compiler. This was GPU pipeline creation, beyond the existing SkSL cache.
A separate Mac Metal run reproduced an 853 ms pop frame. Several individual
runtime glass pipelines took 340–470 ms to compile.

## Implementation

- Runtime-filter inputs keep an explicit decal node when decal semantics apply.
  Animated bounds no longer switch each input independently between clamp and
  decal graphs. Sampling semantics and filter coordinates are preserved.
- A small, maintained Skia patch adds a client-owned Metal cache boundary for
  shader-library loading and render-pipeline creation. Callbacks can run on
  compiler workers. An exported version symbol detects stale Skia archives.
- Core's private Apple service loads build-time `.metallib` libraries, binary
  archives and generic Metal descriptors. Before the event loop starts it creates
  the native pipeline objects. Core does not instantiate or depend on UIKit.
- Graphite gets two compilation worker queues. They assist uncovered work, but
  moving work to a queue alone is not considered a fix for a frame that needs it.
- Missing/custom states have a measured fallback, with an immutable reader archive
  and a separate serial writer. Disk updates are atomic and occur off the UI
  thread. Cache locations include shader ABI, GPU registry ID and OS build.
- OS/driver generations can invalidate native binaries. Known catalog states are
  recovered at launch from minimum-OS IR and persisted, rather than first compiled
  during navigation. This recovery may increase first launch after an OS update.
- Strict diagnostics prohibit both source compilation and binary-archive misses.
  A Skia null-pipeline guard turns rejected compilation into a reported failure.

The initial `shaders/apple/catalog` contained 68 generated Metal sources and
55 render-pipeline descriptions harvested from the actual full-screen Demo in
all eight arrangements, dark 3× and light 2×. It includes text, icons, glass,
blur, tab/lens rendering and stencil-only passes. `bake_metal.py` validates the
source hashes, function aliases, completeness and shader ABI before compilation.
CMake compiles AIR libraries and GPU binaries during the app build and embeds
`NXKitMetalShaders` into the bundle. The initial bake takes additional build time.

The archive compiler generation matters: targeting macOS 13 produced valid files
but zero strict binary hits on the current macOS 27 driver. Targeting 27.0 yielded
55 prepared native pipelines and no runtime compilations. The default archive
OS generation comes from the SDK major version and can be overridden with
`NXKIT_METAL_ARCHIVE_OS_VERSION`. Portable IR still targets macOS 12/iOS 15.
Apple7-and-later GPU slices are selected with `NXKIT_METAL_GPU_FAMILY=apple7`;
using the broader `metal3` set attempted unsupported AMD variants of the
Apple-Silicon-generated shader catalog. Intel/older Apple GPU catalogs and other
OS compiler generations require separate validation or launch-time recovery.

## Verification

`MetalNavigationBenchmark` uses actual Graphite/Metal and production Demo
controllers, not the raster backend. It records draw plus snap/insert/submit CPU
time; the following GPU completion wait is excluded. It samples each transition
at 120 Hz for one second and repeats the eight push/pop pairs three times.

A fresh process with only the offline bundle, no native disk cache, and strict
compilation prohibition rendered 5,833 full-screen frames per run:

| Run | Runtime library compilations | Runtime pipeline compilations | Pop CPU p95 | Maximum pop CPU time |
|---|---:|---:|---:|---:|
| Dark 3× | 0 | 0 | 2.18 ms | 6.42 ms |
| Light 2× | 0 | 0 | 2.52 ms | 16.18 ms |

The 853 ms reproduction cropped to the navigation bar; the strict results render
the full screen. These are stall diagnostics, not identical-workload FPS ratios
or physical-iPhone performance measurements. Source traces and build artifacts
are recorded in `verification.json` and accompanying compressed CSV files.
The final timing runs use the actual bundled Apple7 archive after app builds
finished. Separate strict runs during concurrent iOS compilation also had zero
cache misses, but CPU scheduling produced 92.79/23.82 ms outliers; those traces
are retained as well. A compilation counter is distinct from total frame time.

A missing native archive is a negative control: strict mode fails without
compiling a replacement. With recovery enabled and the bundle's native archive
removed, all 55 catalog pipelines compile before the first frame and none during
navigation. A second strict process loads the persisted archive and compiles zero.
Metal can ignore `FailOnBinaryArchiveMiss` when no archive is attached, so the
strict path explicitly guards that case.

All 32 headless regression tests and six catalog/embedding tests pass. These
include tampered sources, missing aliases, incomplete captures, stale ABIs and
Xcode's SDK-dependent app resource destination. Both macOS and unsigned iOS Demo
builds pass; the actual iOS bundle's manifest, descriptors and binary archive
match the baked outputs. An unchanged iOS rebuild skips the offline compiler.
The raster suites check optical, interaction, lifecycle and module behavior
independently of Metal compilation.
Physical-device execution of the rebuilt iOS app remains to be verified; the
paused Xcode process was not resumed or replaced.

## Build and reproduce

Apply the maintained patch before rebuilding the Apple Skia archives:

```sh
python3 scripts/apple/apply_skia_patches.py
ninja -C Submodules/skia/out/mac-arm64 skia skparagraph
ninja -C Submodules/skia/out/ios-arm64 skia skparagraph
```

The Apple Metal Toolchain component is required for offline builds:

```sh
xcodebuild -downloadComponent MetalToolchain
cmake -S Demo -B Demo/build/macos -DPLATFORM_DESKTOP=ON
cmake --build Demo/build/macos --target ThorVGApp -j6
```

`ThorVGApp.metal-shaders` is the explicit bake target. Other applications can call
`nxkit_target_metal_shaders(appTarget, catalogDirectory)` from
`cmake/NXKitMetalCache.cmake`. `NXKIT_BUILD_METAL_SHADERS=OFF` explicitly opts the
Demo out and restores uncataloged runtime fallback. Source changes invalidate
incompatible catalogs; regenerate them instead of relabeling the ABI.

Build `MetalNavigationBenchmark` with the existing headless regression project,
then validate the actual bundled artifacts:

```sh
NXKIT_METAL_CACHE_DIR=/tmp/metal-strict-cache \
NXKIT_METAL_BUNDLE_DIR="$PWD/Demo/build/macos/ThorVGApp.app/Contents/Resources/NXKitMetalShaders" \
NXKIT_METAL_BUNDLED_ONLY=1 NXKIT_METAL_REQUIRE_ARCHIVE_HIT=1 \
MetalNavigationBenchmark /tmp/metal-strict.csv --catalog --full
```

Add `--light --2x` for the light/2× run. `NXKIT_METAL_DIAGNOSTICS=1` reports
preparation and uncached pipeline times. To regenerate a catalog, set
`NXKIT_METAL_HARVEST_DIR` to a writable directory and omit the strict/bundle flags;
run both profiles into the same directory. The exporter merges matching-ABI
profiles and fails visibly for unsupported pipeline state. Copy its manifest,
`pipelines.mtlp-json` and `sources/` into the source catalog; native cache binaries
are not source inputs. Applications adding shader/state combinations should
harvest those paths too. Arbitrary custom shaders are not guaranteed to be covered
by the standard Demo catalog.

References: Apple's [binary archive build workflow](https://developer.apple.com/documentation/metal/compiling-binary-archives-from-a-custom-configuration-script)
and [device harvesting and compiler-generation limitations](https://developer.apple.com/documentation/metal/creating-binary-archives-from-device-built-pipeline-state-objects).
