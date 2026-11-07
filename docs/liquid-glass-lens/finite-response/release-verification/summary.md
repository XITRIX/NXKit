# Correction checks

- Raster LensView: 96 activation + 32 retirement cases passed.
- iOS Simulator LensView: same 128 cases passed.
- iOS Simulator UISlider: six native touch traces and moving-release bounds passed.
- iOS Simulator UISwitch: native position/lowering bounds and 96 attached-trait color cases passed.
- Live Metal tab motion and system-controls gallery: passed.
- macOS UIKit module: built successfully.

The tests hold lenses for three seconds while moving, lower them while movement
continues, verify retirement by two seconds after lowering, and re-lift during
the fade without a pose jump. The earlier lift-start timeout is superseded.

The Simulator Demo builds with its refreshed offline package. Cold-cache Metal
motion/gallery replays add zero shader-library or pipeline compilations during
rendering. The corrected signed build was installed and launched on iPhone Pro XITRIX on2026-11-07.
