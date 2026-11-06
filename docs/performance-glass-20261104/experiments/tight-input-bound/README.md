> Discarded intermediate experiment. See DISCARDED.md and the parent final report.

# Sustained glass performance follow-up — 2026-11-04

Working tree after `fb556df`, including the prior eased ScrollPocket gradient.

## Device observations

The initial, already-running iPhone 15 Pro Max / iOS 27.2 app was sampled for
30 seconds. Instruments reported **Serious** thermal state throughout the
31.375-second thermal interval. Its CPU stacks include Metal API validation
(`MTLDebugRenderCommandEncoder`, argument/protocol validation) under frame
submission. That overhead is inappropriate for performance measurements.

A separate 25-second capture relaunched the installed app with
`MTL_DEBUG_LAYER=0`, `NXKIT_GLASS_STRESS_REPLAY=1`, `NXKIT_UNCAPPED=0`.
Glass drawing accounted for 5,646 ms of the main thread's 18,446 ms sampled
weight after excluding the first four seconds; material resolution accounted
for 1,241 ms inclusively. These stack weights overlap and must not be added.
The initial screen and replay differ, so these captures are **not** an A/B
measurement of the validation toggle or this renderer patch. Summary JSON and
the original thermal interval are retained here; raw traces are in
`/private/tmp/nxkit-sustained-perf`.

## Changes

- Cache resolved regular/clear material descriptions, including both a grouped
  member's own size and its group's shared material size. Immutable cached
  descriptions are shared with presentation copies; appearance/effect changes
  invalidate them, and animated sizes resolve their current geometry.
- Bound regular glass backdrop inputs by the shader's actual displacement,
  dispersion, detail-tent and light-mode chroma reads. The shader's optical
  uniforms and declared maximum remain unchanged. Clear reflections and custom
  shaders retain their declared radius. Transformed groups use the largest
  affine stretch; region partitioning retains its original conservative reach.
- Generate **ThorVGApp Performance** when building an Xcode Demo target.
  Run/Profile use `RelWithDebInfo`, with Metal API validation and frame capture
  disabled. The ordinary scheme remains available for debugging. CMake's own
  OFF/Disabled scheme output was checked to verify the XML values (1 and 3).

Select **ThorVGApp Performance** in Xcode for sustained device runs. Use
`NXKIT_GLASS_STRESS_REPLAY=1` for the repeatable feed/eight-rail workload, and
remove it for ordinary Demo use. `NXKIT_UNCAPPED=1` is only for throughput
profiling; normal pacing and the explicitly requested continuous rendering are
preserved.

## Validation

The material regression compares tight/full input filters over a busy colored
backdrop at 1×/2×/3×, in light/dark appearances and two sizes. It covers clear
fallback and explicit optical clipping overrides. Glass appearance, material,
container, rim, lens-refraction and ScrollPocket suites pass.

Twelve frozen/current raster snapshots cover both stress appearances at rest
and after scrolling, plus all eight navigation arrangements: eleven are exact;
the light scrolled stress image has a maximum channel difference of 1/255.
The filter's smaller intermediate blur bounds can change final rounding.
See `pixel-comparison.json`.

Offline shader coverage is freshly harvested under ABI
`2a60f0cc19fe0a54830b50379f67efa4312e132a5b94798f16bcd7708dd6c503`.
The additive catalog retains 121 library variants and 102 pipeline descriptors;
90/72 were encountered freshly in 7,310 edge/navigation frames. Retained raw
states have explicit provenance. GPU archives are rebuilt, never relabelled.

## Reproduction

Freeze the prior release harness executables and their matching baked shader
bundle before editing production sources. Build the headless harness with
matching Release Skia, then run `run-paired.py --batch <capture-dir> --build
<headless-build> --bundle <fresh-macos-bundle>`. The script alternates order over
three repetitions of dark 3× and light 2× stress and navigation workloads,
requires archive hits and computes medians from the warm passes.

Device CPU capture:

```sh
xcrun xctrace record --template 'Time Profiler' --device <UDID> \
  --time-limit 30s --output device.trace --attach ThorVGApp
xcrun xctrace export --input device.trace \
  --xpath '/trace-toc/run[@number="1"]/data/table[@schema="time-profile"]' \
  --output device.xml
python3 docs/performance-stress-20261104/summarize_profile.py device.xml
```

Mac offscreen timings include synchronous GPU completion for the stress test;
navigation CPU times exclude that wait. Neither measures presentation latency,
energy, battery temperature, or a thermal improvement on iPhone. The framework
still composites continuously, and repeated live glass/filter passes remain
substantial work. Switch performance is unmeasured.
