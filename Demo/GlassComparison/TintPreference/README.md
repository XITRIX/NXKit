# iOS 27 glass preference comparison

This independent native UIKit fixture crosses preference 0/.25/.5/.75/1,
regular/clear styles, neutral/opaque-blue tint, light/dark appearance, 44-point
circles/112×64 capsules, six flat backgrounds and high-frequency content.
The requested 0/.5/1 combinations comprise 24 distinct material configurations;
the intermediate positions guard interpolation. There are 30 scenes, 720 glass
instances, 480 flat-color controls and 80 textured-transmission controls.

Native materials use public `UIGlassEffect`, `tintColor`, `UIVisualEffectView` and
corner APIs. Only the fixture's preference adapter uses private exports. It sets
a **volatile, process-local** argument domain before UIKit's first getter; every
capture records the effective getter value. It never calls the system setter.
See `docs/glass-tint-preference/iOS27-control.md` for the adapter's evidence.

Use a booted iOS 27 simulator and a Python installation with Pillow and NumPy:

```sh
python3 Demo/GlassComparison/TintPreference/prepare.py /private/tmp/glass-matrix
xcrun simctl install SIMULATOR /private/tmp/glass-matrix/GlassTintMatrix.app
python3 Demo/GlassComparison/TintPreference/capture.py SIMULATOR /private/tmp/glass-matrix/native
cmake --build RASTER_BUILD --target GlassTintMatrix MaterialTests -j6
python3 Demo/GlassComparison/TintPreference/render.py RASTER_BUILD/GlassTintMatrix /private/tmp/glass-matrix/GlassTintMatrix.app /private/tmp/glass-matrix/portable
python3 Demo/GlassComparison/TintPreference/compare.py /private/tmp/glass-matrix/native /private/tmp/glass-matrix/portable /private/tmp/glass-matrix/report
RASTER_BUILD/MaterialTests
```

`capture.py` waits for each settled scene, captures the simulator framebuffer,
crops using the reported stage origin/scale, and acknowledges that specific
scene. `drawHierarchy` PNGs in the app's Documents are diagnostic only: they
reconstruct backdrop filters differently and must not be used as blur references.
The stage's unused outer padding may extend beyond a short simulator screen;
all glass shapes and sample regions remain visible.

The portable renderer uses the production retained view tree, material resolver,
image filters and raster compositor. Its optional third argument overrides frost
for isolated calibration, never for the passing comparison run. `compare.py`
fails above mean flat error 3/255, individual-channel error 16/255, mean texture
amplitude error 3/255, or maximum texture amplitude error 12/255. These are
measured approximation bounds, not a claim of complete visual parity.

Retained native captures/results: `docs/glass-tint-preference/matrix`.
`export-controls.py NATIVE OUTPUT` generates the compact native color table used
by `UIKit/tests/GlassMaterialTests.cpp`. Do not regenerate it from NXKit images.
