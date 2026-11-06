# Pressed glass exterior halo — 2026-11-06

Working tree after `50a7240`.

## Cause and correction

The native material decorates a field that already includes its original backdrop,
shadow and exterior rim. Its interaction lighting used the press intensity without
surface coverage, so pressing a tab illuminated the retained backdrop outside the
platter. This produced the broad rectangular/rounded halo in IMG_2790.jpg.

`nativeGlassDecorate` now multiplies press intensity by the existing surface
coverage. The shared standalone/grouped material retains its interior lighting,
refraction, geometry and exterior shadow/rim; fully exterior pixels receive no
press lighting. This introduces no extra capture, texture, pass or public API.

## Regression evidence

`UIKit/tests/GlassPressCoverageFixture.hpp` renders the actual material program
with constant backdrop inputs and fixed geometry. Before the correction an
exterior channel changed from 187 to 221. Afterward all 432 combinations pass on
Raster and Simulator Metal: exterior probes remain byte-identical while all 432
bodies retain their press response. Cases include quarter/full press, three
button/platter sizes, both themes, 1x/2x/3x, clear/regular styles, neutral/explicit
blue tint, and preference values 0/.5/1. Constant shader children intentionally
isolate material lighting from capture and geometry changes.

Glass container, appearance, tab controller, navigation and interactive
navigation suites also pass. See the accompanying logs. Shader-package and final
physical-device verification are recorded below.

These are regression checks for the reported exterior halo, not a new claim of
pixel-identical UIKit interaction or sustained 120 Hz. The outstanding device
performance investigation remains in
[the headroom report](../performance-ios-20261106/iphone-headroom/README.md).

## Reproduction

Build `NXKitGlassMaterialTests` and `SimulatorGlassBlurTests` in the existing
Simulator regression build. Run the former normally and the latter with
`--press-only`; the full optical suite uses no arguments. The fixture is shared
between these executables so the CPU and Metal programs receive identical
uniforms and probes.

## Offline package

Current shader ABI: `c8d80683d2f566a2c943c762bdc3b3a80475eff45a43835e3c34953cb65e00f8`.
The catalog retains unchanged Core sources and replaces obsolete native material
programs with fresh Simulator and A17 Pro captures. Libraries requested before
pipeline creation are retained as well. Every library and native GPU pipeline
was rebuilt offline for this ABI. The package contains 211 source libraries and
163 pipeline descriptions; provenance is in the catalog manifest. Signed
executable/archive hashes and 221 verified sealed resources are recorded in
`iphone-build.json`.

All 13,028 cold packaged Simulator frames pass: zero source-library compilations,
with 163 pipeline objects prepared before interaction and no further pipeline
compilation. Workloads cover light/dark scrolling, interactive navigation and
large titles. Simulator has no native Metal binary archive, so its launch-time
pipeline preparation is expected. This is coverage verification, not a physical
frame-rate measurement.

The changed UIKit module builds on macOS, and the signed iPhone app build passes.
The corrected package is installed. A strict A17 Pro replay completes with zero
source/pipeline compilations and zero pipeline failures through scrolling and
navigation. Its 30-second second-scroll phase averages 119.97 FPS, with GPU
p50/p95/p99 of 4.95/6.00/6.52 ms while charging in Fair thermal state. This short
check does not establish sustained 120 Hz or a causal performance improvement over
the earlier Serious-state runs. The first 78-second host attempt timed out before
a complete export and is invalid; the repeated run uses a longer bounded startup
allowance, now included in the device harness. The actual Simulator app replay
also passes with zero source compilations and constant prepared pipeline count.
The harness restores ordinary device and Simulator launches successfully. Both
restoration receipts and the complete strict metrics are retained alongside this
report.
