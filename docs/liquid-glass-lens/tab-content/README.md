# Tab content and lens grabbing

[Open the aligned UIKit / NXKit comparison](comparison.html). The ten cases
reuse the retained native UITabBar observations from the
[press comparison](../tab-press/README.md): light/dark, neutral/blue/brown,
and the independent coral/green validation captures. Native observations were
not recaptured for this change. Caption fonts and glyph positions are newly
measured with a public UIKit probe; native-scale screen pixels reuse the
original 3× full-screen captures. NXKit images are new renders of the actual
Demo composition. All ten cases include the previous NXKit pixels, before the caption correction.

## Caption styles and placement

[Inspect the native 3× caption comparison](caption-comparison.html).
The prior Demo used the general 11pt regular Caption 2 style and 90×18pt
centered text frames. Actual UITabBar captions use **10pt semibold** ordinary
ink and **10pt medium** selected ink, with natural alignment in tight **12pt**
frames starting **39pt** below the platter top. Each width is measured from the
resolved font and the leading edge is rounded up to the display grid.

`TabCaptionReference.m` observes actual public UITabBar/UILabel properties and
Core Text glyph positions through `simctl spawn`, without private selectors.
The native font probe confirms all eight font names, sizes, frames and advances.
At 3×, the maximum frame-coordinate difference is **0.00034pt** in both themes;
all glyph identifiers match and glyph-position error is **0.000066pt** or less
(the output precision of the C++ diagnostic). `caption-metrics.json` retains
the checks, source/binary hashes and original screenshot hashes.

The tighter frames exposed whole-point width flooring in SkParagraph. UILabel
now preserves fractional widths: a label fitted to 29⅓pt must not wrap at
29⅓pt. Twenty-four scale/weight/word regressions cover exact-fit and genuinely
narrower widths at 1×/2×/3×. Labels also measure before window traits arrive.
Geometry-only chrome harnesses retain fallback widths until a font context exists.

These are font/geometry results, not a claim that every glyph-edge pixel is
identical. The native-scale viewer shows remaining raster edge quantization,
selected-ink vibrancy and underlying-glass differences without fitted shifts.

## Earlier corrections

- A touch outside the presented continuous lens silhouette animates its center
  toward the finger. A touch inside catches the presented center and preserves
  the finger offset, including during containing-glass inflation. Routed-touch
  tests cover off-center grabs, small movement before pan recognition, external
  approaches, capsule corners, release/cancellation and continued lift.
- Cached layer contents previously used nearest-neighbor sampling. Fractional
  placement and the glass transform magnified its discrete pixel steps. Linear
  sampling now interpolates those bitmaps, including masked text and symbols.
  This leaves integer-aligned 1:1 pixels unchanged. Retained-layer tests cover
  eight subpixel phases, masked/unmasked, at 1×/2×/3×. Lens refraction and text
  regressions also pass. [Current 3× resting crop](rest-3x.png).
- Four-tab symbols now fit the independently recorded native UIImageView bounds
  instead of a common 24-point square. Native UIKit and AppKit symbol padding
  differs, so increasing the nominal font size alone was insufficient. The
  normal and selected rows share these bounds. Navigation symbols retain their
  existing intrinsic sizing.
- Contact alignment exposed incorrect affine inversion: translation was not
  reversed, reflections were rejected and singular matrices were accepted.
  Correct inversion prevents the grab offset drifting as the platter expands.
  Round-trip and invalid-input regressions cover the correction.

## Native symbol evidence

Coordinates below are relative to the resting 360×62-point platter. Dimensions
come from UIImageView entries in the retained native hierarchy, independently
of the NXKit layout. The maximum frame-coordinate discrepancy across all ten
cases is 0.167 points, below one native 3× pixel.

| Symbol | Top | Width | Height |
| --- | ---: | ---: | ---: |
| Home | 10 | 33.333 | 28 |
| Favorites | 9 | 30.333 | 28.333 |
| Library | 9 | 33.333 | 29.333 |
| Settings | 12 | 29.333 | 23.667 |

The caption correction reduces ten-case mean full-crop channel MAE from
6.194 to 5.380/255 at rest and 6.861 to 6.137/255 while held. Full-crop scores include glass, fonts and ink
vibrancy; they do not isolate typography. The current exact figures are in
`metrics.json`. All ten cases retain the native lift-size, platter-scale and
press-color gates. The separate caption geometry gate compares native 3× data;
the ordinary 2× report can differ by one 3× pixel when rounding to its own grid.

## Method and limits

Stage/window metadata determines cropping. Both sides are color-managed to sRGB
at 2×, with no fitted registration, color adjustments or warping. The viewer
provides 50% overlay, absolute difference and removal of matches within 2/255.
Full native snapshots/traces, capture manifests and current source/renderer
hashes are retained. `baseline-provenance.json` identifies the earlier pixels.
The separate 3× crop is generated directly by the retained Demo renderer and
is not upscaled from the comparison images.

Resting selection fill, selected-ink vibrancy, spectral/rim optics and platform
symbol rasterization still differ. Native evidence is iOS 27.0 Simulator;
physical-device and end-to-end Metal/Dawn parity are not established here.
The conditional grab rule follows the requested behavior; current gesture tests
exercise NXKit input routing, rather than claiming a newly captured native
inside-versus-outside gesture timing comparison.

## Reproduce

Build `TabPressRenderer`, `LensTabTests`, `LayerSamplingTests` and
`GlassScreenTests` and `TextRendererTests` in the raster comparison project. With the two native
capture directories from `tab-press-cases.json` and
`tab-press-validation-cases.json`:

```sh
python3 Demo/GlassComparison/compare-tab-press.py ORIGINAL_CAPTURE RENDERER OUTPUT docs/liquid-glass-lens/tab-press/images --content
python3 Demo/GlassComparison/compare-tab-press.py VALIDATION_CAPTURE RENDERER OUTPUT --append --content
```

Compile `TabCaptionReference.m` for arm64 iOS Simulator with UIKit/CoreText and
run it through `xcrun simctl spawn DEVICE PROBE`, retaining stdout as
`OUTPUT/native/caption-fonts.json`. Then run:

```sh
python3 Demo/GlassComparison/verify-tab-captions.py RASTER_BUILD OUTPUT
```

Run the focused tests through CTest. The iOS Demo build also succeeds with these
production changes. No private UIKit APIs are used by the implementation.
