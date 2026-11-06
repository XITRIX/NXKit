# Scroll-edge effects — iOS 26 compatibility

Working tree after `09e631b`, audited 2026-11-04. The visual reference is **iOS
26.0, release 23A343**, running Apple's UIKit in a dedicated simulator. iOS 27
appearance is not used as the target. The [native audit](native/README.md)
retains the runtime identity, API probes, 49 cases and original PNGs.

## Application API

```cpp
// Automatic discovery follows the controller root and then first subviews,
// stopping at the first UIScrollView or a leaf. It does not search siblings.
// For complex placements, select explicitly; nullptr restores discovery.
setContentScrollView(scrollView, NSDirectionalRectEdge::top |
                                 NSDirectionalRectEdge::bottom);
scrollView->topEdgeEffect()->setStyle(UIScrollEdgeEffectStyle::soft);
scrollView->bottomEdgeEffect()->setStyle(UIScrollEdgeEffectStyle::hard);
scrollView->topEdgeEffect()->setHidden(true);
```

Each edge returns the same independently mutable effect object throughout the
scroll view's lifetime. All default to `automatic` and visible. The C++ value
enum translates Apple's three singleton style objects; callers never create
or replace an edge object. Style metadata and scroll selection belong to
UIKitCore, which still builds without either component kit.

UIKit resolves automatic to soft. Its navigation/tab containers install each
screen's fading and blur on a private content host. Source and destination
hosts accompany their content through push, pop, and interactive navigation;
a retained screen is not painted through another screen's overlay. NXKit
resolves automatic, soft and hard to its currently supported hard appearance.
Its navigation and toolbar layers retain the shared material across screens.

The full-screen iOS Demo's Appearance section has a **Scroll edge** row that
cycles Automatic, Soft and Hard independently for each tab's collection screen.
Scroll through the content, push a navigation-arrangement screen, and pop or
cancel an interactive pop to inspect the per-screen effects.

The base `contentScrollViewForEdge` reports explicit bindings, rather than
publishing the bars' automatic heuristic. A direct `nullptr` override disables
an edge; forwarding to the base preserves automatic fallback. C++ cannot
reliably distinguish a forwarding override from an override that calls the
base and then always returns null, so use a direct null return to suppress an
edge. References are weak. The portable getter supports four individual
edges; iOS 26's observed getter accepts only top and bottom. Aggregate getters
reject; aggregate setters, including all/none, are supported.

For custom edge chrome within a UIKit navigation/tab screen:

```cpp
auto interaction = std::make_shared<UIScrollEdgeElementContainerInteraction>();
interaction->setScrollView(scrollView);
interaction->setEdge(UIRectEdge::left);
container->addInteraction(interaction);
```

Core's `UIInteraction` protocol and `UIView` interaction collection preserve
ownership, move callbacks, ordering, and safe reentrant mutation. The view owns
its interactions; the interaction and explicit selection reference views weakly.
Side safe insets alone do not activate native side effects. The interaction
associates overlay descendants with the appropriate edge. Current pocket geometry uses the associated container’s bounds, rather than
shrinking to its descendant labels/images.

## Rendering and private progressive blur

Soft pockets have two independent operations: a fixed content-opacity mask and a
custom variable-kernel blur. Blur sigma at each pixel is `4pt * gradientAlpha`.
The shader changes its sampling offsets with that value; it does not fade a
uniformly blurred image over a sharp image. Variable-kernel passes run along the one-dimensional edge gradient first and
perpendicular to it second. Narrow kernels and 1×/2× rendering retain full
resolution; broader 3× kernels use a prefiltered lower-resolution input and a
fused finishing shader, compensating prefilter/resampling variance. Source samples clamp at the current viewport boundary. A zero-radius
pixel returns the original sharp sample.

The private implementation uses one `UIScrollPocket` per physical edge, following
the `_UIScrollPocket` naming in the retained iOS26 reference. Public configuration
stays on `UIScrollEdgeEffect`. Bounds use the selected scroll view’s live safe
area and the actual associated edge-container bounds, clipped to the viewport.
There is no reference-device extension or assumed bar height. Activation uses
the scroll presentation offset and scales with the current pocket extent.

Content fade stays at 0.4 opacity at the outside, recovering to 1 inward. It is
separate from the radius field and independent of content coverage/luminance.
The neutral Core hook leaves application masks intact and draws content once
on the live canvas. There is no CPU coverage/luminance replay, adaptive black
backing or full-viewport mask-picture rasterization. Masks and filters are cached
by geometry, activation and display scale; nested glass observes the live backdrop.

See the [variable-kernel verification](variable-kernel/README.md). The earlier
[masked-uniform tuning](scroll-pocket/tuned/README.md), [initial optimization](scroll-pocket/README.md),
[adaptive comparison](comparison/README.md) and [Metal report](metal/README.md)
are retained as historical evidence and do not describe the current edge kernel.

`private/UIBlurEffect.h` explicitly exposes the factory observed in Apple's
shipped iOS 26 framework:

```cpp
auto effect = UIBlurEffect::effectWithVariableBlurRadius(radius, imageMask);
auto backdrop = new_shared<BackdropEffectView>(effect);
```

This is a **private UIKit extension**, not the public `UIBlurEffect` style API.
Image alpha zero selects sharp content, one selects the full radius, and a
ramp varies the radius spatially. The mask is normalized to the effect view.
Nil images and nonfinite/negative radii reject before installing an effect;
zero radius preserves the backdrop. Small private-effect radii use dense
separable sampling; larger radii use Gaussian variance levels, avoiding the
periodic artifacts of widely spaced fixed taps. ScrollPocket uses the separate
fixed Gaussian path described above. The private
factory uses an empirical 1.8 radius-to-sigma factor fitted to the native
radius-24 probe; default edge blur has separate calibration. This
approximates the observed behavior; it does not recover Apple's kernel or
accessibility adaptations.

## Evidence and limits

[Apple's content-scroll selection API](https://developer.apple.com/documentation/uikit/uiviewcontroller/setcontentscrollview(_:for:))
and [edge element interaction API](https://developer.apple.com/documentation/uikit/uiscrolledgeelementcontainerinteraction)
set the public contract. The native fixture also checks defaults, explicit
bindings, hidden edges, hard/soft/automatic, top/bottom rest and intrusion,
overscroll, side chrome, and private alpha masks. Native runtime inspection is
read-only apart from setting APIs in the isolated comparison app. Private
selector evidence is distinguished from public API documentation.

The portable fixture uses the actual UIKit/Core view tree on Skia raster, with
matching geometry, content offsets, traits and Apple fonts. Reproduce it with
`PortableScrollEdgeReference schedule.csv OUTPUT_DIR`; CSV columns are documented
at the top of `Demo/ScrollEdgeComparison/PortableReference.cpp`. The comparison
script generates paired images, geometry checks, per-row optical fits and hashes:

```sh
python3 Demo/ScrollEdgeComparison/compare-portable.py \
    docs/scroll-edge-effects/native PORTABLE_OUTPUT COMPARISON_OUTPUT
```

The earlier [paired comparison report](comparison/README.md) retains all 49
native/portable cases and dense/sparse profiles for the adaptive baseline. The
[fixed ScrollPocket report](scroll-pocket/README.md) records the new intentional
visual policy, unchanged edge geometry and before/after images.

These measurements separate carrier geometry from pixels. Low coordinate error
does not establish pixel parity. Whole-image residuals include independent
navigation/tab glass, glyph rendering, semantic colors and native chrome
vibrancy. Arbitrary descendant-shaped contours, multiple independent chrome
groups on one edge, automatic installation in standalone/custom containers,
accessibility/material adaptations and physical-device/Switch GPU visual parity
remain incomplete. No iOS 27 visual behavior, physical-device timing or native
animation parity is inferred from static iOS 26 comparisons.

## Build and regression verification

See the [variable-kernel report](variable-kernel/README.md) for current test,
build, shader and performance verification. The earlier [verification record](verification/results.json)
and [Metal report](metal/README.md) describe the previous adaptive implementation
and identify its exact binaries and shader ABI; they are retained for the
before/after comparison.
