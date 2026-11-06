# Native continuous-corner comparison

This is the original approximation and retained baseline. The
[continuous-corner correction](continuous/README.md) supersedes its curve model
and adds fresh iPadOS 27 masks plus aligned before/after glass overlays.

`CornerReference.m` renders solid white native CALayers on black, with circular
corners in the left column and continuous corners in the right column. The
native reference is iPadOS 26.4.1 at 2×. NXKit's corner regression fixture
renders the same five sizes/radii through the actual CALayer renderer. Both
images are cropped by recorded scene coordinates without image adjustment.

The portable continuous shape approximates UIKit's private curve with a
superellipse and cubic segments. A radius-dependent transition handles
capsules. The shape is shared by fill, clipping, borders, shadows, blur and
glass; the shader uses matching implicit distances and normals. Continuous is
the requested NXKit default, and circular remains an explicit option.

`metrics.json` gives full-shape RGB error and thresholded silhouette overlap
for each fixture. These solid-mask measurements isolate geometry and edge
coverage; they do not establish full Liquid Glass optical or GPU parity.
`comparison.png` enlarges the 52-point-radius corner without smoothing, so
individual antialiased pixels remain visible. `glass-edges.png` compares the saved native fresh light-mode glass corner with the earlier circular/double-coverage raster and the current production shader/continuous clip. The glass crop remains a raster material comparison, excluding NXKit layer shadows and GPU execution.

Reproduction from the repository root (Python needs Pillow and NumPy):

```sh
sh Demo/GlassComparison/build-corners.sh /tmp/nxkit-corners
xcrun simctl install DEVICE_ID /tmp/nxkit-corners/NativeCornerReference.app
xcrun simctl launch --terminate-running-process DEVICE_ID org.nxkit.NativeCornerReference
xcrun simctl io DEVICE_ID screenshot /tmp/nxkit-corners/native-screen.png
xcrun simctl get_app_container DEVICE_ID org.nxkit.NativeCornerReference data
# Copy Documents/corners.json from that container into /tmp/nxkit-corners/.
# With NXKIT_BUILD_TESTS=ON and matching platform Skia libraries:
NXKitCornerRenderingTests /tmp/nxkit-corners/nxkit.ppm
python3 Demo/GlassComparison/compare-corners.py /tmp/nxkit-corners/native-screen.png /tmp/nxkit-corners/corners.json /tmp/nxkit-corners/nxkit.ppm /tmp/nxkit-corners/report
```

The corner/AA raster regressions also cover fractional positions, both corner
curves, 1×/2×/3×, rotated layers, masks-to-bounds, inside border width, empty
bounds, clamped radii, XML, invalidation and presentation copies. The iOS build
uses current headers and Graphite archives. The local raster execution uses
the older archive's matching 2025 headers and a headless platform adapter;
it does not run a GPU backend or SDL window loop.
