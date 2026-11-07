# Shared application-facing lens configuration

2026-11-07, working tree after `b895e97`. `LensView` is an NXKit extension in
UIKit. Its configuration follows the useful app-callable portion of private
`_UILiquidLensView`; it is not an Apple public UIKit class or a binary-compatible
replacement. [The complete audit](api-audit/README.md) retains runtime metadata,
shipped-code inspection and fresh app-space calls, with Evidence IDs and limits.

The 2026-11-07 [finite response follow-up](finite-response/README.md) applies a
shared two-second elastic fade after lowering to all clients, at the user's
request. Elasticity remains active throughout the lifted state, including
deceleration after contact ends. Lifted geometry, optics and control motion remain independent.
This is a portable policy in addition to the captured native settings.

## Public surface

| Native input | Portable API |
| --- | --- |
| initWithFrame / style | frame constructor; Style::small / large (regular / selection aliases retained) |
| initWithRestingBackground | constructor accepting shared_ptr<UIView>; hasCustomRestingBackground / restingBackgroundView |
| restingBackgroundColor | nullable override; a custom resting view retains its own appearance |
| restingBorderColor | nullable border; native 3pt resting / 0pt lifted width |
| restingShadowProperties | nullable shared_ptr<UIShadowProperties>, applied under the resting fill |
| _UIShadowProperties | public portable UIShadowProperties: color, opacity, offset, radius, copy, scaleBy, updatedShadowWithIntensity, propertiesFromView, applyToView |
| liftedContentView | weak selected-content source |
| liftedContentMode | preserveSource=0 / hideSource=1; names are portable, values/behavior native |
| warpsContentBelow | default false; independent of transition blur |
| setLifted:animated:alongsideAnimations:completion: | four-argument setLifted with shared minimum-lift lifetime, alongside model changes and grouped two-boolean completion |

The two-argument convenience remains. Native normal completion is observed as
`(true,false)`; names for those native flags are not established. NXKit reports
that pair when its group settles, including collected callbacks across retargets.
Same-state four-argument calls run alongside without completion. Nonanimated
changed calls run alongside/completion synchronously. Alongside geometry uses the
portable UIView animation facility; exact interrupted native spring velocity is
not claimed. Animated lowering respects the captured spec.lensHangTime (0.22s):
it keeps the actual lifted state and defers alongside changes until the minimum
lifetime ends. Same-state four-argument relifts do not cancel an already queued
unlift, matching the native setter. The nonanimated two-argument convenience is
an explicit portable snap/cancellation path. Externally clocked Slider motion
keeps its own synchronized lift phase and bypasses this automatic clock.
Switch and TabBar geometry follow the shared alongside callback. See the
[short-tap investigation](../switch/tap-hang/README.md) for eight direct native
probes and four real-touch switch captures. Generic resting color remains clear unless provided by the caller,
rather than reproducing private `_controlForegroundColor` resolution.

Native `liftProgress` is a debouncing animation object, and `spec` is readonly.
Neither establishes an app-facing scalar-progress or arbitrary-spec setter.
Render graph, timers and private helper setters remain internal. Native
`liftedContainerView` relocates a portal rather than the actual view, and alternate
`overridePunchoutView` masking is not fully established. Neither is exposed as a
pointer-only stub; those advanced portal-placement APIs remain future work.

## Shared visual behavior

Automatic lenses and controls that synchronize their geometry clocks now use the
same glass-parameter retirement. Refraction, face, ring/shadow and highlight inputs
return from their captured lifted settings. Material distances retain the lifted
reference size as live SDF bounds shrink. The tab return uses its measured .4
critically damped material phase; its large-lens coefficients differ from the
slider's small-lens coefficients. A stationary native tab capture supplies 52
interior per-frame expectations across both themes; the largest parameter residual
is 0.00514 (normalized phase residual below 0.0003). No per-tab shadow workaround
is applied. [Native evidence](tab-shadow/README-native.md) includes full filter
inputs and real OS touch/video traces.

Resting shadow properties belong to the unclipped fading host, behind its clipped
fill, and follow its live bounds. This is shared lens configuration. UISlider uses
the public shadow object instead of editing the host's private CALayer. The tab
bar supplies its own resting blur material through the custom-background
constructor, chooses large style / hideSource / warping, and supplies alongside
layout through the common lift entry. Slider owns value projection and geometry;
TabBar owns selection travel and geometry. Shared material code owns the optical
and shadow retirement in both cases.

Native shadowPathIsBounds/punchoutShadow and opacityPair graph semantics remain
approximated by existing portable shadow/compositing behavior; this change does
not claim complete private compositor parity. The native tab's material return is
verified independently of those unresolved graph details. Automatic material return
terminates at the captured 480ms boundary, avoiding imperceptible analytic-tail
frames after the native face material has completed.

```cpp
auto resting = new_shared<UIView>();
resting->setBackgroundColor(UIColor::white);
auto lens = new_shared<LensView>(resting, LensView::Style::small);
auto shadow = std::make_shared<UIShadowProperties>();
shadow->setOpacity(.14f);
shadow->setRadius(5);
shadow->setOffset({0, 1});
lens->setRestingShadowProperties(shadow);
lens->setLifted(true, true, [lens] {
    lens->setBounds({0, 0, 57.35f, 37.2f});
}, [](bool, bool) {});
```

XML supports small/large styles, liftedContentMode, restingBorderColor and
restingShadowColor/Opacity/Radius/OffsetX/OffsetY, with finite/radius validation.
The existing source, trait, scale, event and animation checks remain required.

Final Simulator build and regression logs: [shared-verification/summary.json](shared-verification/summary.json).
