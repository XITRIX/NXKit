# Text rendering: native UILabel versus NXKit

[Open the aligned comparison](comparison.html). This review corrects the heavier
strokes and vertical offset reported in the full-screen glass demo.

## Findings and corrections

1. **Apple glyph coverage was being altered twice.** SkParagraph's default
   hinting requests Core Text font smoothing, including for grayscale text.
   Skia's default text surface also applies contrast/gamma coverage adjustments.
   Plain Apple text now uses unsmoothed grayscale glyphs and linear coverage,
   without changing the requested font weight or applying a visual thinning filter.
   UILabel bitmaps and the Apple platform surfaces use the same policy; actual
   UILabel, CATextLayer and UITextView tile coverage is regression-tested.
2. **Label placement used font-wide extrema.** `fTop`/`fBottom` include glyphs
   unrelated to the displayed text. UILabel now uses its primary typographic
   line box, with display-pixel rounding of the block, centered origin and
   baselines. It compensates for SkParagraph's whole-point baseline rounding.
   Taller emoji fallback no longer expands a plain label's primary line box.
3. **The resolved typeface was not necessarily the shaped typeface.** Setting
   `TextStyle::setTypeface` alone does not select SkParagraph's shaping face.
   The resolved face is now registered in its font collection under its real
   family, retaining fallback. Apple backends request UIFont/NSFont's actual
   system font at the requested point size and weight, instead of accepting
   Core Text's unnamed-family Helvetica fallback.
4. **Normal font tracking was missing.** Skia removes Core Text tracking from
   glyph advances. The renderer now restores the size-dependent normal
   horizontal AAT `trak` values from the font itself. This is not a hard-coded
   SF spacing adjustment. The parser validates lengths, offsets and size order;
   interpolation and malformed tables have focused regressions. Format source:
   [Apple TrueType tracking table](https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6trak.html).
5. **Measurement had independent errors.** Empty labels could report a huge
   negative width. Labels now return zero for empty content and round intrinsic
   size to display pixels without unconditional extra one-point width padding.
   Display-scale changes invalidate intrinsic measurement. Nonfinite label font
   size/weight and nonpositive sizes are rejected.

## Evidence

The independent reference uses public UIKit UILabel and Core Text on an iPhone
17 Pro running the iOS 27.0 Simulator, built with SDK 27.1. It exports native
font metrics, `sizeThatFits`, text rectangles, public first-baseline layout
anchor measurements, and a separate known-baseline Core Text raster control.
Display traits are explicitly updated to match each 1×/2×/3× renderer scale.

Six images cover both themes at those scales. Each has eleven rows: identical
Roboto file at 11/13/17/22/34 points, system regular/semibold/bold, multiline,
Unicode/emoji fallback and empty content. The NXKit images use the actual
UILabel raster/layer path and production Apple font resolver, with Core Text's
macOS font manager. No translations, fitted offsets, resampling or weight fitting
are used in the comparison images. Embedded profiles are converted to sRGB.

Thirty identical-file single-line cases isolate the renderer from font choice:

| Measurement | Before | Corrected |
| --- | ---: | ---: |
| Mean absolute ink-coverage error | 20.945% | 0.0051% |
| Mean absolute vertical ink-centroid error | 0.9091 pt | 0.00016 pt |
| Worst corrected vertical ink-centroid error | — | 0.00087 pt |

Ink coverage is the sum of foreground coverage, not a claim that every pixel
matches. The corrected acceptance checks require coverage within 0.2% and
vertical centroid within 0.51 points in every same-file single-line case. All
30 pass; actual results are substantially tighter. Full per-row errors,
including glyph-edge/position residuals, are in [metrics.json](metrics.json).

The before executable and source were archived before the changes. Its existing
headless default-font hooks returned no explicit face/family; system-font rows
therefore also include the corrected test-harness resolution. Identical-file
Roboto cases force the same Core Text family in both revisions and isolate the
actual label/coverage corrections. Native and current source/build/capture hashes
are recorded in [provenance.json](provenance.json).

![Same-file body text: native, before, corrected](body-comparison.png)

## Fractional label widths

The later [tab-caption comparison](../liquid-glass-lens/tab-content/caption-comparison.html)
exposed SkParagraph's default whole-point width flooring. UILabel now disables
that compatibility mode: a caption measured at 29⅓ points must also fit at
29⅓ points. Focused 1×/2×/3× regressions cover intrinsic-fit stability and
wrapping at genuinely narrower widths, with medium/semibold system fonts.
Unattached labels can measure before inheriting display traits.

## Remaining limits

System rows use the iOS UIFont and macOS NSFont versions respectively. Most
system regular/headline geometry now aligns, but the 22-point bold control
retains a one-pixel baseline difference at 3×, and emoji artwork/scaling differs.
The two-line control retains about 0.17-point aggregate vertical centroid error
at 3×. HarfBuzz/Core Text glyph positions and edge quantization still leave pixel
residuals even when total coverage and vertical centroids agree. This report does
not establish complete typography or font-fallback parity.

The public UIFont/descriptor API, Dynamic Type, attributed text, line-count and
truncation APIs remain incomplete. These tests do not establish arbitrary affine
transform, physical-device, Metal/Dawn or Switch text pixel/performance parity.
The iOS Demo builds and links; macOS executes the actual retained raster tests.

The font control uses the existing Skia resource `Roboto-Regular.ttf`, copyright
2011 Google Inc., licensed under [Apache 2.0](https://www.apache.org/licenses/LICENSE-2.0).
No Apple font files are embedded or added to the repository.

## Reproduce

```sh
python3 Demo/TextComparison/build-native.py /private/tmp/nxkit-text-native
xcrun simctl install DEVICE /private/tmp/nxkit-text-native/TextReference.app
xcrun simctl launch --terminate-running-process DEVICE org.nxkit.TextReference
python3 Demo/TextComparison/capture-native.py DEVICE /private/tmp/nxkit-text-native/captures
cmake --build /private/tmp/nxkit-container/raster-tests --target TextRendererTests GlassScreenTests LensTabTests AdaptiveTabTests LensRefractionTests -j4
ctest --test-dir /private/tmp/nxkit-container/raster-tests -R '^(TextRenderer|GlassScreen|LensTab|AdaptiveTab|LensRefraction)Tests$' --output-on-failure
python3 Demo/TextComparison/compare.py /private/tmp/nxkit-text-native/captures /private/tmp/nxkit-text-before /private/tmp/nxkit-container/raster-tests/TextRendererTests docs/text-rendering
```

The comparison command requires Pillow and NumPy. Its second argument is the
preserved pre-change renderer directory containing six PPM/JSON renders and the
archived executable/source; those historical inputs must not be regenerated
with corrected code. Published before PNGs remain in this report's `images/`.
Configure `Demo/GlassComparison/RasterTests/CMakeLists.txt` with compatible Skia
archives/headers and `NXKIT_RASTER_CONTROLLER_TESTS=ON` for controller checks.

The [full-screen glass report](../liquid-glass-lens/screen/comparison.html) is
also refreshed using the corrected production text renderer, retaining its six
native SwiftUI states. Earlier typography metrics and overview are preserved in
its `typography-before/` folder.
