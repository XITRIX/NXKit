> The saved images below precede the gallery tint calibration. Geometry and
> connectivity remain covered by the container fixture; current tint/order
> comparisons and fresh container metrics are in [the gallery report](../liquid-glass-gallery/README.md).

# Glass containers

`GlassContainerView` hosts the copied UIKit-shaped `UIGlassContainerEffect`.
Add glass `BackdropEffectView` items to its `contentView()`. The default spacing
is **0**, matching both captured UIKit versions. Positive spacing produces
continuous bridges between nearby items. Moving their frames/bounds, animating
presentation geometry or pressing interactive items changes the same combined
field. There is no separate merge timer or change to child hit-test geometry.

```cpp
UIGlassContainerEffect grouping;
grouping.setSpacing(20);
auto container = new_shared<GlassContainerView>(grouping);
container->setFrame({0, 0, 400, 240});

UIGlassEffect glass;
glass.setInteractive(true);
auto left = new_shared<BackdropEffectView>(glass);
left->setFrame({40, 70, 100, 80});
left->layer()->setCornerRadius(28);
container->contentView()->addSubview(left);
auto right = new_shared<BackdropEffectView>(glass);
right->setFrame({180, 70, 100, 80});
right->layer()->setCornerRadius(28);
container->contentView()->addSubview(right);

UIView::animate(0.6, [right] { right->setFrame({145, 70, 100, 80}); });
```

Install the container in the normal view hierarchy. Reinstall a modified
container effect with `setEffect()`; configuration values are copied. Negative
or nonfinite spacing throws; XML `GlassContainerView` accepts finite,
nonnegative `spacing`. The Demo's Glass screen puts its draggable clear and
regular items in a container with spacing 24.

The renderer collects live presentation shapes each frame, including bounds
origins, anchor points, affine transforms, radius/curve and interactive motion.
A polynomial smooth union weights facing normals to avoid inflating parallel
edges. A single backdrop filter shades the merged field, blends each item's
resolved material/tint in painter order with a narrow boundary transition, supplies each item's frost input, and applies coverage
once. Child material draws are suppressed only during that traversal; content,
controls and gestures still render/receive input normally. Insertion, removal,
hidden state and reparenting take effect without membership registration.
Nested containers form separate groups. Blur, tint and refraction do not stack
at an overlap. The default elevation shadow follows the merged field.

The host is an intentional NXKit API difference: Apple installs the effect on
`UIVisualEffectView`. See Apple's [UIGlassContainerEffect documentation](https://developer.apple.com/documentation/uikit/uiglasscontainereffect)
and [WWDC25: Build a UIKit app with the new design](https://developer.apple.com/videos/play/wwdc2025/284/).
The merge equation is an original approximation calibrated against observable
contours; Apple's private implementation is not available.

## Verification, 2026-10-27

The independent `ContainerReference.m` fixture links Apple UIKit. It uses the
same 720 × 760 point scene as the production retained-view raster tests: sixteen
pairs with spacing 0/10/20/40 and gaps 4/12/24/44, plus overlapping/separated
rounded rectangles. Requests move retained items nearer/farther, resize them,
switch to dark appearance, and return to the initial state. Screenshots come
from the actual Simulator framebuffer. SDK: iPhoneSimulator 27.1, deployment 26.
Both test devices are iPad Pro 11-inch M5 at scale 2.

| Captured native version | Exact connectivity cases | Minimum silhouette IoU | Returned native silhouette IoU |
| --- | ---: | ---: | ---: |
| iPadOS 26.4.1 | 108 / 108 | 98.63% | 99.9998% |
| iPadOS 27.0 | 107 / 108 | 96.74% | 99.9790% |

At spacing 10 / gap 6, UIKit 27 retains a tiny antialiased bridge that is absent
in UIKit 26 and NXKit. The comparison records this one-point transition
boundary difference explicitly. Default comparisons tolerate at most one point
at the connectivity threshold; `--strict` fails for that UIKit 27 case. All
other connectivity mismatches and silhouette IoU below 95% fail the test.
Tint hue plus contrast identify the contour, including native neutral reflection
across an otherwise dark optical seam. Raw channel errors are also recorded.
No per-image alignment, exposure adjustment or retouching is performed.

The production raster regression additionally covers 1×/2×/3× coverage,
independent content and hit testing, presentation movement, interactive
inflation, affine translation/rotation, two/three items, mixed tints, opacity, overlapping
material without doubled tint, invalid spacing, XML, custom effect exclusion,
empty bounds, nested containers, clipping and removal/reparenting. The four
raster material/appearance/corner/container executables pass. iOS Demo and all
four glass regression executables build and link.

## Limits

This validates grouping geometry and lifecycle, **not pixel-identical optics**.
The captures still expose substantial explicit-tint brightness differences,
particularly in dark mode, and different internal highlights at the join.
Native spring timing and end-to-end Graphite Metal/Dawn output/performance have
not been compared here. Material bevel distances under item transforms
remain an approximation. Shader work grows with item count; the tests cover
small UI groups rather than large collections.

An alpha-masked item, a masked/clipped ordinary wrapper, or a perspective/3D
wrapper is an independent compositing boundary and keeps ordinary independent
glass rendering. Such branches do not merge across the container. The
container's own clip/mask still applies to the complete combined surface.
Spacing changes install immediately; this does not implement Apple's private
materialization or effect-to-effect transition machinery. Existing glass
appearance adaptation remains opt-in and disabled by default.

## Reproduce

Use matching raster Skia headers/archives and ABI definitions and an installed SDL3
CMake package. This headless harness executes the actual UIKitCore layer tree;
it substitutes only the platform/window/font backend. With this workspace's
older `out/mac-arm64-angle` archive, matching headers were exported from Skia
commit `5f2e30e0c8bd00b2ff314a6c751ceac95fedb913`. Newer current headers cannot be
mixed with that archive. The workspace archive is a debug Skia build: pass `NXKIT_SKIA_DEBUG=ON`
even when the application is optimized. Leave it off only for a release Skia
archive. Mismatched ref-count ABI can crash image decoding. Build raster Skia
from current sources instead when validating a current raster backend.

```sh
cmake -S Demo/GlassComparison/RasterTests -B /tmp/nxkit-glass-tests -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DNXKIT_SKIA_DEBUG=ON -DNXKIT_SKIA_RASTER_HEADERS=/path/to/matching/headers \
  -DNXKIT_SKIA_RASTER_ARCHIVE=/path/to/matching/archives
cmake --build /tmp/nxkit-glass-tests -j6
ctest --test-dir /tmp/nxkit-glass-tests --output-on-failure

sh Demo/GlassComparison/build-container.sh /tmp/nxkit-container
python3 Demo/GlassComparison/capture-container.py DEVICE_ID \
  /tmp/nxkit-container/NativeContainerReference.app /tmp/nxkit-container/native
# Generate the identical retained NXKit states. Arguments: output gap size dark.
/tmp/nxkit-glass-tests/ContainerTests /tmp/nxkit-container/initial.ppm 0 1 0
/tmp/nxkit-glass-tests/ContainerTests /tmp/nxkit-container/near.ppm -6 1 0
/tmp/nxkit-glass-tests/ContainerTests /tmp/nxkit-container/far.ppm 18 1 0
/tmp/nxkit-glass-tests/ContainerTests /tmp/nxkit-container/large.ppm 0 1.25 0
/tmp/nxkit-glass-tests/ContainerTests /tmp/nxkit-container/dark.ppm 0 1 1
python3 Demo/GlassComparison/compare-container.py /tmp/nxkit-container/native \
  /tmp/nxkit-container /tmp/nxkit-container/report
```

The comparison script requires Pillow and NumPy. Preserve `screen.png` and
`capture.json` for every native checkpoint when regenerating the report.
