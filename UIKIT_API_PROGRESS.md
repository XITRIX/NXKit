# UIKit core API progress

This document tracks NXKit against the portable, application-facing core of Apple's UIKit. It is both a compatibility audit and an implementation roadmap.

- Last audited: **2026-08-02**
- NXKit revision: **working tree after `d6e6c6b9aa44ae88e4b9a49a15ebc76d4039a8da`**
- UIKit-cross-platform reference: **[`ed2c290`](https://github.com/flowkey/UIKit-cross-platform/commit/ed2c290025f2dfd714ed945153069ca54e55a23e)**

## How to maintain this document

Any patch that changes a UIKit-facing public API or its behavior must update this document in the same patch. Follow the maintenance contract in `AGENTS.md`.

Statuses describe behavior, not declarations:

- **Usable** — the listed core behavior is available and has no known application-blocking defect; this does not imply every Apple overload or convenience API exists.
- **Partial** — a meaningful subset works, but important behavior, fidelity, or verification is missing.
- **Stub/unsafe** — the public surface exists but is empty, misleading, or has a known correctness/lifetime blocker.
- **Missing** — no equivalent implementation exists.
- **Alternative** — NXKit intentionally solves the need with a different, non-source-compatible API.
- **Out of scope** — intentionally excluded from the current portable-core target.

Priorities:

- **P0** — required for a dependable small application.
- **P1** — required by most production applications.
- **P2** — common advanced application functionality.
- **P3** — valuable breadth after the core is dependable.

Apple's [views and controls](https://developer.apple.com/documentation/uikit/views-and-controls), [view controllers](https://developer.apple.com/documentation/uikit/view-controllers), [UIScrollView](https://developer.apple.com/documentation/uikit/uiscrollview), and [CALayer](https://developer.apple.com/documentation/quartzcore/calayer) documentation define the target. UIKit-cross-platform is a secondary behavioral and test reference, not the specification.

## P0 — dependable application foundation

| UIKit API or capability | Status | NXKit evidence and remaining work |
| --- | --- | --- |
| `CGFloat`, `CGPoint`, `CGSize`, `CGRect` | Partial | `NXFloat`, `NXPoint`, `NXSize`, and `NXRect` provide common geometry and transforms. Names and portions of Core Graphics behavior differ; add edge-case tests for null/empty/infinite geometry, standardization, intersection, containment, and pixel alignment. |
| `UIEdgeInsets` | Partial | Basic value type and arithmetic exist. Verify equality, inset operations, direction-aware edges, and UIKit-compatible defaults. |
| `CGAffineTransform`, `CATransform3D` | Partial | `NXAffineTransform` and `NXTransform3D` support common transforms and animation. Expand conversion, concatenation, inversion, decomposition, and numerical tests. |
| Safe object ownership | Stub/unsafe | UIKit-style objects depend on the custom `new_shared` ownership path. Stack allocation or `std::make_shared` can create competing owners. Replace `SharedBase.hpp` with standard ownership before declaring the object model dependable. |
| Main-thread UI confinement | Partial | A main `DispatchQueue` exists, but UI objects have no enforcement or assertions comparable to `@MainActor`. Add a UI-thread contract and debug checks. |
| `UIApplicationMain`, `UIApplication`, `UIApplicationDelegate` | Partial | Launch, quit, foreground/background callbacks, SDL event handling, and one global application exist. The build now exports `SK_RELEASE` to match the checked-in release Skia archives, preventing incompatible inline `SkRefCnt` disposal semantics at shutdown. Apple and Switch teardown also clear promoted-image caches while the Graphite recorder is valid and drain GPU resources before destroying the context; Switch restores nxlink-redirected descriptors and terminates EGL for same-process homebrew-launcher handoff. Switch hardware verification is pending. Complete lifecycle ordering, activation semantics, error handling, and reproducible tests. |
| `UIWindow` and root controller | Partial | Root-controller installation, key visibility, event dispatch, safe area, traits, and focus exist. Clarify key-window semantics, multiple-window support without adopting `UIScene`, resizing, and teardown. |
| `UIResponder` and responder chain | Partial | Touch and press forwarding exists. Add `canPerformAction`-style routing where portable, first-responder management, loop protection, and chain tests. |
| `UIEvent`, `UITouch`, presses and keys | Partial | Touch and gamepad/press events exist. The SDL3 Switch backend now preserves finger identity across polls and emits balanced down/motion/up events; the full Switch Demo `.nro` build verifies backend integration, but hardware touch behavior still needs manual confirmation. Complete timestamps, phases, multi-touch identity, cancellation, modifier/key semantics, mouse input, and deterministic tests. |
| `UIView` geometry and hierarchy | Partial | Frame/bounds/center, subviews, superview, hit testing, conversion, alpha, hidden state, backgrounds, alpha masks, tint and content modes exist. Mask views now participate in display/layout and trait/tint propagation, and descendant traversal correctly advances through each ancestor. Fix index validation, remaining hierarchy-cycle checks, invalidation, conversion through transforms, and insertion parity. |
| `UIView` display and invalidation | Stub/unsafe | Drawing exists, but several layer/view setters do not mark the global tree dirty. Make every visual mutation schedule the required display/render pass before treating this as usable. |
| `UIView` layout lifecycle | Partial | `setNeedsLayout`, `layoutIfNeeded`, margins, safe areas, sizing, and controller layout callbacks exist. Verify callback ordering, propagation, hidden views, re-entrant layout, and intrinsic sizing invalidation. |
| Auto Layout (`NSLayoutConstraint`, anchors, guides) | Alternative | Yoga flex layout is the current portable layout system. Document the intentional incompatibility. Add `UILayoutGuide`-like safe/readable guides only if they improve portable layout; do not claim Auto Layout compatibility. |
| `CALayer` geometry, hierarchy, and visual properties | Partial | Bounds, position, anchor point, affine/3D transform, z-order, opacity, background, rounded corners, borders, shadows, contents, hierarchy, alpha masks, and presentation copies exist. Fix incomplete insertion, invalidation, contents rectangles/centers, coordinate conversion, and remaining safe-hierarchy mutation gaps. |
| Core Animation actions and transactions | Partial | `CAAction`, `CABasicAnimation`, spring animation, timing functions, model/presentation layers, implicit actions, and basic transactions exist. Add robust begin/commit nesting, completion semantics, repeat/autoreverse, fill behavior, additive/keyframe/group animation where useful, interruption, and tests. |
| `UIView` animation APIs | Partial | Duration, delay, options, spring parameters, completion, and without-animation blocks exist. Verify lifecycle timing, curve/options behavior, nested blocks, interruption, completion delivery, and interaction gating. |
| `CADisplayLink` | Partial | A callback is registered with the main dispatch loop and can be invalidated. Add timestamp/duration, preferred rate/range, pause, safe lifetime/thread behavior, frame-drop behavior, and tests. |
| `UIViewController` view loading and lifecycle | Partial | Lazy view loading, appearance callbacks, title, safe-area callbacks, traits, and responder participation exist. Add strict appearance ordering, repeated presentation tests, memory/unload behavior, and transition cancellation semantics. |
| View-controller containment | Partial | `addChild`, parent/children, move callbacks, and removal exist. Validate ordering and ensure containers forward appearance transitions correctly. |
| Modal `present` / `dismiss` | Partial | Full-screen-style presentation and dismissal exist. Complete ownership semantics, nested presentation, interruption, transition completion timing, and presentation styles that make sense cross-platform. |
| `UIColor` and dynamic colors | Partial | RGBA colors, many semantic colors, alpha changes, tint and dark/light dynamic providers exist. Add color-space-independent semantics, stable resolution rules, more system colors, and tests. Pattern colors are not core priority. |
| Trait environment and `UITraitCollection` | Partial | Interface style and display scale propagate. Add layout direction, preferred content size, accessibility contrast, interface idiom where portable, and explicit trait-change registration/ordering. |
| `UIImage` / `CGImage` | Partial | Images load from path, resources, and data; scale and template rendering exist, and raster images are promoted through a bounded Graphite image-provider cache on every supported backend. Add orientation-independent image metadata, resizing/cropping, cap insets or a portable equivalent, decoding errors, and cache/lifetime tests. |
| `UIImageView` | Partial | Basic image display, tint/template mode, content mode, and fitting exist. Add highlighted/animated images only after core invalidation and state behavior are dependable. |
| `UIFont` and font descriptors | Missing | Labels accept only numeric size and weight. Add a portable `UIFont` facade for family, size, weight, traits, metrics, fallback, and custom font registration. |
| `UILabel` | Partial | SkParagraph/ICU shaping, text, color, size, weight, alignment, measurement, and rendering exist; default-family resolution also supports platform custom-data font managers. Expose and implement line count, wrapping, truncation, attributed text, baseline behavior, font objects, intrinsic size, and accessibility text. Remove debug logging. |
| `UIControl` target/action and state | Stub/unsafe | Enabled/selected/highlighted bits and one `primaryAction` exist. Implement combinable UIKit-like states, target/action or event-scoped actions, event dispatch, disabled behavior, tracking/cancellation, and state-change invalidation. |
| `UIButton` | Partial | A title label, image view, four visual styles, Yoga layout, focus and primary action exist. Add per-state title/image/color, content alignment/insets, configuration/state updates, disabled behavior, touch cancellation, and intrinsic sizing. |
| `UIGestureRecognizer` base | Partial | State, delegate, enablement, touch/press routing, and simultaneous-recognition cancellation exist. Complete dependency/failure relationships, cancellation policies, touch filtering, reset rules, and delegate coverage. |
| `UITapGestureRecognizer` | Partial | Single-touch tap/press recognition exists. Add tap count, touch count, movement/duration thresholds, cancellation behavior, and tests. |
| `UIPanGestureRecognizer` | Partial | Translation and velocity exist. Add min/max touch counts, robust multi-touch centroid behavior, coordinate-conversion tests, and deterministic velocity sampling. |
| `UIScrollView` basic scrolling | Partial | Offset, insets, bouncing, safe-area adjustment, pan, deceleration, and spring-back exist. `contentSize` incorrectly derives from only the first subview and has no setter; fix this first. |
| `UIScrollView` indicators and delegate | Stub/unsafe | Indicator methods are empty, begin-dragging is delivered repeatedly, and end-dragging is not delivered. Implement the public behavior or remove/mark the surface until it works. |
| Editable text: `UITextInput`, `UITextField`, basic `UITextView` | Missing | Required for forms, search, login, and settings. Build a portable text-input client, SDL IME bridge, selection/caret model, clipboard integration, composition/marked text, secure entry, and keyboard navigation before styling breadth. |
| Core accessibility element semantics | Missing | Add accessible flag, label, value, hint, traits/role, enabled/selected state, frame, hidden state, grouping, ordering, actions, focus, and a platform bridge. Accessibility is part of the core definition of done, not optional polish. See Apple's [Accessibility for UIKit](https://developer.apple.com/documentation/uikit/accessibility-for-uikit). |
| Automated core tests | Missing | Add a fast test target covering ownership, geometry, view/layer hierarchy, conversion, layout, lifecycle, controls, gestures, scrolling, traits, and animation. Port applicable behavioral tests from UIKit-cross-platform. |

## P1 — common production application structure

| UIKit API or capability | Status | NXKit evidence and remaining work |
| --- | --- | --- |
| `UINavigationController` | Missing | `NXNavigationController` is fixed console/demo chrome, lacks normal push/pop behavior, and contains product-specific assets/text. Implement a separate UIKit-compatible stack container in `UIKitCore`. |
| `UINavigationBar`, `UINavigationItem`, bar-button items | Missing | Build data-driven navigation items, title/back behavior, left/right actions, safe-area layout, appearance, and focus/touch input. |
| `UITabBarController`, `UITabBar`, tab items | Missing | `NXTabBarController` is a 410-point sidebar, not a UIKit tab controller. Implement selection, child containment, item state, customizable placement, and adaptive bottom/sidebar layout without hard-coded product styling. |
| `UIScreen`-like display information | Missing | Expose logical bounds, pixel scale, refresh rate/range, and display identity through a portable facade rather than requiring application code to access `SkiaCtx`. |
| `IndexPath` | Partial | A two-component section/item value exists. Add const access, equality/order/hash, arbitrary components if needed, and tests before using it as the stable public list index type. |
| `UITableView`, cells, data source and delegate | Missing | High-value foundation for settings, lists, menus, and feeds. Start with vertical reusable cells, headers/footers, selection, scrolling, insert/delete/reload, stable reuse, focus, accessibility, and keyboard/gamepad navigation. |
| `UICollectionView`, cells and basic layouts | Missing | Implement after table/reuse primitives. Begin with a flow/grid layout, reusable cells/supplementaries, selection, batch updates, and accessibility. |
| Reuse and diffable data model | Missing | Define stable identifiers and snapshot/diff behavior shared by table and collection views. A simpler C++ API is acceptable if lifecycle and update semantics are documented. |
| `UIStackView` | Alternative | Yoga covers most stack-layout needs, but there is no UIKit-compatible arranged-subview API. Add only if UIKit familiarity materially improves common layout code. |
| `UITextView` display/editing subset | Missing | Build on the P0 text-input model with multiline layout, scrolling, selection, attributed runs, links, and accessibility. It may reuse `UIScrollView` only after scroll behavior is corrected. |
| `UISwitch` | Missing | Implement value state, action events, disabled/highlighted behavior, animation, focus, and accessibility. |
| `UISlider` | Missing | Implement range/value, continuous events, touch/gamepad/keyboard interaction, visuals, and accessibility increment/decrement. |
| `UISegmentedControl` | Missing | Implement items, selection, momentary mode if needed, control events, intrinsic sizing, focus, and accessibility. |
| `UIActivityIndicatorView` and `UIProgressView` | Missing | Provide determinate/indeterminate progress primitives with reduced-motion-aware animation and accessibility values. |
| `UIAlertController` / actions | Missing | Implement portable alert and action-sheet semantics, focus trapping, keyboard/gamepad selection, cancellation, safe-area sizing, and accessibility. Avoid platform-specific iOS presentation chrome. |
| `UISearchBar` or search-field composition | Missing | Implement after `UITextField`; a composed control is sufficient if behavior, clear/cancel actions, focus, and accessibility are familiar. |
| `UIVisualEffectView` / blur | Partial | `UIBlurView` and `CABlurLayer` provide real Skia background blur and tint. Align naming, invalidation, radius/style behavior, clipping, performance, and fallback rendering. |
| `CAGradientLayer` | Missing | Add linear gradients first using Skia, including colors, locations, start/end points, scale, animation, and mask compatibility. |
| `CAShapeLayer`, `CGPath`, `UIBezierPath` core | Missing | Needed for icons, progress rings, clipping, custom controls, and vector drawing. Start with move/line/curve/close, fill/stroke, line styles, bounds, hit testing, and animation-safe ownership. |
| Layer masking | Partial | `CALayer::mask` now uses destination-in alpha compositing, honors mask geometry and presentation state, preserves the backdrop required by blur layers, and invalidates with its masked hierarchy. Add focused pixel tests for partial alpha, transforms, animated masks, nested masks, nonzero bounds origins, and transparent backbuffers; evaluate luminance masks only as an explicit extension because UIKit's contract is alpha-based. |
| Focus engine | Partial | Directional focus, focus environments, coordination, and gamepad input exist. Replace hierarchy-order-only movement with geometric candidate scoring, disabled filtering, focus guides or overrides, and accessibility integration. |
| Keyboard navigation and shortcuts | Partial | Low-level key/press types exist, but standard activation, tab navigation, directional navigation, escape/back, and command routing are incomplete. |
| Layout direction and localization support | Missing | Add left-to-right/right-to-left traits, leading/trailing layout concepts, natural alignment correctness, string/resource localization hooks, and mirrored control behavior. |
| Dynamic type / scalable metrics | Missing | Add preferred content-size categories or a platform-neutral scale trait, font metrics, relayout, and accessibility-size support. |
| Clipboard / `UIPasteboard`-like service | Missing | Provide portable text and image copy/paste through SDL/platform backends, required by editable text and common application workflows. |
| Application resources / bundles | Partial | `NXData`, image resource loading, and libromfs exist. Define a stable bundle/resource abstraction, errors, locale variants, scale variants, and testable lookup rules. |
| `UINib` / declarative view loading | Alternative | A custom TinyXML2 view registry and XML inflater exist. It is not compatible with Apple nibs, storyboards, outlets, actions, or archives. Keep the dialect portable and document its schema; do not imply Interface Builder compatibility. |
| Notifications / observation utility | Missing | A small safe notification/observer primitive is useful for UIKit-like lifecycle and app state. Prefer typed C++ callbacks/tokens and deterministic removal over copying Objective-C selector APIs. |

## P2 — common advanced functionality

| UIKit API or capability | Status | NXKit evidence and remaining work |
| --- | --- | --- |
| Pinch, long-press, swipe and rotation gestures | Missing | Implement shared recognizer timing/state infrastructure first, then individual recognizers with touch-count and cancellation tests. |
| Scroll-view zooming | Missing | Add zoom scale/range, content centering, delegate hooks, gesture arbitration, and accessibility after basic scrolling is dependable. |
| `UIPickerView` and a portable date-picker model | Missing | Prioritize general picker mechanics; keep platform-specific calendar appearance separate from the data/control contract. |
| `UIPageControl` | Missing | Add page count/current page, value events, focus/touch input, intrinsic sizing, and accessibility. |
| `UIToolbar` and general bar items | Missing | Reuse navigation item/action and appearance primitives; avoid iOS-only placement assumptions. |
| `UISplitViewController` | Missing | Implement adaptive two/three-column containment only after navigation and trait systems are stable. |
| `UIPageViewController` | Missing | Implement after container lifecycle and scroll/gesture arbitration are tested. |
| Menus and contextual actions | Missing | Provide portable action models, nested menus, enabled/selected state, mouse/touch/gamepad presentation, keyboard shortcuts, and accessibility. |
| Pointer, hover and mouse interaction | Missing | SDL mouse input exists at a low level. Add hover events, cursor/interaction state, control highlighting, and consistent hit testing. |
| Drag and drop inside the application | Missing | Start with local typed payloads and reordering. Native inter-application drag and drop can remain platform optional. |
| Multiple windows | Missing | Support multiple `UIWindow` roots and event routing without requiring Apple's `UIScene` API. |
| Custom modal transitions and presentations | Missing | Add transition contexts, cancellation, presentation containers, dimming, and interactive progress after standard modal behavior is stable. |
| Interruptible view animation / property animator | Missing | Current animations cover common implicit/basic/spring cases. Add pause, continue, reverse, fraction-complete, interruption, and completion-position semantics. |
| Advanced attributed text and text attachments | Missing | Build on `UIFont`, path/drawing, and editable text. Include paragraph styles, runs, links, attachments, selection, and accessibility. |
| Image animation and resizable images | Missing | Add only after image lifetime, caching, and invalidation are well tested. |
| State restoration | Missing | Use portable identifiers and serializable controller/view state; exclude Apple's process- and scene-specific mechanics. |

## P3 — later portable breadth

| UIKit API or capability | Status | Notes |
| --- | --- | --- |
| Advanced layer types (`CAReplicatorLayer`, tiled rendering, emitter-like effects) | Missing | Add based on demonstrated application demand and Skia feasibility. |
| Appearance proxies and broad theming | Missing | Prefer explicit theme/trait propagation first; add appearance-style defaults only with deterministic scope and invalidation. |
| Content configuration APIs | Missing | Useful once reusable list cells and control state configuration exist. |
| Rich text layout manager compatibility | Missing | Consider a smaller portable API unless source familiarity justifies UIKit/TextKit-shaped types. |
| Native inter-application drag/drop and platform services | Missing | Platform capability, not a requirement for the portable core. |

## Intentional divergences to preserve or decide explicitly

These are not silently “complete.” They must remain documented until a deliberate compatibility decision changes them.

| Area | Current divergence |
| --- | --- |
| Language/API shape | C++ setters/getters, callbacks, smart pointers, and value types replace Objective-C/Swift properties, selectors, ARC, and protocols. |
| Geometry names | NXKit currently uses `NXPoint`, `NXSize`, `NXRect`, `NXAffineTransform`, and `NXTransform3D` rather than Core Graphics names. |
| Layout | Yoga flex layout replaces Auto Layout constraints and anchors. |
| Interface files | NXKit's TinyXML2 format is a custom declarative layout dialect, not an Apple nib/storyboard parser. |
| Rendering | Views are custom-rendered through Skia rather than native platform widgets. Native platform look, accessibility, text input, and system integration must therefore be implemented explicitly. |
| Extension components | `NXNavigationController`, `NXTabBarController`, and `NXControl` are opinionated console/demo components and do not count as UIKit API coverage. |

## Currently out of scope

Do not add these to the active completeness tables unless the project scope changes:

- `UIScene`, scene sessions, iOS process restoration, and iOS-specific application-shell lifecycle
- status-bar, home-indicator, device-orientation, and iPhone/iPad idiom policy APIs
- Apple Pencil, 3D Touch, and device-specific sensor interactions
- UIKit wrappers for camera, photo library, contacts, mail, messages, telephony, StoreKit, or authentication services
- printing, AirPrint, document browser/picker, and iOS share-sheet controllers
- CarPlay, watchOS, visionOS-only, or extension-host APIs
- WebKit, MapKit, AVKit, SpriteKit, and other separate Apple frameworks
- haptic APIs until a portable feedback abstraction becomes a project goal

Safe areas, keyboard/gamepad focus, mouse input, multiple windows, accessibility, localization, and clipboard remain in scope because they are broadly useful across supported and future platforms.

## Near-term milestone order

1. Replace unsafe shared ownership and add the initial automated test target.
2. Make view/layer invalidation, masking, hierarchy operations, and lifecycle ordering dependable.
3. Finish `UIControl`, `UIButton`, `UILabel`/`UIFont`, editable text/IME, and `UIScrollView` core behavior.
4. Add accessibility semantics and a first platform bridge.
5. Implement standard navigation/tab containers and reusable table/list infrastructure.
6. Add gradients, paths/shapes, common controls, alerts, and localization/dynamic type.
7. Add Android, Windows, or Linux backends only with shared conformance tests so platform expansion does not fork behavior.
