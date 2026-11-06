# Private liquid-lens configuration audit

This is a build-specific private UIKit investigation, not a recovered Apple header. The target is iOS27.0 Simulator UIKitCore, SHA256 `5cefe8cfb213b0dfcb378feb86ddd9062fda37fb4f331b3ceb782070a20292a4`. No private UIKit code is linked into NXKit. The portable API can share useful behavior without exposing Apple's internal render graph.

## Evidence and scope

- Runtime class-local Objective-C inventory: [native-interface.json](native-interface.json), **62 methods / 28 properties**, `_UILiquidLensView : UIView`. Extracted from the retained 144-profile native capture, original runtime evidence `ev_00804c3e2e408ea042163c6889f41fa905db7c931b6708550354b3b1cb0865b7`. This does not enumerate inherited or Swift-only members.
- Focused shipped-code inspection: [rea-static.json](rea-static.json), `ev_d98c3743222729a0ec2bb9890ffc8b6693768e48b5b0805553be1a3b812d9568`. LLDB runs against the extracted binary; it does not execute UIKit. The capture exits0; REA reports overall truncation, while the structured findings and hash manifest are complete in its terminal frames. Exact focused ranges are retained in [disassembly](disassembly).
- Fresh authored app-space calls: [callbacks.json](callbacks.json), [rea-callbacks.json](rea-callbacks.json), `ev_48766809c029a1dc7524ed2fc7a9384a52b4a1f29b4fb6e77df3a286c8d93f32`. iOS27.0 phone Simulator, scale3. The case calls private APIs directly; it does not test touch routing. REA exits0 and retains the complete JSON terminal output, but reports overall capture truncation. The first helper attempt failed scene-lifecycle enforcement before its callback schedule; it provides no callback evidence and is discarded.

## Useful configuration surface

| Native member | Established behavior | Portable recommendation |
|---|---|---|
| `initWithRestingBackground:` | Accepts a custom UIView; default nil constructs the internal resting host. `hasCustomRestingBackground` and `restingBackgroundView` are readonly. | Constructor accepting an optional custom resting view; readonly access to the host. |
| `style` / `setStyle:` | NSInteger; observed controls use0 for slider/switch and1 for tab selection. | Existing regular/selection names can have small/large C++ aliases. Native enum spellings are unknown. |
| `restingBackgroundColor` | Strong nullable UIColor; setter updates resting host. With a custom resting view the update helper does nothing; otherwise nil falls back to `_controlForegroundColor`. | Usable nullable override, preserving custom-view ownership. |
| `restingBorderColor` | Strong nullable UIColor; setter updates both root and glass layers. Their width is3pt unlifted,0 lifted. | Shared rest border configuration, not a slider-only decoration. |
| `restingShadowProperties` | Strong nullable `_UIShadowProperties`; details below. | Shared shadow value object and common lens setter. |
| `warpsContentBelow` | BOOL; fresh `initWithRestingBackground:nil` default **false**. Actual switch/tab callers settrue; slider remainsfalse. | Configurable common lens policy; do not infer generic constructor default from one control's configured state. |
| `liftedContentView` | Weak UIView; setter updates the source portal's `sourceView`. | Existing weak selected-content source can be the portable equivalent. |
| `liftedContentMode` | NSInteger; fresh default0. Setter sets source portal `hidesSourceView=(mode==1)` and refreshes placement without animation. Actual tab1, slider/switch0. | C++ names `preserveSource=0`, `hideSource=1` describe observed semantics; names are portable, not recovered native enum names. |
| `liftedContainerView` | Weak UIView; when an attached lift portal exists, changing the container relocates that portal into the new container, falling back to the window. Actual lens view is not relocated. | Expose only with usable portal placement/coordinate behavior. A weak pointer alone would be incomplete. |
| `overridePunchoutView` | Weak UIView; setter invokes `updatePunchoutView`. | Keep internal until alternate-source punchout behavior is implemented and tested. Setter existence alone does not establish full clipping semantics. |

Fresh nil constructor also reports style0, no custom background, and nil resting background/border/shadow overrides. These are constructor observations; later control-specific setup deliberately differs.

## Shadow value object

The lens type is **`_UIShadowProperties`**, distinct from another shipped class named `UIShadowProperties` without the leading underscore. Its exact four native properties are `opacity` (CGFloat/double, default0), strong `color` (UIColor, default black), `offset` (CGSize, default zero), and `radius` (CGFloat/double, default0).

`propertiesFromView:` reads the view's `_outermostLayer`; `applyToView:` writes its shadow color, offset, radius and opacity. `copyWithZone:` copies all four fields. `scaleBy:` mutates only radius and both offset components. `updatedShadowWithIntensity:` returns a copy whose opacity is multiplied by intensity clamped to0…1.

The lens's own shadow setter stores the object and configures **restingBackground.layer**, enabling `shadowPathIsBounds` and `punchoutShadow`; it writes all four fields. Nil resets radius, offset and opacity to0. This setter does not itself apply a lift-progress multiplier. Native punchout flags should be treated as render semantics; a portable exterior-only shadow approximation must be documented, not claimed to reproduce those private flags exactly.

## Lift callbacks and lifecycle

The actual declared entry is `setLifted:animated:alongsideAnimations:completion:`. No two-argument `setLifted:animated:` appears in this class-local metadata. `lifted` is readonly; `_lifted`, `actuallySetLifted:...`, timers and graph getters are internal implementation mechanisms.

The method encoding `v40@0:8B16B20@?24@?32` alone does not reveal nested block signatures. Shipped bridge instructions establish **alongside `void()`** and **completion `void(BOOL,BOOL)`**. In particular the completion bridge at `0x1c50d1fa4` transfers two masked booleans to the Objective-C block, and normal grouped completion closures explicitly supply `(YES,NO)`. Older research fixtures declaring a zero-argument completion were underdeclared; they cannot establish the completion ABI.

The fresh case establishes:

- Nonanimated up/down calls run alongside, then completion `(YES,NO)`, synchronously before returning.
- Same-state calls, animated or not, run alongside synchronously and return **without calling completion**.
- Animated normal and interrupted/retargeted cases in this bounded run eventually deliver `(YES,NO)`. Several callbacks are collected until the shared group settles. The final same-state down callback is absent, as expected.
- An unlift requested during the minimum hang interval returns while `lifted` is stilltrue; its alongside callback runs later. A same-state relift in this case does not cancel that pending unlift.

The boolean semantic names are **unresolved**: these normal/retarget samples never vary the pair, and static bridge code establishes types, not names. Do not call the second bit `interrupted` or claim generic UIView completion semantics without more evidence. A portable callback with a simpler documented contract is an intentional C++ extension; exposing anonymous native bits would not make a useful API. The observed delayed-unlift behavior need not be preserved if NXKit deliberately provides safer cancellation, but that divergence must be explicit.

## Keep internal

The readonly `spec` getter returns `_UILiquidLensViewVariantSpec` subclasses. Existing captures establish variant defaults and material/spring settings, not a `setSpec:` method. Do not add a public spec setter from serialized archive keys. `liftProgress` is a `_UIDebouncingAnimatableFloat` implementation object, not evidence for a public external-progress API. Keep `liftPortal`, `glass`, `belowGlassWarpBackdrop`, `contentWrapper`, `_removeAllAnimations:`, timers, group completion and progress-driven render hooks internal. Separate caller motion geometry from shared lens material lifecycle without exposing diagnostics as application configuration.

The API inventory, constructor/default policy, shadow schema, source mode setter and callback ABI questions are answered for this build. Callback flag names, full alternate punchout semantics, and native enum spellings remain explicitly unresolved. No physical iPhone action was performed by this audit.

Reproduce static checks with `/usr/bin/python3 docs/liquid-glass-lens/api-audit/audit.py`. The authored runtime helper is [capture-callbacks.py](capture-callbacks.py); pass a booted Simulator UUID and scratch output directory. Its source is [CallbackReference.m](CallbackReference.m).
