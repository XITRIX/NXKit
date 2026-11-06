# Shared Glass demo gallery

Verified 2026-10-27, working tree after `a25895b`.

The Glass tab now opens a scrollable collection of 17 buttons. Each pushes a
separate scene: fixed clear/regular, tinted clear/regular, movable clear/regular,
neutral/tinted morphing, resizing, press feedback, corner curves, iOS 26 contrast
opt-in, a dedicated blue/coral two-tint scene, fixed clear/tinted buttons and
separate/container-backed mock navigation button rows. Back/B returns to the menu.
The menu and all pushed demo screens use the same centered, measured Yoga column as Navigation Test:
720-point maximum width, 48-point side padding, 64-point buttons, 16-point
gaps and safe-area-aware scrolling. Descriptions wrap to their measured height,
and the preview keeps its 720:440 aspect ratio inside that column. The optical previews keep their
logical geometry while their holder scales with the screen. The four fixed-button
examples resize their stage instead, preserving touch targets of at least 52 points.
A controller-layout smoke check exercises all 17 inner screens at widths 390,
834 and 1280, then returns to 390, checking preview size, button alignment/gaps
and scroll extent.
Scenes offer backdrop cycling, reset, movement/join/resize replay where applicable,
and direct dragging. The two-tint scene also swaps the view order. Ordinary input
and glass feedback remain separate, and swapping updates both drawing and hit testing.

See the [fixed-button examples and UIKit comparison](fixed-buttons/README.md).

## Reference and measurements

`Demo/app/Screens/GlassDemoViewController/GlassDemoSpec.h` supplies the exact
720 × 440 geometry and palette to both the app and independent Apple UIKit
`GalleryReference.m`. Both use the same prepared artwork. `GalleryRenderer`
instantiates the actual app's `GlassDemoScene` and renders its retained UIKitCore
layer tree with the production blur/filter/SDF shaders on Skia raster, at 2×.
Native images are actual iPad Simulator framebuffer captures, compiled with the
27.1 SDK and run on **iPadOS 26.4.1** and **27.0**, not CALayer snapshots.

The saved 27.0 gallery audit contains 66 checkpoints across the first 12 scenes:
light/dark, three retained move/join/resize states plus return, and explicit tint
controls on artwork, gray, black and white. A separate 14-checkpoint two-tint
series adds approach, join, swapped order and separation in both themes on each
native version. Comparisons use recorded scene coordinates, with no registration,
exposure correction or image retouching. Small float serialization differences
are tolerated within 0.001 points; a geometry mismatch fails the comparison.

![UIKit, previous NXKit material, current NXKit](gallery.png)

Mean absolute RGB channel errors use conservative central body regions (the
middle 60% of each item's width/height), excluding every foreground plus.
The mean weights each checkpoint equally; this measures body appearance,
not contours, animation timing, touch latency, or full-renderer parity.

| 27.0 default-policy cases | Before | Current |
|---|---:|---:|
| 64 checkpoints excluding the intentional iOS 26 opt-in | 44.980 | 7.569 |
| Neutral regular, light | 24.827 | 8.538 |
| Neutral regular, dark | 26.495 | 10.027 |
| Clear tint, light / dark | 92.846 / 92.846 | 2.314 / 2.315 |
| Regular tint, light / dark | 61.688 / 95.459 | 2.873 / 1.279 |

[All 66 measurements](metrics.json) include the opt-in scene's mismatches against
27.0 instead of hiding them. The baseline is the previous working-tree material,
rendered with the same gallery before this calibration. It is not a separate
committed release. Reproducing a before column requires preserving that renderer;
current/native comparisons require only this source tree.

Changes include restoring explicit tint on clear glass, separate clear/regular
tint vibrancy, a neutral regular body that remains visible over black in a fixed
light theme, and better dark brightness/saturation through size changes. Explicit
`tintOpacity` remains a low-level optical override; without it a supplied tint
uses the native-calibrated path. Optical color transforms are approximations
fitted to the native controls, not Apple's private implementation.

## Two different tints

![Two tints, both orders and both appearances](two-tints27/two-tints.png)

[Approach/join/separate checkpoints](two-tints27/two-tints.gif) have illustrative
playback timing, not captured animation-frame timing. The current blue circle
and coral capsule keep distinct colors. The front item owns most of the overlap,
with a narrow handoff near its boundary; averaging tint with the geometry's
smooth-union weights produced an incorrect broad mixed-color region.
Geometry still merges independently into a single antialiased silhouette.

The material handoff is calibrated to an approximately eight-point transition,
starting ahead of the front item's geometric edge. The native internal optical
seam is darker and more detailed, especially in light appearance; NXKit does
not yet reproduce that shading. Swapping removes/readds the views in both
adapters. On this UIKit runtime, merely bringing an existing view forward did
not refresh the glass material's tint ordering, whereas reinsertion did.

All four joined order/theme comparisons agree on tint ownership on each native
version. The comparison script asserts ownership in an overlap patch below the
foreground symbols, alongside the pure-color/order and hit-testing regressions.
Mean body errors across the 14 states are **5.306/255** on
[27.0](two-tints27/metrics.json) and **4.900/255** on
[26.4.1](two-tints26/metrics.json). The geometry and NXKit frames are identical
for both version comparisons. The new two-item case has no before column.

## Limits and verification

The default targets theme-fixed 27.0 behavior. A [24-state 26.4.1 baseline](metrics26.json)
is also saved: neutral states differ substantially with that runtime's retained
material/appearance behavior. The split-underlay opt-in gallery scene also does
not match this native 26.4.1 setup, even though NXKit's selection and descendant
color propagation pass the dedicated retained-view regressions. The earlier
[controlled contrast fixture](../liquid-glass-ios27/contrast/README.md) records
the size/content distinction used for this approximate opt-in policy. This
gallery does not establish full iOS 26 material parity or universal private
thresholds. [View the native 26.4.1 baseline](gallery26.png).

Six headless tests pass: retained appearance, corner coverage, material/motion,
container behavior, all gallery scene construction/reset/order/hit-testing,
and the dependency's XML test. The iOS Demo and material/appearance/corner/container
targets build/link successfully. Native fixture builds/captures also succeed.
The NXKit GPU/input loop and on-device timing were not run. macOS Graphite/Metal
and Switch archives remain absent; the available older raster archive is not a
substitute for those platform builds. Fresh container comparisons also pass: [26.4.1](container26.json) matches all
108 connectivity cases (minimum silhouette IoU 98.38%); [27.0](container27.json)
matches 107/108 (96.91%), retaining the documented one-point AA threshold difference.
Exact inner seams, corner splines, neutral
mid-size states, touch timing and backend performance remain partial.

## Reproduce

Requires Xcode, an iPad Simulator with iOS 26+, Python with NumPy/Pillow, SDL3,
and raster Skia headers/archives with matching revision and ABI. The workspace's
older `out/mac-arm64-angle` archive is a debug build with headers from
`5f2e30e0c8bd00b2ff314a6c751ceac95fedb913`. Pass `NXKIT_SKIA_DEBUG=ON` for that archive;
use OFF for a release archive. Application optimization is independent of this
Skia ABI choice. See the [container build instructions](../liquid-glass-container/README.md#reproduce).

```sh
cmake -S Demo/GlassComparison/RasterTests -B /tmp/nxkit-glass-tests -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DNXKIT_SKIA_DEBUG=ON \
  -DNXKIT_SKIA_RASTER_HEADERS=/path/to/matching/headers \
  -DNXKIT_SKIA_RASTER_ARCHIVE=/path/to/matching/archives
cmake --build /tmp/nxkit-glass-tests -j6
ctest --test-dir /tmp/nxkit-glass-tests --output-on-failure
sh Demo/GlassComparison/build-gallery.sh /tmp/nxkit-gallery
python3 Demo/GlassComparison/capture-gallery.py DEVICE_ID \
  /tmp/nxkit-gallery/NativeGalleryReference.app /tmp/nxkit-gallery/native
python3 Demo/GlassComparison/render-gallery.py /tmp/nxkit-glass-tests/GalleryRenderer \
  Demo/resources/img/glass_demo_scene.png /tmp/nxkit-gallery/native /tmp/nxkit-gallery/raster
python3 Demo/GlassComparison/compare-gallery.py \
  /tmp/nxkit-gallery/native /tmp/nxkit-gallery/raster /tmp/nxkit-gallery/report
```

The current default capture includes 68 states (the added two-tint scene also
appears in its baseline). Use `--two-tints` for the dedicated 14-state order test,
`--baseline-only` for all 13 initial states in both themes, or `--material-controls`
for the 16 neutral style/theme/backdrop controls. Repeat on both Simulator versions.
Keep `screen.png`, `capture.json` and `series.json` when regenerating measurements.
