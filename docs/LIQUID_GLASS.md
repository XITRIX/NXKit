# Liquid Glass material and interaction

`UIGlassEffect` adapts the paraxial lens geometry
from [`liquid_glass_widgets`](https://github.com/sdegenaar/liquid_glass_widgets),
pinned to `5d2859796ef0f6964e2fdf25962b033eede27814`. It uses analytic SDF normals
instead of the reference's geometry texture, with NXKit's live scene behind it.
Its current material is calibrated against an actual UIKit Simulator capture,
using the reproducible [`Demo/GlassComparison`](../Demo/GlassComparison/README.md)
fixture. It does not use the reference's lightweight fallback appearance.

Regular glass uses a smooth quintic inward refraction shoulder, with zero slope
at the bevel endpoints and a softer interior displacement. This reduces
the angular, doubled source folds. Neutral regular material combines a broad
frosted body with a size-dependent detail contribution: positive sharp-minus-blur
in light appearance and negative detail in dark. Both styles keep their material
through the bevel, with no transparent edge band. Clear retains its original
paraxial fold and two-point frost. Neutral regular defaults to 6–12 points in
light appearance and 12 in dark; opaque regular tint retains 3–6 points.
Explicit optical setters override these defaults. Light regular glass additionally
filters chroma over a broader neighborhood without changing its luminance; larger
neutral controls soften the sharp detail lane. The actual resolved frost bounds
both kernels, and zero frost disables them.

Regular vibrancy retains extended RGB until after tint and dark-body compression.
Clamping earlier washed out saturated backgrounds. Body density, contrast,
vibrancy and dark compression follow presentation size and inherited appearance.
Custom regular tint responds to source luminance and chroma, with size-dependent
neutral highlights in dark appearance. Tint alpha controls its contribution;
an explicit `tintOpacity` selects the low-level optical tint path. Clear tint
keeps its existing transfer.

Regular edges use two directional light lobes with a narrow head and soft tail;
there is no all-around white hairline. Light angle, strength, width, directional
power and dark/bright backdrop strengths remain configurable. Clear retains its
0.8-point hairline and exterior-color reflection controls. Optical distances
remain in local points; filters run in pixels, and glass hosts now adopt their
screen's display scale, including active presentation layers. A same-screen
explicit layer scale override survives layout and theme changes.

Regular glass also has a separate half-point exterior dark contour, attenuated
at the reflection poles and calibrated in both themes on light/colored
backgrounds. Light material retains a tight white core even when its body is
almost white. This contour darkens only the backdrop contribution, preserving
partially covered material over black; it is independent of the elevation
shadow and honors edge strength, absorption and specular width overrides.
Containers apply it once around the merged SDF, so it never follows internal
member edges. See the [contour/highlight overlays](liquid-glass-rim/comparison.html),
which include the exterior band omitted from the earlier inner-rim scores.

The [current iPadOS 27 optical matrix](liquid-glass-refraction/README.md) contains both themes,
five sizes and ten backgrounds, with neutral/blue/coral tint. Its viewer aligns
native and NXKit pixels from observed geometry and provides overlays, absolute
differences and transparent residuals that remove exact matches (or separately,
pixels whose every channel differs by at most 2/255). There is no fitted position,
color or exposure correction. Earlier hierarchy captures embed Display P3;
the shared loader converts embedded profiles to sRGB before comparison. Fresh
framebuffer captures declare sRGB directly. Earlier raw-channel tint metrics are
superseded by these color-managed comparisons.

The [retained-join matrix](liquid-glass-interaction/retained-joins/current-combined/README.md)
compares both themes, both insertion orders, neutral/blue and blue/coral pairs,
and items that remain connected after release. Explicit regular tint takes
priority over neutral glass; equal-priority materials follow painter order
through a smooth ownership transition. Each active press light stops at its source
silhouette, then lights whichever material owns that group-space point. This
allows a pressed rear tint to illuminate the front tint in their overlap while
keeping the extruded bridge unlit. The calibrated union
contour remains unchanged.

Default regular glass shadows now use a shifted-distance Gaussian survival
profile in both themes; clear glass has no automatic exterior shadow. Shared
containers adopt their largest regular member's elevation independently of the
container's overall bounds. Mixed-size neutral regular members also share
material defaults resolved at the largest neutral short side, retaining their
own lens geometry and explicit overrides. Clear and explicitly tinted materials
keep independent size resolution. The [shadow report](liquid-glass-shadows/README.md)
and [mixed-size controls](liquid-glass-shadows/shared-material/README.md) retain
native evidence and remaining limitations.

The portable `LensView` extension adds a caller-controlled resting/lifted lens
with explicit regular/selection variants. Open **Glass → Interactive tab bar** for the dedicated floating-bar demo.
It uses a regular-glass base and independent selection lens, with tap/drag
selection, artwork/gray/black/white/colored-stripe/grid backdrops, persistent lift,
next-tab and height controls. The separate **Lens · tab bar and knobs** study
retains compact and wide layouts and standalone knobs. This is
not a complete UIKit tab controller. See the [native lens research](liquid-glass-lens/research.md),
[selected-content refraction](liquid-glass-lens/refraction/comparison.html),
[background chromatic comparison](liquid-glass-lens/chromatic/comparison.html),
[liquid motion replay](liquid-glass-lens/motion/comparison.html) and
[combined review](liquid-glass-review/comparison.html). Exact caustics and native
control tracking remain incomplete.

For a control with a stationary selected-color row, keep that row beneath the
lens in the same view hierarchy and supply it as a weak optical source:

```cpp
auto bar = new_shared<UIView>(NXRect(0, 0, 640, 76));
auto selectedRow = new_shared<UIView>(bar->bounds());
bar->addSubview(selectedRow); // Populate with the selected-color icons/labels.
auto lens = new_shared<LensView>(LensView::Style::selection);
lens->setFrame({262, 10, 116, 56});
bar->addSubview(lens);
lens->setLiftedContentView(selectedRow);
lens->setInteractive(true);
lens->setLifted(true, true);
```

The lens samples the complete presented source before masking its final output.
Moving the lens leaves the row in place. The input's ordinary drawing is
suppressed automatically while borrowed, so antialiased/translucent ink is
composited once. Keep actual alpha and hidden properties meaningful; do not use
an alpha-zero ancestor to hide the row. Source and ancestor visibility remain
effective. Clearing the source restores its ordinary drawing; callers retain
ownership. `contentView()` supplies the owned input when no external source is
set. The containing control remains responsible for its ordinary-color row and
selection/input routing. The demo's **Refraction grid** button exposes the edge
bending while dragging.

Elastic response is separately opt-in with `setInteractive(true)`; it does not
choose a selected tab, change the model frame, or lift the material for callers.
The default `InteractionSource::contact` lets a fixed knob react to a pull.
A control that moves its lens selects `InteractionSource::viewMotion` to use
its placement history, including ancestor movement. The two inputs are exclusive.
Contact mode is a portable extension; native calibration uses moving system lenses.

Forward a complete contact lifetime in stable superview coordinates, with
velocity in points per second:

```cpp
lens->setInteractionSource(LensView::InteractionSource::viewMotion);
lens->beginInteraction(touchPoint);
// Update the caller-owned lens frame during the drag, then forward contact.
lens->updateInteraction(touchPoint, touchVelocity);
lens->endInteraction(); // Retains spring momentum while removing the input.
// Use endInteraction({}, true) when the control cancels the drag.
```

Direct lens touches use the same path through a passive observer. A control
can also forward explicit input when the lens's ordinary hit testing is disabled.
The Demo enables both the base `UIGlassEffect` and selection `LensView`; their
observers leave ordinary tap actions intact. Rendering-only deformation moves
the optical surface and ordinary-content punchout together while selected ink
stays in its underlying stationary row. The [motion report](liquid-glass-lens/motion/README.md)
separates that response from native tab/slider placement and lift sizing.
The Demo approaches the finger only for touches starting outside the presented
lens silhouette; touches inside preserve their grab offset.
`BackdropEffectView::interactionTransform()` exposes the visual-only affine
shared by model and presentation layers, so controls can undo platter inflation
when mapping contact coordinates. Cached layer contents use linear sampling
under fractional placement and deformation, retaining exact whole-pixel 1:1
coverage. See the [tab content comparison](liquid-glass-lens/tab-content/README.md).

Lifted selected content uses a measured inward fold near the perimeter and an
identity map in the interior. The selection variant separates color channels
before that fold, including their independent coverage: black glyphs refract
and occlude a colored backdrop correctly instead of becoming tinted overlays.
Direct picture sampling preserves fractional positioning and sharp center ink.
The [tinted-source comparison](liquid-glass-lens/selected-chromatic/comparison.html)
checks selected ink separately from ordinary white icons in both themes and
strengthens its measured axial color separation. Colored-background and corner
validation retain the remaining differences rather than hiding them in a
backdrop-only score.
The [source-rendering notes](liquid-glass-lens/source-rendering.md) describe the
portable reconstruction and its bounds. Native corner position, channel spread,
and the softness of spectral bands still differ visibly.

The selection lens also separates ordinary backdrop RGB rays near the perimeter.
Its measured tangential color ordering is independent of the selected-source
fold. The chromatic report retains the fine-grid footprint regressions as well
as improved marker placement; adding color separation does not establish exact
spectral parity.

These are observable-runtime approximations, not Apple's private material
formula. Mixed-color stripes, continuous-corner optical geometry, and exact
refraction still have visible residuals. The measured runtime is iPadOS 27.0;
this does not establish physical iOS 27.2 or Metal/Dawn parity. See the saved
[model analysis](liquid-glass-optics/optical-model.md) for rejected candidates and
known tradeoffs.

The MIT notices are retained in
[`licenses/liquid_glass_widgets-MIT.txt`](../licenses/liquid_glass_widgets-MIT.txt).
`BackdropEffect::glass()` resolves the clear material; install `UIGlassEffect`
on a view for size-adaptive regular styling and touch feedback.

## Optional iOS 26 appearance adaptation

```cpp
UIGlassEffect glass(UIGlassEffect::Style::regular);
glass.setAdaptsAppearanceToBackdrop(true); // NXKit extension; default false.
auto glassView = new_shared<BackdropEffectView>(glass);

// Changing an installed effect:
auto updated = *glassView->glassEffect();
updated.setAdaptsAppearanceToBackdrop(false);
glassView->setEffect(updated);
```

The flag restores contrast-driven light/dark selection for small regular
glass. Dark backdrops select dark material and light dynamic foreground colors;
bright backdrops select light material and dark foreground colors. Mid-contrast
content prefers the inherited app appearance. Large regular glass follows the
inherited appearance, and clear glass ignores the flag. The default does not
switch appearance from backdrop contrast. Optical tone and appearance selection
are separate; the former is calibrated to the theme-fixed iPadOS 27 material.

`BackdropEffectView` keeps its own inherited traits and supplies the selected
appearance to ordinary descendant views, including views added after selection.
Dynamic colors such as `UIColor::label` follow it; fixed colors stay fixed.
Controller-owned roots retain the existing controller trait propagation rules.
Disabling the flag, installing clear/custom glass, enlarging the glass, and
reparenting restore or recompute inherited colors. XML accepts
`adaptsAppearanceToBackdrop="true"` and rejects malformed booleans.

The GPU path takes a prefix snapshot before drawing the material and reduces
it to an 8×8 asynchronous readback, with one request in flight per glass host.
Completions run through the main queue, are rejected after replacement or
reparenting, and request another sample if the backdrop changed during a
transfer. The initial frame uses inherited appearance until sampling completes.
The default-false path performs no snapshots or readbacks. Ordinary size and
optical parameters now resolve from presentation-layer bounds during animation.

Apple's thresholds are private: NXKit currently treats a shortest dimension
below 100 points as small, uses app-dependent luminance thresholds (0.25/0.75),
and a 0.035 hysteresis band. These approximate the controlled 56/64-point and
160-point fixtures; they are not claimed as exact UIKit thresholds. Sampling
uses the visible surface's rectangular area, so rotated shapes and isolated
opacity/mask groups need further backend validation. Existing dark material and
explicit tint optical mismatches also remain; this switch restores appearance
selection rather than claiming complete native material parity.

The fixed-size black/white/gray/checker controls in
[`liquid-glass-ios27/contrast`](liquid-glass-ios27/contrast/results.json)
confirm the small/large distinction on iPadOS 26.4.1. On the tested iPadOS 27.0
runtime, both sizes retain the inherited appearance. These observations are
runtime-specific, not an Apple guarantee about later releases.

## Continuous corners and edge coverage

```cpp
glassView->layer()->setCornerRadius(28);
// Already the default for every NXKit CALayer:
glassView->layer()->setCornerCurve(CALayerCornerCurve::continuous);
// Explicit opt-out:
glassView->layer()->setCornerCurve(CALayerCornerCurve::circular);
```

`cornerCurve()` and `setCornerCurve()` follow NXKit's getter/setter convention
for [Apple's `CALayer.cornerCurve`](https://developer.apple.com/documentation/quartzcore/calayer/cornercurve).
NXKit deliberately defaults to continuous corners. XML accepts
`cornerCurve="continuous"` or `cornerCurve="circular"`; invalid values fail
visibly. Copies and presentation layers preserve the curve, and changing the
curve invalidates rendering and updates an already active presentation. The
curve itself switches discretely; animated radius and bounds keep resolving
the selected geometry at each presentation size.

Backgrounds, inside borders, shadows, clipped subviews, blur and glass share
one rounded shape. Glass refraction normals, rim distance and its interactive
corner check use the corresponding continuous geometry. Continuous corners
use a bounded-cache rounded-superellipse/circular-arc reconstruction adapted
from Flutter's BSD-licensed implementation. Unlike the former near-circular
capsule approximation, this preserves the long easing transition into the flat
edge. Shared CPU-precomputed octants drive the path, shader distance and normal,
including container joins, exterior contours and lifted-lens sampling. Fresh
solid CALayers on iPadOS 27.0 match the retained 26.4.1 reference byte for byte.
The private UIKit spline remains approximate; the updated
[corner overlays](liquid-glass-corners/continuous/comparison.html) and
[measurements](liquid-glass-corners/continuous/metrics.json) expose residuals in
both solid masks and regular glass across both themes.

Glass edge coverage is applied once by the antialiased clip in device space.
The built-in shader returns fully shaded material to this clip instead of
applying a second edge fade. `masksToBounds` reuses the layer's existing clip;
it does not add another glass clip. Standalone shader users retain analytic
coverage unless their host supplies `externalShapeCoverage = 1`. Custom shaders
can read the automatic `cornerCurve` float (0 circular, 1 continuous) and opt
into the same single-coverage contract.

`NXKitCornerRenderingTests` compares glass and solid-layer coverage at fractional
positions, 1×/2×/3×, with and without masks, and under rotation; it also checks
inside borders, XML, invalidation, copies, active presentation updates, empty
bounds and radius clamping. Corner, material and retained-appearance tests pass
on the headless raster fixture. The iOS Demo and regression targets build/link;
end-to-end Metal/Dawn appearance and performance still require device testing.

## UIKit-shaped interaction

```cpp
UIGlassEffect glass(UIGlassEffect::Style::regular);
glass.setInteractive(true); // Default is false, like UIKit.

auto glassView = new_shared<BackdropEffectView>(glass);
glassView->setFrame(NXRect(20, 20, 132, 56));
glassView->layer()->setCornerRadius(28);
```

This follows Apple's opt-in [`isInteractive`](https://developer.apple.com/documentation/uikit/uiglasseffect/isinteractive)
configuration, shown in [WWDC25's UIKit session](https://developer.apple.com/videos/play/wwdc2025/284/).
Effects are copied into their host. To change an installed effect, edit a copy
returned by `glassEffect()` and call `setEffect()` again. A flag on the effect
controls visual feedback; it neither enables nor disables normal view input.
Canonical `UIVisualEffect` inheritance and `UIVisualEffectView/contentView` are
still outside this implementation; `BackdropEffectView` hosts the effect.

- Real UIKit touch traces now calibrate two interaction modes. On the tested
  iPadOS 27.0 runtime the longest-side boundary is 120 points (119 is soft,
  120 is stiff), rather than 80. Small glass grows 16 points; large inflation
  interpolates from 16 to 4 points using the shortest dimension. This is an
  observed private policy, not an Apple API guarantee.
- Dragging adds centered, independent-axis squash/stretch. Pull resistance
  changes when the finger crosses the edge; large glass resists it more strongly.
  Press and release springs use the measured size-dependent damping; the pressed response
  remains active until that finger ends or is cancelled. Additional fingers do
  not replace the tracked finger or release its highlight.
- Press lighting rises over 150 ms and disappears within 60 ms of release.
  Regular neutral and tinted materials use separately measured light/dark
  overlay strengths; clear retains the earlier response. Long neutral regular
  platters instead brighten transmitted color with a slight chroma gain,
  calibrated against ten native phone tab-bar cases in both themes. This avoids
  turning a colored platter into a white plate. See the
  [aligned tab-press comparison](liquid-glass-lens/tab-press/comparison.html).
  The spring returns through a small undershoot while retaining its velocity
  when interrupted by another press.
- A passive recognizer observes touches on the glass and its descendants. It
  cannot cancel or prevent button, pan, scroll or application recognizers.
- Feedback is a rendering transform, composed after the application's transform.
  Public bounds, layout, hit-test geometry and transforms stay app-owned. Model
  and presentation layers share the live motion, including during app animations.
- Disabling the effect or installing a custom shader immediately clears feedback.
  Removal/reparenting, ancestor hiding/disabling, and expired touches reset the
  interaction on the next main-loop callback. Display links run only on the main dispatch queue; background queues drain only their own work.

The XML spelling is `isInteractive="true"` on `BackdropEffectView`; malformed
values fail attribute parsing. `edgeAbsorption` is a validated optional optical
override (0–1). Set `layer()->setShadowOpacity(0)` to suppress the default glass
shadow; an explicit layer shadow takes precedence.

The [53-case UIKit interaction comparison](liquid-glass-interaction/README.md)
retains actual input and presentation rectangles, full-curve replay tests,
mixed-tint merge captures, acquisition limits and reproduction instructions.
Grouped items shade exact per-item material modes in their own coordinates,
then blend rendered colors. The smooth-union field's numerical gradient corrects
rim/optical distance at narrow joins without moving its calibrated contour.

## Verification

`NXKitGlassMaterialTests` executes the SkSL shader on Skia's raster backend and
checks default clear/regular body presence, tint, appearance, press light location,
clipping, empty geometry,
frame-rate-independent motion, point-based growth, release and interruption.
Actual two-input image-filter regressions check the clear inward lens fold,
regular frost through the bevel even with a 24-point override, and sub-point
rim coverage at 1×, 2× and 3×. Independent native sRGB controls now cover both themes, 52/120-point sizes,
neutral/blue/coral tint, seven solid backgrounds and three withheld pastel
backgrounds. Directed-light rotation/strength and explicit zero-frost/tone
overrides have focused regressions. Native black/gray calibration samples constrain
regular body tone independently of blur.
`NXKitGestureRecognizerTests` also covers descendant button delivery, cancellation,
simultaneous app pans, transform preservation, runtime disabling/removal and XML.
`NXKitDispatchQueueTests` verifies main-queue display-link confinement and invalidation.
`NXKitLayerRenderingTests` covers presentation-layer lighting/filter invalidation,
effect replacement, parameter propagation and the original custom-shader optical path.
`NXKitGlassAppearanceTests` covers live raster backdrop samples, dynamic foreground
pixels, newly inserted content, size animations, disabling/replacement, reparenting,
clear/large exclusions, XML validation and an intervening backdrop change during
a pending transfer.

With the required platform Skia archives installed, configure with
`-DNXKIT_BUILD_TESTS=ON`, build those five targets and run their executables on
the corresponding platform. Custom shaders may declare `sharpContent` for an unfiltered second input and
`filterScale` to opt into pixel-resolution filtering; existing one-input shaders
keep their original coordinate contract. The standalone material/motion test can also link
against a compatible raster-capable Skia archive without starting a GPU backend.

The Demo's Glass screen enables interaction for both clear and regular glass.
Press, hold, drag across and beyond each shape, release, and adjust the regular
lens size with the slider. Check light/dark appearance and 1×/2×/3× display scales.
The saved [native comparison](liquid-glass-comparison/comparison.png) uses the
same artwork, frames, radii and 2× scale: Apple UIKit on an iPadOS 26.4.1
Simulator versus NXKit's production material/filter graph on Skia's raster
backend. [Enlarged crops](liquid-glass-comparison/details.png) expose the edge
and refraction differences. Mean absolute RGB channel error inside the shapes
fell from 19.506 to 2.020 for clear, 63.633 to 3.901 for the regular card, and
65.436 to 3.230 for the regular bar. Those metrics exclude a four-point perimeter
and apply only to this scene. Native shadows, exact continuous-corner calibration, dark appearance,
touch motion, and end-to-end Metal/Dawn visual/performance verification remain
before claiming pixel-identical or completely UIKit-compatible results.

Verification on 2026-10-27: the standalone material/motion/appearance-policy,
main-queue and retained-view appearance tests passed on macOS. The appearance
fixture linked the portable UIKit core to the existing raster archive with its
2025-era headers (archived locally from Skia `5f2e30e0c8bd00b2ff314a6c751ceac95fedb913`)
and a headless platform adapter; it did not execute Graphite or the SDL window loop.
The iOS Demo and all five named regression targets compiled
and linked with the installed iPhoneOS 27.1 SDK; those iOS executables were not
run on a device. The normal macOS configuration stopped at the missing
`Submodules/skia/out/mac-arm64/libskia.a` prerequisite. The macOS raster check
used the existing `out/mac-arm64-angle/libskia.a`, without testing a GPU backend.
The Switch Graphite archive at `out/horizon/libskia.a` is also absent, so the
shared submission path is not Switch-build-verified.


## Dark, tint and retained-size audit

The subsequent [54-checkpoint comparison](../Demo/GlassComparison/README.md#dark-appearance-tint-and-retained-resizing)
uses the same native views while they grow through five sizes and shrink,
with neutral, opaque blue, and opaque coral tint inputs in both appearances.
[Dark/tint examples](liquid-glass-size-tint/dark-tints.png),
[light/tint examples](liquid-glass-size-tint/light-tints.png), and
[neutral size states](liquid-glass-size-tint/neutral-sizes.png) expose remaining
mismatches. The GIF previews in that directory show settled checkpoints with
illustrative timing, rather than animation-frame or latency parity.

At the time of that historical audit, regular dark glass was too bright and
saturated, explicit clear tint was lost, and regular tint had the wrong tone.
The gallery calibration now fixes clear tint and substantially improves both
regular tint and neutral dark tone. The [fresh-versus-resized capture](liquid-glass-size-tint/fresh-vs-resized.png)
also demonstrates a native neutral-tone change after resizing in light traits,
including when no animation is used. The earlier small error measurements
apply to the saved fresh light-mode scene, not retained resize behavior.
These images retain the old material for comparison; current measurements
and remaining gaps are recorded in the gallery report.


The resized neutral-tone change was specific to the tested **26.4.1** runtime.
The [27.0 validation](liquid-glass-ios27/README.md) uses the same native binary
and confirms stable light and dark themes through direct and animated resize
round trips, with less than 0.001/255 mean channel change at the restored size.
Do not reproduce the older runtime's theme flip as the iOS 27 target. This does
not establish removal of all size/backdrop-dependent color adjustment.

## Grouped glass

`GlassContainerView` combines child `BackdropEffectView` materials into one SDF
surface. See [usage, native comparison results, limits and reproduction](liquid-glass-container/README.md). The gallery includes separate, movable, grouped and two-tint demos using this container.
Explicit regular tint takes priority over neutral material, with a smooth
eight-point ownership transition; equal-priority materials follow painter order.
Geometry blending controls the outer SDF surface. Shared press lighting uses
the active source silhouette and the owning material at each point. The
[retained-join report](liquid-glass-interaction/retained-joins/README.md) records
both tint orders, persistent joins after release, improvements and remaining
internal shading differences.
