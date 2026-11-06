# Fixed interactive glass buttons

The Glass gallery includes four additional examples:

- **Interactive · clear buttons** — a circle, capsule and rounded rectangle.
- **Interactive · tinted buttons** — the same fixed controls in blue, coral and green.
- **Navigation buttons · mock** — leading Back, trailing More (ellipsis) and an emphasized blue “+”.
- **Navigation buttons · container** — the same row inside one `GlassContainerView`.

The navigation rows are button-placement mockups built from glass hosts and
ordinary controls. They exercise no navigation-bar API or navigation action.
Taps toggle the icon to a checkmark; Reset restores the original symbols.
All four examples enable `UIGlassEffect::isInteractive` and install no app pan
gesture. Touch inflation, stretch and lighting leave their layout frames fixed.
The original separate-material row remains available beside the grouped variant.

The container row uses eight points of merge spacing and a twelve-point gap
between its trailing controls. The items remain separate at rest, and pressing
the pair lets their inflated surfaces join. The leading Back button shares that
container but stays isolated by its distance. Explicit tint still follows each
item, including the blue emphasized button.

New examples retain 52-point-or-larger touch targets on narrow screens. The stage
adapts to its holder instead of scaling the controls down with the artwork.
Leading/trailing margins remain twenty points, and the trailing gap stays twelve.
All scenes support the existing backdrop cycle and light/dark inherited traits.

`GlassDemoSpec.h` shares geometry, material choices, icons and container spacing
with the independent `GalleryReference.m` UIKit fixture. The native mock uses
`UIVisualEffectView`, `UIGlassEffect`, `UIGlassContainerEffect` and ordinary buttons.

![UIKit and NXKit fixed interactive buttons in light and dark appearance](fixed-buttons.png)

The captures compare actual iPadOS 27.0 Simulator framebuffers with NXKit's
retained production layer tree on Skia raster, using identical 720×440-point
stages at 2×. They include artwork and split black/white/gray backdrops, both
appearances and all four new scenes. Geometry is checked before comparison.
[Metrics](metrics.json) sample central material regions with icon pixels excluded;
they do not establish full-image or native touch-timing parity. Rims, refraction
detail and group-join shading still show differences. This change adds fixtures
and examples without recalibrating the core material.

Reproduce using the existing matching raster Skia/SDL setup:

```sh
cmake --build YOUR_RASTER_BUILD --target GalleryRenderer GalleryTests
ctest --test-dir YOUR_RASTER_BUILD -R '^GalleryTests$' --output-on-failure
sh Demo/GlassComparison/build-gallery.sh /tmp/fixed-glass-native
python3 Demo/GlassComparison/capture-gallery.py IPAD_SIMULATOR_ID \
  /tmp/fixed-glass-native/NativeGalleryReference.app /tmp/fixed-glass-captures --fixed-buttons
python3 Demo/GlassComparison/render-gallery.py YOUR_RASTER_BUILD/GalleryRenderer \
  Demo/resources/img/glass_demo_scene.png /tmp/fixed-glass-captures /tmp/fixed-glass-rasters
python3 Demo/GlassComparison/compare-gallery.py \
  /tmp/fixed-glass-captures /tmp/fixed-glass-rasters /tmp/fixed-glass-report
```

The comparison script requires Pillow and NumPy. Verified 2026-10-27, working
tree after `a25895b`: gallery regressions check real touch delivery, press glow,
release/cancellation, fixed frames, narrow/wide fitting, edge anchoring, reset,
and the container's press-induced SDF neck. The controller-layout smoke harness
checks all 17 scenes at widths 390/834/1280 and back to 390. The iOS Demo and
native UIKit fixture build successfully. NXKit Metal/Dawn execution and native
press-animation timing remain unverified.
