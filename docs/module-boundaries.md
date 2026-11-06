# UI modules and migration

The component kit and execution platform are independent choices. Applications
may put an Apple UIButton inside an NX navigation screen on Switch, or an
NXControl inside an Apple-styled screen on macOS. Supporting a new component
never requires selecting one global application theme or an OS-specific kit.
UISwitch is still unimplemented; this separation makes such composition possible
when its UIKit implementation lands.

| Module | Responsibilities | Dependencies |
| --- | --- | --- |
| UIKitCore | UIView/UIViewController, containment/lifecycle, generic presentation infrastructure, shared container capabilities, UIBarItem/UITabBarItem metadata, UIControl state/actions, UILabel/UIImageView/UITextView display, scrolling, traits, focus/input/gestures, Yoga, layers/animation, resources, neutral blur/backdrop shaders and platform backends | SDL, Skia, Yoga, resource/XML infrastructure |
| UIKit | Apple UINavigationBar/UINavigationController, UITabBarView/UITabBarController, UIButton styling, UIGlassEffect/UIGlassContainerEffect, BackdropEffectView/GlassContainerView/LensView hosts, native-calibrated glass/lens shaders and interaction, optional XML registration | UIKitCore |
| NXKit | NXControl, NXNavigationController, NXTabBarController, navigation items/widgets, separators, responder legends and controller icon providers | UIKitCore |
| Demo | Screen composition and comparison fixtures, including native tab comparison artwork/geometry and the Apple tabs integration screen | Both component kits |

UIKitCore includes shared semantic color and font resources; they do not install
kit-specific control feedback. Generic blur/tint stays in UIBlurView/CABlurLayer.
Custom shaders can back an ordinary UIView through CABackdropLayer; compilation,
automatic uniforms, validation, scale-aware filtering and rounded clipping live
in Core. Apple material recipes and animation parameters do not participate in
Core linking. BackdropEffectView remains a UIKit rendering extension because its
default is Apple regular glass and it manages that material's interaction and
appearance; the Apple UIVisualEffectView class hierarchy is not yet implemented.

## Build and includes

- Core: `UIKitCore` / `Xitrix::UIKitCore`, `<UIKitCore.h>`.
- Apple components: `XITRIXUIKit` / `Xitrix::UIKit`, `<UIKit.h>`.
- Nintendo components: `NXKit` / `Xitrix::NXKit`, `<NXKit.h>`.

`XITRIXUIKit` remains the actual UIKit library target. Naming that CMake target
UIKit makes Xcode confuse it with Apple’s native UIKit framework, so the public
consumer name is `Xitrix::UIKit`. The common
C++ namespace remains NXKit. Disable unused kits with UIKIT_BUILD_UIKIT or
UIKIT_BUILD_NXKIT. No platform backend implicitly selects a component kit.
NXKit.h no longer imports UIKit.h; mixed applications include and link both.

Core's application registration registers only Core views. Call
`NXKit::registerUIKitViews()` before inflating XML using UIButton or Apple effect
hosts. It is safe to call repeatedly. UIButton's style converter is local to
UIKit instead of being imported by UIView's shared XML helpers. Unknown classes
and unsupported attributes continue to fail through the existing XML diagnostics.

## Behavior changes and migration

UIControl now tracks input, actions and state without modifying application
opacity, transforms, shadows or elevation in its default visual hooks. UIButton
retains its former Apple focus/highlight feedback in UIKit; NXControl keeps its
own visual policy. Applications that styled a bare UIControl by relying on its
old automatic visuals should subclass its feedback hooks or use an appropriate
kit component.

`BackdropEffect::glass()` is removed from the neutral shader descriptor. Use
`UIGlassEffect(UIGlassEffect::Style::clear).backdropEffect()` when requesting the
same resolved Apple material, or install UIGlassEffect on a UIKit effect host.
The resolver is an explicit portable rendering extension, not an Apple header
claim. Core's custom BackdropEffect constructor and uniform APIs remain usable
without UIKit. `UIColor::resolvedColorWithTraitCollection()` follows Apple's
public API name and returns a constant color resolved against explicit traits.

## Container capability contract

Core defines UINavigationControllerInterface and UITabBarControllerInterface.
Concrete containers inherit UIViewController once and implement the applicable
interface. The interface method names follow Apple's stack and selection APIs;
chrome, appearance and kit-specific delegates are not part of the shared contract.
The interfaces have virtual destructors and do not own a second controller base.

UIViewController's navigationController() and tabBarController() search actual
parents, starting at the immediate parent, and return the nearest implementer.
They exclude self, do not load views, do not retain ancestors, and update
immediately after removal/reparenting. Presenting a controller does not make its
presenter an ancestor. A compact NX tab temporarily pushed into navigation
therefore sees that navigation container and no longer sees the tab controller
through actual containment. Logical tab membership is kept by NXTabBarController's
model, not by a hidden association in Core.

NX containers now establish child containment on configuration, before loading
views. Unloaded stack replacement/pop and tab replacement remove old parents as
well. This intentionally replaces the previous lazy containment so discovery is
usable during screen setup. Existing loaded transition and lifecycle handling
remains owned by each container.

Two C++ differences are deliberate: the getters return capability interfaces
rather than Apple's concrete classes; empty tab selection is optional instead
of NSNotFound, and selection setters return bool. Keep the eventual UIKit
concrete class names available for real implementations rather than abstract
placeholders. UINavigationController and UITabBarController are concrete UIKit components;
their styling and bars remain outside Core.

## Tab containers

Core defines `UIBarItem`, `UITabBarItem`, `UIViewController::tabBarItem()` and
`UITabBarControllerMode::{automatic,tabBar,tabSidebar}`. Items hold style-neutral
title, images, enabled state, tag and badge data. A default item follows its
controller's title until the item receives an explicit title. Replacing an item
or changing metadata updates the installed presentation live.

UIKit owns `UITabBarView` (also available as Apple's familiar `UITabBar` alias)
and `UITabBarController`. The bar has `items`/`setItems(items, animated)`,
`selectedItem`, a weak `UITabBarDelegate`, inherited `tintColor` and optional
`unselectedItemTintColor`. Standalone bars begin unselected; a null selected item
clears their lens. Assign distinct non-null items. User selection notifies the
delegate, including reselecting; programmatic selection is silent. Images honor
template rendering and `selectedImage` falls back to `image`. Badge values and
colors update live. Managed bar items/delegates belong to the controller;
configure them through child controllers and the controller delegate. Setting a
managed bar's selected item routes through controller selection.

The controller implements mutable `viewControllers`, index/controller selection,
`setViewControllers(controllers, animated)`, read-only `tabBar`, and weak
`UITabBarControllerDelegate` should/did-select callbacks. It establishes child
containment before views load, retains each child's state, loads only the selected
content, balances appearance, and reserves child safe-area space for its floating
bar without overwriting application additional insets. Delegate/lifecycle
mutations are revalidated or queued until the current update is stable. Invalid
indices return false; invalid hierarchy/item lists throw `std::invalid_argument`
before mutation. An empty list clears selection, containment and bar space.

`automatic` resolves to bottom `tabBar` for UITabBarController on every platform,
including wide screens. NXTabBarController resolves it to its existing
`tabSidebar` flow, including its compact list/navigation adaptation. Explicit
`tabBar` is supported only by UIKit; explicit `tabSidebar` only by NXKit. Each
rejects the other mode with `std::invalid_argument` while preserving its current
mode. UIKit sidebar is deferred. This policy deliberately differs from Apple's
platform-adaptive automatic mode.

The former Demo LensTabBarView is now a thin adapter over UITabBarView. It keeps
sample titles, fallback/native artwork and measured comparison geometry in Demo;
production UIKit owns glass, lens motion, gestures and item-driven layout. The
new Apple tabs Demo screen nests a real UITabBarController in NX containment and
exercises persistent content and badges. More/customization, modern UITab groups,
sidebar, appearance configuration, accessibility and native GPU parity remain
incomplete; these APIs are tracked as Partial.

The existing NX compact adapter still uses NXNavigationController's private
stack observation for its Nintendo-specific flow. Ordinary content can use the
new shared capability today; making that adaptive flow interoperate with a
custom navigation implementation needs an explicit transition/observation
contract, rather than guessing from push/pop calls or sharing a delegate slot.

## Verification

Regression sources now live under the module they exercise. Mixed Apple/Nintendo
material and container tests live under UIKit/tests and link both kits. Core's
module-boundary test uses a custom navigation implementation and renders a
custom shader without either component library. The existing raster harness can
run it alone with NXKIT_RASTER_CORE_ONLY=ON. Raster checks do not establish GPU
Metal/Dawn fidelity or physical Switch behavior.

The retained mac-arm64-angle raster snapshot uses debug Skia headers. The
verified CMake fixture uses NXKIT_SKIA_DEBUG=ON and matching archived headers
(NXKIT_SKIA_RASTER_HEADERS); do not combine that archive with current Graphite
headers. The small standalone build-raster.sh adapter accepts
SKIA_RASTER_DEBUG=ON and SKIA_RASTER_HEADERS for the same reason. Legacy
comparison-script --core-archive/--core-library flags now replace the UIKit
material archive; the neutral UIKitCore archive remains linked separately.

Public naming references: Apple's [tab bar](https://developer.apple.com/documentation/uikit/uitabbar),
[tab items](https://developer.apple.com/documentation/uikit/uitabbaritem),
[tab controller](https://developer.apple.com/documentation/uikit/uitabbarcontroller),
[navigation controller operations](https://developer.apple.com/documentation/uikit/uinavigationcontroller),
[ancestor navigation lookup](https://developer.apple.com/documentation/uikit/uiviewcontroller/navigationcontroller),
[tab controller modes](https://developer.apple.com/documentation/uikit/uitabbarcontroller/mode-swift.enum),
and [explicit dynamic-color resolution](https://developer.apple.com/documentation/uikit/uicolor/resolvedcolor(with:)).

Verified on 2026-11-02:

- All 30 raster regression checks passed with the matching debug archive/header configuration.
- All nine Core-only checks passed with neither component kit configured.
- Core, UIKit and NXKit umbrella headers and the new public headers compiled individually using public include directories.
- The iOS arm64 Debug Demo built and linked with signing disabled.
- The Nintendo Switch Demo built and packaged ThorVGApp.nro.
- The standalone material-comparison adapter built with the relocated sources.
- Matching Apple Metal/Graphite archives are now rebuilt from current Skia; the
  normal macOS Demo and unsigned iOS Xcode Debug Demo both compile/link. Fresh
  Core/navigation/tab regressions pass 3/3 with those headers and archives. See
  [the Apple build manifest](skia-upgrade/apple-build.json).

Platform builds verify compilation/linking and packaging, not device execution.

## Navigation components — 2026-11-03

Core now owns `UINavigationItem`, `UIBarButtonItem` and lazy
`UIViewController::navigationItem()`. UIKit owns the concrete `UINavigationBar`
and `UINavigationController`. The existing NX navigation types remain independent.
The full-screen iOS Demo is built from UIKit tab and navigation containers with
its original list content as navigation roots. Native comparison evidence and
remaining compatibility gaps are described in [the navigation audit](navigation/README.md).
