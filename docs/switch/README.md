# UISwitch native reference

This is a build-specific UIKit compatibility reference, measured on the iOS 27.0 phone Simulator (iPhone 17 Pro Max, scale 3). It is not a recovered private header and does not establish behavior on every Apple platform.

The public contract comes from [Apple's UISwitch documentation](https://developer.apple.com/documentation/uikit/uiswitch) and the installed SDK's `UISwitch.h`: `isOn`, `setOn:animated:`, nullable `onTintColor`, `thumbTintColor`, `onImage`, `offImage`, resolved `style`, and `preferredStyle`. `setOn:animated:` does not send an action. `title` and checkbox presentation are Catalyst Mac-idiom features, rather than required portable iPhone behavior.

## Measured geometry and appearance

The attached default sliding switch enforces a 63 × 28 point frame, including when constructed with a 51 × 31 or 200 × 100 request. Both `intrinsicContentSize` and `sizeThatFits:` report 61 × 28. This width discrepancy is observable native behavior. Explicit `setFrame:` and `sizeToFit` still enforce 63 × 28. Direct `setBounds:` accepts arbitrary bounds but leaves the internal 63 × 28 visual element unchanged in the bounded immediate-layout probe.

The resting knob is 37 × 24 with a 2 point inset: x = 2 off and x = 24 on. Forced RTL swaps these positions. The track corner radius is 14. A held knob's target size is 58 × 38⅓; its spring overshoots the width to approximately 60.6 points in the long hold. The target size is pixel-aligned on this 3× reference; do not assume the last fractional point is a universal metric.

The knob uses `_UILiquidLensView` style 0 (small), `warpsContentBelow = YES`, preserved source mode 0, nil source view, and an opaque white resting color. Its resting border and shadow overrides are nil. More importantly, the resting lens, its content host, and its background child all have **zero shadow opacity**. The slider's explicit resting shadow should not be copied to this control.

The default off track is `tertiaryLabelColor`; the on track is `systemGreenColor`. `onTintColor` and `thumbTintColor` getters initially return nil. Their overrides affect the on track and resting knob respectively. Disabling the switch sets the internal visual element's opacity to 0.5 while leaving the public root at opacity 1. Assigning `onImage`/`offImage` leaves the modern provider's two image-view bounds empty, with no observed image presentation. Requesting checkbox style is stored, but `style` resolves to sliding on this phone. Setting `title` throws the native unsupported-Mac-idiom exception.

## Touch and setter observations

Three real OS touch cases were captured: an off-to-on tap in light mode, an on-to-off stationary one-second contact in dark mode, and an off-to-on drag in light mode. Both stationary cases emit `valueChanged`, followed by `touchUpInside`, on release. The drag case emits `valueChanged` on release and no `touchUpInside`. No `touchDown` event was observed. Public `tracking`, `touchInside`, and `highlighted` remain false in these native samples; the modern provider uses its own interaction path.

The public animated `setOn:YES` moves the resting 37 × 24 knob to the other endpoint without creating a lifted backdrop subtree. It emits no control events. This is distinct from a finger-driven state change.

The full long-contact [motion CSV](native/lift-motion.csv) retains model and presentation geometry, with time measured from the observed down/up dispatch. The display callback and compositor are not synchronized at zero latency: fit a commit-phase offset when comparing springs. The exposed `SwitchWellPosition` `CASpringAnimation` has mass 1, stiffness 85.3772, and damping 18.47996, but it is the **well image's** spring; the knob's internal frame spring is not exposed as a root CA animation.

## Reproduce

Build the standalone oracle without touching the NXKit main build:

```sh
python3 Demo/SwitchComparison/build.py /private/tmp/nxkit-switch-native Demo/SwitchComparison/cases.json
xcodebuild -project /private/tmp/nxkit-switch-native/Interaction.xcodeproj -scheme Interaction -destination 'platform=iOS Simulator,id=3F494163-D29F-446D-9379-20EBA1E70C53' -derivedDataPath /private/tmp/nxkit-switch-native/build build-for-testing
xcodebuild -project /private/tmp/nxkit-switch-native/Interaction.xcodeproj -scheme Interaction -destination 'platform=iOS Simulator,id=3F494163-D29F-446D-9379-20EBA1E70C53' -derivedDataPath /private/tmp/nxkit-switch-native/build test-without-building
```

The source is adapted from the repository's slider touch oracle. UIKit itself handles touches, state, and lens lift; the probe only samples getters/layers and calls public setters in the explicitly marked programmatic case. Read-only private lens getters use checked runtime method signatures. XCTest's event path APIs are used only by the native test runner, not by NXKit.

## Evidence and limits

The successful three-touch run is retained in [REA capture](rea-touch-capture.json), Evidence `ev_633c53e8cc9360cd5e9e0d9ed1ca2a94b1c89a534e5c4446631359936ef82bc4`. The command exits 0. REA's terminal capture is marked truncated; the separately retained complete `native/*/trace.json.gz` files and geometry records are the quantitative source. REA does not independently observe filesystem writes in this run. The programmatic probe was initially executed directly through `simctl`; its configuration and empty touch list distinguish it from OS-touch observations.

Cancel behavior, drag-reversal thresholds, velocity-dependent release, accessibility-switch-control, Catalyst checkbox presentation, arbitrary scale metrics, and exact pixel equivalence remain unmeasured here. Diagnostic layer sampling adds overhead, so these captures must not be used as performance measurements.
