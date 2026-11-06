# Slider/control verification — 2026-11-06

The initial implementation verification below predates the
[lift / material follow-up](../lift-motion/README.md); its ABI and shader counts
are historical. Current lift comparison and installation results are recorded
there.

Source builds for UIKitCore, UIKit and NXKit succeeded on macOS. The iOS
Simulator Demo builds with 242 freshly compiled Metal libraries and 196 pipeline
descriptions. No physical-device build, installation, haptics or thermal result
is claimed by this task.

The control, slider, gesture, scroll physics, Core module boundary, LensView,
LensMotion, tab controller, navigation and interactive-navigation regressions
pass on Simulator. Slider tests pass on macOS too. The slider suite compares 35
independent native geometry cases and six OS-delivered native touch traces,
including fast release projection and final event ordering. It also checks XML
creation, cancellation/range changes, input adjustment and the rounded filled
track cap. A separate point-space shadow check passes at 1x/2x/3x; the existing
glass shadow suite passes.

Raster versus Simulator Metal: 72 resting/partial/full lift cases, both themes,
1x/2x/3x, disabled/enabled, values.25/.75. Average channel error 0.04025/255,
maximum 92/255 at contours; Raster and Graphite use different edge coverage.
The cold bundled replay performs zero shader-library or pipeline compilations
during rendering. Pipeline preparation at Simulator launch remains intentional.
Raw shader states are freshly captured against the final ABI; earlier exact
content-addressed MSL and pipeline descriptions are retained as a superset and
all binary libraries are rebuilt. No old GPU binary is retagged.

The 26 native resting screenshot comparisons in `../comparison/metrics.json`
convert Display P3 to sRGB before measurement. Default slider ROI error is
0.0659/255 light and 0.0409/255 dark. The custom image fixture uses the native
renderer-captured sRGB pink (255,55,95), independent of the current UI trait.
These averages include empty areas and are not claims of full optical parity.
Native trace and private solver limitations are in `../README.md`.

Known unrelated regression-fixture limitation: the full legacy
`NXKitLayerRenderingTests` currently expects a local-only opacity extent and
obsolete glass uniforms such as `saturation` on the native shader. It fails that
old extent assertion and throws on the absent uniform. This task's focused
`--shadow-only` path passes; the complete legacy suite is not reported as passing.
Its unrelated glass assertions need migration to the native shader contract.

Reproduce using the existing Simulator build:

```sh
cmake --build Demo/build/ios-sim-ninja --target NXKitControlTests NXKitSliderTests SimulatorGlassBlurTests
xcrun simctl spawn DEVICE /absolute/path/to/NXKitSliderTests.app/NXKitSliderTests
xcrun simctl spawn DEVICE /absolute/path/to/NXKitLayerRenderingTests.app/NXKitLayerRenderingTests --shadow-only
```

`SimulatorGlassBlurTests --slider-only` supports the existing
`NXKIT_METAL_BUNDLE_DIR` and `NXKIT_METAL_BUNDLED_ONLY` switches. The retained logs
record the actual successful runs; paths and timing are machine-specific.
