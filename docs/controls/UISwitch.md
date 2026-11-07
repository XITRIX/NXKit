# UISwitch

`UISwitch` is an optional UIKit component built on `UIControl` and the shared
`LensView`. Include `<UISwitch.h>` or `<UIKit.h>` and link `Xitrix::UIKit`.

The public surface follows Apple's `UISwitch.h`: `isOn`, `setOn(on, animated)`,
nullable `onTintColor` and `thumbTintColor`, `onImage` and `offImage`, concrete
`style`, and requested `preferredStyle`. Construction optionally accepts a
`UIAction` registered for `valueChanged`. Programmatic setters and explicit
event dispatch never toggle the control implicitly or send a change event.

Touch input supports taps and dragging the thumb, cancellation, disabled state,
exclusive tracking inside scroll views, and interruption of an animated state
change. Console Select and the portable `accessibilityActivate` hook toggle the
state. Native accessibility bridging remains a Core limitation.

Like the observed modern native provider, switch gestures leave inherited
`isTracking` and `isHighlighted` false. A tap sends `valueChanged` before
`touchUpInside`, without `touchDown`; dragging commits `valueChanged` on release
and does not send `touchUpInside`. A drag that returns to its original side
does not send a change. Programmatic animated changes move the resting knob
without lifting it.

The measured iOS 27 provider enforces a 63 × 28 frame at initialization and
subsequent `setFrame` calls, while its intrinsic/fitting size reports 61 × 28.
The resting knob is 37 × 24, with 22 points of travel and two-point insets.
Its held bounds become 58 × 38⅓. These are provider metrics, not assumptions
about a screen or enclosing row. Direct `setBounds` remains supported.

The modern sliding provider retains `onImage` and `offImage` without displaying
them, matching the observed UIKit provider. On non-Catalyst platforms,
`preferredStyle=checkbox` resolves to `style=sliding`. The Catalyst-only `title`
property and AppKit checkbox presentation are not included.

The knob uses shared lens material, refraction, resting fill and shadow hosting,
and lens state transitions; there is no switch-specific optics shader.
The small-lens lift and lowering bounds and the separate critically damped
programmatic position response are checked against retained native traces.
The fluid well artwork, gesture thresholds and velocity-dependent release,
arbitrary-scale geometry and platform accessibility remain incomplete;
this component is tracked as Partial. Run
`python3 Demo/SwitchComparison/generate-motion.py --check` to verify that the
regression data still matches the immutable captured traces.

`registerUIKitViews()` enables `<UISwitch on="true" preferredStyle="sliding"/>`.
The `onTintColor` and `thumbTintColor` attributes accept hexadecimal colors or
`nil`; image attributes accept resource names or `nil`. Invalid attributes fail visibly.

Regression target: `NXKitSwitchTests`. The Glass appearance Demo contains an
interactive switch and an animated programmatic-toggle button.
The iOS fullscreen list also opens **System UI elements → Switch**, with live
On, Enabled, animation, tint, preferred-style, background and event settings.
Its sibling Slider screen exercises range, value, continuous events, tints,
style, ticks and endpoint images.

References: [Apple UISwitch documentation](https://developer.apple.com/documentation/uikit/uiswitch)
and the installed iPhoneSimulator 27.1 SDK's public `UISwitch.h`. Runtime
measurement provenance and native captures are retained under `docs/switch/`.
