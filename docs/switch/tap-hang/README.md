# Short switch taps complete the shared lens lift

The old switch ended its expansion immediately when the contact ended. A quick tap could therefore start lowering before the knob had reached its full glass shape. Native UIKit separates contact completion from the lens's visual lifetime: an animated lift has a **0.22 second minimum hang**, shared by small and large lens styles.

## Native evidence

A fresh native `UISwitch` capture on iOS 27.0 (24A434), iPhone 17 Pro Max Simulator at scale 3, exercised off→on light tap, on→off dark tap, 120 ms hold, and 300 ms hold. These used the existing OS-touch [SwitchComparison fixture](../../../Demo/SwitchComparison/SwitchReference.m) and XCUITest runner, with trace output written once after the motion ended. No control tracking calls or lens state were authored in these switch cases.

| Native sequence | Delivered contact duration | First sampled model lift to sampled model lower | Maximum presentation bounds width |
| --- | ---: | ---: | ---: |
| Off→on tap, light | 80.31 ms | 212.30 ms | 60.5959 pt |
| On→off tap, dark | 64.27 ms | 211.02 ms | 60.5997 pt |
| 120 ms requested hold | 130.96 ms | 199.88 ms | 60.5976 pt |
| 300 ms requested hold | 297.05 ms | 266.50 ms | 60.5991 pt |

The model intervals in this table are **sampled bounds**, not precise timer measurements: first lift may occur before its first display callback. Both short taps nevertheless complete the full 58 pt lift and approximately 60.60 pt spring overshoot before shrinking. The long hold lowers when released, since its minimum hang has already elapsed. Control `valueChanged` remains followed by `touchUpInside`; neither waits for visual lowering. [Full fresh summaries](native/tap-summary.json), [source/build manifest](native/source-manifest.json), and [raw native traces](native/manifest.json).

The exact timer was checked independently with direct app-space calls to native `_UILiquidLensView`, using both styles and animated lowering requested immediately or after 30, 100, or 300 ms. Requests before the deadline deferred the alongside callback until about 220 ms from lift; 300 ms requests executed it immediately. This fixture authors private API calls deliberately and **does not test touch routing**. It does not change native timing or spec settings. Source: [MinimumLiftReference.m](../../../Demo/SwitchComparison/MinimumLiftReference.m); reproduction: [capture-minimum-lift.py](../../../Demo/SwitchComparison/capture-minimum-lift.py); [retained manifest](minimum-lift/manifest.json) and [trace](minimum-lift/trace.json.gz).

This independent run has REA Evidence **`ev_ffdaff76e537dc4e121beafdb8756001a3a80d48803c4e783b9aad95f3a5c58a`**. Its [receipt](../../controls/list-selection-switch-tap/rea-minimum-lift.json) records the bounded authored scenario. Earlier shipped-code evidence establishes that the setter reads `spec.lensHangTime`, subtracts time since lifting, and schedules `unliftDelayTimer` for the remainder; see the [lens API audit](../../liquid-glass-lens/api-audit/README.md). Both captured native profile variants already contain `lensHangTime = 0.22` in `NativeLensProfileData.hpp`.

## Implementation

`LensView` owns the minimum interval and delays the alongside callback, so the setting remains shared across controls. `UISwitch::setPressed` now changes its geometry spring target **inside that callback**, using the lens's actual lift state. Contact ownership, value changes, and action delivery remain independent and finish immediately. There is no additional switch timer.

The position spring has its own clock. An animated programmatic `setOn` no longer restarts the independent elevation clock. Cancellation, disabling, removal, and nonanimated state assignment can force the visual state back to rest and invalidate a pending visual transition. Cleaning up an already completed tap does not emit a new touch cancellation or undo its committed value.

Focused switch regressions cover short and long contacts at 60/120 Hz, both initial values, complete expansion/overshoot, immediate control events, forced cancellation, pending visual cleanup after release, and independent programmatic position changes. Existing native held-lift and lowering-curve comparisons continue to cover the spring shapes. The shared lens deadline and callback ordering have separate LensView regressions.

Verification completed with `NXKitSwitchTests` in the macOS build and `SwitchTests` in the headless raster harness: **38 new tap/lifecycle/clock cases**, **96 existing trait cases**, and zero failures. Existing native comparisons retain maximum residuals of 0.008494 pt for held lift, 0.000000600 pt for programmatic position, and 0.000001862 pt for lowering bounds. `generate-motion.py --check` passes for 31 position and 27 lowering samples. Simulator integration checks are reported separately by the parent task.

The finite captures do not establish all rapid repeated-tap, multitouch, accessibility, RTL, or OS-version combinations. Absolute display callback timing includes Simulator and instrumentation scheduling; the direct alongside-call trace, rather than sampled bounds alone, establishes the 0.22 second policy.
