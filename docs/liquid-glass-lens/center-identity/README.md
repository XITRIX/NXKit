# Lens center identity regression

[Open the before/after comparison](comparison.html).

The supplied screen recording shows a moving kink through the middle of the selected Library icon. This is a regression from applying the continuous-corner distance field to lifted-content sampling, not an intended lens fold.

The signed-distance approximation can report an interior depth greater than half the lens height. The selected-source shader blended the perimeter's folded coordinate back to the original coordinate, then clamped that final coordinate to half-height. This undid identity mapping and displaced interior content; its changing normal direction made the kink travel across the icon.

The correction bounds the perimeter fold **before** blending to the original coordinate. Deep-center sampling is consequently identity regardless of the approximate depth. The continuous silhouette, perimeter normals, chromatic source sampling and backdrop optics retain their geometry.

The retained-renderer regression uses an opaque Retina texture so it can compare source pixels directly without fitting colors or relying on a blurred background. It checks 48 combinations: light/dark, regular/selection, 95×54 / 111×70 / 116×56 / 70×110, and three translations including fractional positions. The previous core fails all 48 center checks; the corrected core passes all 48. The four exported examples show the 95×54 lens at x68.5, y40.25. Their center ROI is the same one scored by the executable. Nearest-neighbor enlargement in `overview.png` exposes sampling errors; the scored full-resolution images have no resampling or registration.

[Metrics and source/archive hashes](metrics.json) accompany the images. Existing lens, full lens-refraction, tab-composition and full-screen regression executables pass; the iOS Demo builds and links. Existing native one-point sweeps establish the intended straight center and perimeter folds in [source rendering](../source-rendering.md) and [edge refraction](../edge-refraction.md). This follow-up is a regression reproduction and correction, not a new complete UIKit optical comparison or physical-device execution of the corrected build.

To reproduce, build `NXKitLensRefractionTests` (or `LensRefractionTests` in the standalone raster harness). Run it without arguments for the full suite. Run `LensRefractionTests --center-only OUTPUT_PREFIX` to execute the 48 center cases and write four full-resolution PPM fixtures; omit the prefix to skip output. `build-lens-refraction.py RASTER_BUILD OUTPUT FROZEN_CORE --tests` builds the same executable against the frozen prior core without changing production code. Do not use its older `--before` overlay-composition branch for this comparison.

Export the viewer with `python Demo/GlassComparison/compare-lens-center.py RUN OUTPUT` using Pillow and NumPy. Keep the PPM fixtures, center logs and frozen core archives (`before-core.a` / `after-core.a`) in RUN. The exporter requires all 48 old-core failures, the successful new-core result and exact identity in every exported center ROI before writing the report.
