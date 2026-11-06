# Independent 3× iPhone control

The native 27.0 / 24A434 iPhone 17 Pro Max Simulator pair uses the same
116×56-point lifted selection lens and blue markers over gray 32 as the 2×
reference. No filter or margin override is applied. Native metadata verifies
screen scale 3, foreground margin 100, backdrop scale 1 and layer contentsScale 1.

Top/bottom G/B separation is 1.57333/1.51914 pt. Left/right tangential separation
is 1.05653/1.05269 pt; blue lies .21481/.22736 pt inward from green. This is close
to the 2× reference rather than establishing a display-scale-proportional margin.
It does not expose the actual intermediate texture allocation or working scale.

The final portable renderer's largest axial G/B gap error is **.047 pt**;
its largest side-normal gap error is **.025 pt**. Mean RGB centroid position
error is .101 pt, with a maximum of .259 pt. [Portable measurements](portable-metrics.json)
retain all per-channel errors and renderer/source/ABI hashes. Portable blue/control
PNGs are retained. The display-independent virtual extent remains an inferred
policy; actual native allocator padding is unmeasured.

The fixture's 720×560-point stage is wider than the phone viewport. Its visible
intersection is stage x140…580, and all four fixed measurement ROIs lie fully
inside it. Retained stage PNGs include black padding outside the captured
viewport. [Metrics](metrics.json) record this visibility check, native scale,
framebuffer dimensions, requests and hashes. Full metadata is compressed here.
The isolated Simulator required one restart after its install service timed out;
the successful captures were taken after boot completed.

Verify retained pixels without registration, scaling or fitted gain:

```sh
/usr/bin/python3 docs/liquid-glass-lens/native-settings/unit-reference/check-holdouts.py
```

Reusable functions in `unit-reference/measure.py` also accept portable renderer
PNGs after their explicit color-space conversion:

- `measure(image, control, region, color, scale)` returns per-channel mass and centroids.
- `ink_points(image, control, color, scale, points, clip_y)` measures the four declared axial marker positions.

Coordinates are logical points in each image's known model space. Pass the
renderer scale and geometry directly; do not align images by their pixels.
