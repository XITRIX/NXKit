# Portable control events and slider

`UIControl` belongs to UIKitCore. It owns option-set state, event registrations,
weak target/action dispatch, touch tracking and focus/input routing. It applies
no component-kit styling. `UISlider` belongs to UIKit and builds its Apple
appearance and LiquidLens feedback on those hooks.

```cpp
auto slider = new_shared<UISlider>();
slider->setMinimumValue(0);
slider->setMaximumValue(1);
slider->setValue(.5f); // Silent, including animated setters.
slider->addAction(UIAction("Changed", "volume", [](const UIAction& action) {
    auto slider = std::dynamic_pointer_cast<UISlider>(action.sender());
    // Read slider->value().
}), UIControlEvent::valueChanged);
```

The compatibility target is Apple's public
[UIControl](https://developer.apple.com/documentation/uikit/uicontrol) and
[UISlider](https://developer.apple.com/documentation/uikit/uislider) API.
Native iOS 27 measurements, reproduction and limits are retained in
[the slider investigation](../slider/README.md).

## State and dispatch

State and event masks use Apple's numeric option-set values and support `|`,
`&`, `~`, `|=` and `&=`. Disabling a control cancels active input while preserving
manually assigned selected/highlighted flags. Disabled controls reject hit
testing. Explicit `sendActions`, `sendAction` and `performPrimaryAction` remain
programmatic dispatch even when disabled; input paths enforce interaction gates.

An action identifier replaces the existing handler and unions registered event
bits. Removing an action can remove only some event bits. Dispatch and
`enumerateEventHandlers` snapshot registrations, so callbacks may change the
registry safely. An action registered for multiple bits runs once per dispatch.
Targets are weak. A null target searches the responder chain, with cycle
protection; a null removal target matches all target registrations. A null entry
in `allTargets()` represents responder-chain routing.

Portable selectors are strings passed to `UIResponder::canPerformAction` and
`performAction`. The default implementation bridges existing named responder
commands. There is no Objective-C runtime or selector signature reflection.
`UIAction` remains a C++ value: copies preserve its stable identifier, and the
handler receives a dispatch copy carrying a weak sender. The original optional
`primaryAction` property is retained as a compatibility adapter; new code should
use `addAction`. Initializer actions participate in the registry.

Touch-down/repeat, inside/outside drag, enter/exit, up and cancellation flow
through the gesture arbitration system and overridable tracking hooks. Parent
visibility, interaction, alpha and removal cancel active control tracking. The
SDL fallback infers repeated taps on the same hit view within 300ms and 22pt,
excluding overlapping contacts, cancelled contacts and substantial movement.
These are portable policy constants, not recovered Apple thresholds. Explicit
platform tap counts are preserved. Leading/trailing content alignment resolves
to left/right until Core has an RTL layout direction trait.

## Slider

The default slider has a 34pt fitting height, 6pt track, and 37×24pt white thumb.
Contact grows the default knob to 1.55× and uses the shared native-profile lens
renderer with elastic view-motion response. Offset pickup does not jump; a 10pt
horizontal deadband precedes relative dragging over the live track width.
Far track contacts do not move the default thumb. Thumbless sliders use direct
track tracking. Custom thumb images keep their natural dimensions without lift.

Release projects the last moving velocity with a .99 deceleration rate and
keeps tracking alive while the value settles. `touchUpInside/Outside` observes
tracking=false, followed by the final `valueChanged`. Noncontinuous mode emits
one final value event. Cancellation emits no deferred final value event. Range
and style changes cancel incompatible active tracking. Programmatic setters are
silent and can interrupt value animations. Geometry follows bounds, endpoint
images and traits; there are no reference-device layout constants.

Keyboard/controller arrows and the portable accessibility increment/decrement
methods change by 1% of the range, or to the adjacent enabled tick. The platform
accessibility bridge and haptics remain unimplemented infrastructure.
`UISliderStyle::defaultStyle` represents Apple's `default` enum case, which is a
reserved C++ keyword. Track configurations are copied on assignment and return
editable copies; finite values and null tick entries are validated. Tick-only
snapping, enabled ranges, neutral fill and title/image labels are portable
behavior; complete native tick presentation parity is not established.

The exact private velocity estimator, flex solver, projection activation cutoff,
and interrupted-animation timing remain approximations. The measured cutoff is
bracketed between 60 and 122pt/s; this port uses 100pt/s. No private Apple API is
linked. OS menus, tooltip interaction, context-menu behavior and symbol effects
are not added as empty public stubs.

## Verification

`NXKitControlTests` exercises registrations, reentrancy, weak ownership, state,
responder-chain dispatch and real window-routed touch events. `NXKitSliderTests`
compares 35 independent native geometry cases and six retained OS-touch traces,
plus range/image/configuration, cancellation, nested-scroll and lens behavior.
`SimulatorGlassBlurTests --slider-only` compares Raster and Metal for 72 cases
across 1×/2×/3×, both themes, disabled state and partial/full lens lift.

Regenerate/check golden data with `Demo/SliderComparison/generate-geometry.py`
and `generate-touch.py`; `--check` verifies exact source-derived expectations.
The Glass appearance Demo uses live adjustment and writes its per-app setting
on release, retaining Follow system.
