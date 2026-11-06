# Native lens perimeter investigation

The reference is the running UIKit lens on **iPadOS 27.0 (24A434), 2×**.
This investigation corrects the scope of the earlier centered-star comparison.
Main ink remaining sharp does not imply that selected content bypasses every
optical pass. No Apple renderer source or private equation has been recovered.

## Reproducible oracle

`Demo/GlassComparison/LensReference.m` independently constructs the observed
private class in a native-only fixture. `build-lens.py` and `capture-lens.py`
build it, request each scene, and save the actual framebuffer and runtime
metadata. Representative [native captures, metadata, and source assets](native/edge-probes/)
are retained in this repository. The new full session is `/private/tmp/nxkit-lens-refraction-20261028`;
the earlier `/private/tmp/nxkit-lens-native` session is preserved.

The first pilot uses a 116×56pt lens at stage `(302.25,260)` on a regular-glass
640×76pt bar. Normal and selected sources cover `(-64,-64,768,204)` in bar
coordinates, giving **64pt overscan on every side**. Assets are 1536×408 RGBA
at 2×; the entire exported extent is populated. There is no lens-sized source
clip. The source contains a 16pt grid, differently colored vector crosses, and
native monospaced text crossing all four lens edges. Separate opaque coordinate
maps and sparse markers probe correspondence rather than relying on repeating
text. Per-case JSON records actual frames, source alignment, runtime getters,
feature geometry, color-space conversion, and source/executable SHA-256 hashes.

Framebuffers with an embedded ICC profile are converted to sRGB with LittleCMS;
those already carrying an sRGB chunk remain sRGB. Alpha source assets receive
the same conversion. No profile is guessed from the operating-system version.

## Observations

- **Direct main ink and strong edge deformation coexist.** In `pilot-portal`,
  central selected magenta crosses and text remain straight, while all four
  edges show strong compressed/folded grid and text, with separated color
  branches. The centered-star test did not exercise those branches.
- **Selected source contributes the distorted edge pixels.** `pilot-portal-only`
  hides normal content but retains the selected portal. Blue/cyan edge features
  remain. The selected interior is byte-identical to `pilot-portal`; whole-lens
  mean RGB difference is 7.70/255 because normal-content visibility changes near
  the perimeter. `pilot-underlay`, which removes the selected portal, instead
  produces branches from the normal red/green/purple palette.
- Changing `liftedContentMode` from 1 to 0 or `warpsContentBelow` from true to
  false keeps the measured central rectangle byte-identical but changes the
  perimeter. Against `pilot-portal`, whole-lens mean differences are 0.91 and
  2.31/255 respectively. These are configuration comparisons, not parameter fits.
- Native portal ownership suppresses ordinary selected-source display outside
  the lens while its source and ancestors remain visible. Setting a separate
  selected-source ancestor to alpha 0 or 0.001 removes both center ink and its
  perimeter branches. Alpha 1 restores the center byte-exactly; the whole-lens
  difference from the unwrapped portal-only source is only 0.053/255. NXKit's
  retained-source path therefore suppresses ordinary source output through weak
  lens ownership while the source and its host remain alpha 1. Source and ancestor
  opacity retain their normal meaning; no hidden host is used to fake a portal.
- Removing the public glass base changes the lens's sampled backdrop and body
  considerably, while selected-colored perimeter branches remain. A lens over
  regular glass and a bare lens are separate optical configurations. A second
  test places the rich grid in the background **beneath** the public glass,
  with no selected content. The lens over regular glass inherits strong frost;
  clear glass is mildly blurred, and the bare grid stays crisp. There is no
  observed raw-background bypass through regular glass in this fixture.
  Changing `warpsContentBelow` changes mean lens RGB by 13.48/255 with no base,
  6.95 with clear glass, and 0.88 with regular glass. This does not quantify the
  private flag's complete behavior, but separates selected-source sharpness from
  underlying-background frost.

## Inferences and unresolved details

The observations require access to the full selected-content source during the
edge pass. They reject both a final unwarped overlay as the complete model and
preclipping the source to the capsule before sampling. They support selected content becoming identity-mapped in the interior and
strongly remapped near the perimeter. They do not establish that a separate
unwarped pass remains visible underneath those edge samples.
The captures alone do not establish Apple's number of passes, exact compositing
order, filter implementation, or private material formula.

An opaque R/G coordinate-map pilot suggests a nonmonotonic inward fold near the
straight top edge: approximate decoded source y is 21.6pt at output y11pt,
16.0pt at output y16pt, then approximately identity by output y24pt. This is a
rough diagnostic, **not a fit**: perimeter shading changes channel values and
8-bit ramp quantization limits precision. Single-pixel line sweeps measure correspondence without interpreting shaded RGB
as coordinates. At the straight top midpoint of the 116×56pt style1 lens (top
edge at bar y10), subtracting the identical shader-only control gives:

| Source horizontal band (bar pt) | Native output support at midpoint (bar pt) |
| --- | --- |
| y4..5 through y16..17, in 2pt steps | No detectable ink |
| y18..19 | y10..19 |
| y20..21 | y19.5..21 |
| y22..23 | y22..23 |
| y24..25 through y40..41, in 2pt steps | Identity |

Support uses mean absolute RGB change above 10/255 in the 2× framebuffer.
The source line is exactly 1pt thick. This places the visible near-edge sampling
roughly 8pt inward, approaching identity within about 12pt of the perimeter.
The nonuniform magnification spreads one source band over many output pixels.
An unconditional direct overlay would incorrectly show the y10..17 source
bands, so the line sweep is stronger evidence than a centered glyph. The same phenomenon occurs at the curved left midpoint: source x270..271
spreads to output x262..271, x272..273 maps to x271..273, and x274 onward is
identity. The silhouette is x262.25pt, so x-source reach again is approximately
8pt. The finite band width and 0.5pt output sampling limit exact transfer recovery.

Explicit style0 changes the transfer even with the same geometry. A source band
at y22..23 appears at both y10.5..11 and y22..23; the outer branch reaches mean
RGB contrast 71/255 against its shader-only control. On the left, source
x274..275 appears at x262.5..263 and x274..275. Source y18..19 instead spreads
across y12.5..19, and source x270..271 across x265..270.5. This demonstrates a
nonmonotonic fold: approximately 12pt inward sampling at the silhouette,
falling toward approximately 8pt before returning to identity. Native runtime
metadata identifies explicit style0 as SmallVariantSpec and style1 as
LargeVariantSpec. These captures do not establish an automatic size threshold.

The colored grid alone does not isolate chromatic sampling. Full-width black
line sweeps are nearly achromatic after subtracting the no-content material,
but **those lines cannot detect channel offsets along their own length**.
Four additional isolated-marker captures close that blind spot. In style1,
the red marker at the top has its R centroid 1.36pt right of G and B 1.62pt
left of G. At the bottom, R is 1.54pt left and B 1.58pt right. On the left,
R is 0.97pt below G and B 1.07pt above; on the right those directions reverse.
Style0's channel centroids differ by less than 0.1pt in the same measurement.
The colored perimeter cannot be attributed only to differently colored rows.

The [marker measurements](native/edge-probes/marker-channels.json) use fixed
pixel regions and channel response normalized against the corresponding
no-content lens. Their source points, radius2pt, RGBA assets, actual frames,
and both-theme native captures are retained. Corner markers additionally show
two separated spatial branches. These observations constrain a two-dimensional
color transfer; they do not uniquely identify a channel-offset formula or the
number of private rendering passes. A uniform clockwise channel rotation does
not explain all four measured edge directions.

The broad matrix varies both themes, styles, sizes, fractional positions,
rest/lift, public regular/clear/no base, source modes, and lifted container.
A shader-only `contentSetup:none` control shares the exact nuisance parameters
with each scored source case. Main-ink and additional-copy errors must be
reported separately; a low body-average error can conceal a missing edge branch.

This simulator is 27.0, not the user's 27.2 device. Static captures do not verify
velocity-dependent stretching, touch cancellation, dynamic portal behavior,
accessibility variants, or universal native matching. The fixture's rich source
outside the platter is intentional diagnostic content, not proposed UI design.


## Capture and measurement commands

```sh
python Demo/GlassComparison/build-lens.py /private/tmp/lens-edge-reference
python Demo/GlassComparison/capture-lens.py DEVICE /private/tmp/lens-edge-reference Demo/GlassComparison/lens-edge-cases.json
python Demo/GlassComparison/capture-lens.py DEVICE /private/tmp/lens-edge-reference Demo/GlassComparison/lens-line-cases.json
python Demo/GlassComparison/capture-lens.py DEVICE /private/tmp/lens-edge-reference Demo/GlassComparison/lens-backdrop-cases.json
python Demo/GlassComparison/capture-lens.py DEVICE /private/tmp/lens-edge-reference Demo/GlassComparison/lens-color-cases.json
python Demo/GlassComparison/capture-lens.py DEVICE /private/tmp/lens-edge-reference Demo/GlassComparison/lens-marker-cases.json
python Demo/GlassComparison/analyze-lens-lines.py /private/tmp/lens-edge-reference/native /private/tmp/lens-line-support.json
```

The saved [line-support table](native/edge-probes/line-support.json) includes 64
source-line captures, both explicit styles, measured half-open pixel intervals,
actual lens frames, matched control IDs, and the fixed contrast threshold.
Each 1pt band is independently drawn; no native line correspondence is inferred
from the portable shader. Full source assets and raw framebuffer captures are
retained by the capture driver.

Changing the lifted-container reference to the whole glass view in this
particular manually composed hierarchy failed to acknowledge a capture. That
configuration is omitted from the reproducible matrix and has no parity claim.
The supported oracle here uses the glass content view as lifted container.
A real UITabBar's different private hierarchy remains separately documented in
[the runtime research](research.md).


Native `hidden = true` is a diagnostic limitation of this private composition:
the getter becomes true while sibling portal/compositor output remains visible.
The `hidden-lens-*` and `backdrop-grid-*-hidden` captures are therefore **not**
no-lens optical controls. Selected-source ancestor alpha0/0.001 controls, by
contrast, remove the selected ink as observed above. Portable `LensView` remains
subject to ordinary `UIView` visibility; it does not reproduce a private sibling
compositor escaping a hidden view.


An independent [radial-candidate check](native/edge-probes/radial-candidate-line-review.json)
verified the mechanism using rendered output, before adding chromatic transfer.
Both native and candidate remove the source band at y12..13. Both spread the
source y18..19 band over output y10..19; candidate contrast energy is 7.3% higher.
At the curved left edge, candidate support is 0.5pt narrower at each end, while
contrast energy differs by 0.3%. This validates radial source remapping without
using body-average error as the acceptance criterion. It does not establish
parity for the missing two-dimensional color transfer or every lens size.


## Accepted channel model and remaining differences

The independently reviewed C4 model applies the channel-coordinate perturbation
**before** the radial fold and composites each source channel with its own alpha
coverage. This corrects a held-out corner failure: applying channel offsets
after the fold turned a native cyan branch strongly blue. At the isolated cyan
outer corner, native R/G/B coverage sums are approximately 21/15/8; C4 gives
23/13/8, while the rejected post-fold prototype gave 0/8/52. The inner branch
has the same corrected ordering. These are coverage-weighted pixel counts,
not physical light energy. The [accepted marker review](native/edge-probes/accepted-marker-review.json)
records the geometry, source snapshot, decision, and limitations.

The temporary image-filter prototype was also rejected because a fractional
lens origin introduced a half-device-pixel source phase shift. Direct source
sampling restores every deep-center pixel in the rich pilot byte-for-byte to
the earlier sharp-center implementation. Independent channel destination
transmission avoids occluding the other backdrop channels around translucent
or antialiased ink. The main and edge branches share a live source; they are
not separate fitted icon copies.

Black text/grid on blue and colored-stripe backgrounds is an explicit holdout.
At the 620 original black source pixels within the edge band, error improves
from 74.9 to 33.6/255 on blue and 84.4 to 37.0 on stripes. Corresponding white-ink
dark-theme cases improve from 65.6 to 30.2 and 56.6 to 28.7. The
[fixed-mask measurements](native/edge-probes/colored-ink-review.json) include
source-support disappearance as well as displacement; they do not recover
individual native rays or establish equality.

Acceptance is a substantial correction of the missing source-refraction path,
**not pixel parity**. Native top/bottom spectral bands remain broader and softer;
the portable result has crisp, discrete channel stems and thinner folded grid
strokes. Side-channel separation is approximately 0.6pt too small across R to B,
and some corner centroids remain up to approximately 1pt away. Copy-region
errors remain materially visible. No additional blur model is inferred merely
to reduce those errors. The final aligned comparison should retain those
residuals and the simulator-version limit instead of hiding them in a low
whole-image or body-average score.
