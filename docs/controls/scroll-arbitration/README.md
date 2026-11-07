# Control and scroll gesture arbitration

Native reference captured 2026-11-07 on iPhone 17 Pro Max Simulator, iOS **27.0 (24A434)**, built against SDK 27.1. This is a finite 28-case native UIKit reference, not a claim about all UIKit versions, platforms, or input types. Native transport test result: **TEST SUCCEEDED**; all 28 traces contain a complete physical touch sequence. This collection fixture asserts that the application is ready and drives gestures; it does not assert that native outcomes conform to a separate implementation.

The reference uses public XCUITest `pressForDuration:` / `thenDragToCoordinate:withVelocity:thenHoldForDuration:` to deliver actual OS touches. No synthetic `UITouch`, direct tracking calls, authored lens state, custom gesture delegates, or failure-dependency mutations were used. Observer targets were added to the native recognizers, public scroll/control overrides forwarded to `super`, and a display link sampled read-only state and lens presentation bounds. Instrumentation adds overhead; timings below are relative to the first **UIWindow delivery**, rather than a guarantee of hardware latency or an exact recognition threshold.

## Observations

The important distinction is **buffered view touch delivery versus gesture recognition and visual feedback**. `UISlider` and `UISwitch` do not have the same pickup policy.

| Native case | Observed ownership and feedback |
| --- | --- |
| Switch stationary hold, default scroll settings | Scroll's `touchesShouldBegin:withEvent:inContentView:` returns YES around 150 ms. Internal long press starts its target action about 20 ms later; the lens then lifts. |
| Switch vertical drag before admission | Scroll wins; switch does not lift or change. Both 100 pt/s and 1000 pt/s cases passed. |
| Switch horizontal drag at 100 pt/s | Switch's target actions and lens lift wait until the scroll delay expires. Scroll offset remains zero; switch changes on release. |
| Switch hold 650 ms, then vertical drag | Already admitted switch retains ownership, stays lifted, and changes on release. Scroll offset stays zero; no public cancellation hook is called. |
| Switch tap lasting 50 ms | Scroll releases delayed touches at touch-up; internal press/action feedback runs afterward and the switch toggles. |
| Switch outside a scroll view | No scroll admission delay; lens lifts around 34 ms in this run. |
| Switch with `delaysContentTouches = NO`, vertical drag | Internal switch press wins before scroll; switch lifts and toggles, scroll offset remains zero. |
| Switch in a horizontally scrolling ancestor, horizontal drag | Scroll wins before admission; no lift or value change. |
| Slider **thumb** stationary hold | Thumb's own zero-duration expansion recognizer lifts immediately, independently of delayed `UIControl` tracking. Tracking begins around 150 ms in the default scroll view. |
| Slider thumb vertical or horizontal drag | Slider owns both axes in the tested native cases, including fast vertical movement and horizontal movement in a horizontally scrolling ancestor. Vertical thumb movement leaves the value unchanged but does not scroll. |
| Slider **track**, outside the thumb, fast vertical drag | Scroll wins, no thumb lift or control tracking. This also verifies that the fixture's scroll view is genuinely scrollable. |
| Slider track tap | Native `beginTrackingWithTouch:withEvent:` returns NO and the value stays unchanged. |

A deliberately short edge case also matters when interpreting recognizer state: a switch drag of 70 pt at 1000 pt/s, followed by only 50 ms of holding, finishes before the 150 ms scroll delay. Its internal pan reports `Began` while target actions remain deferred. At touch-up, `Began` and `Ended` target actions flush together; the switch does not lift or change in this run. This is a bounded observation, not an instruction to generalize this outcome to all horizontal drags.

### Recognizer settings read from UIKit

| Recognizer | `delaysTouchesBegan` | `delaysTouchesEnded` | `cancelsTouchesInView` | Other observed settings |
| --- | --- | --- | --- | --- |
| `UIScrollViewDelayedTouchesBeganGestureRecognizer` | YES | YES | YES | Scroll `delaysContentTouches` and `canCancelContentTouches` both default YES. |
| `UIScrollViewPanGestureRecognizer` | NO | NO | YES | Scroll owns the delegate. |
| Slider's `UIPanGestureRecognizer` | NO | YES | NO | Delegate `_UIFluidSliderElasticPanDriver`. |
| Slider's `UILongPressGestureRecognizer` | NO | YES | NO | `minimumPressDuration = 0`, `allowableMovement = DBL_MAX`; delegate `UIKit._UISliderGlassVisualElement`. |
| Switch's `UITapGestureRecognizer` | NO | YES | YES | Attached to the switch. |
| Switch's internal `UILongPressGestureRecognizer` | NO | YES | YES | `minimumPressDuration = 0.01`, `allowableMovement = 10`; attached to `UISwitchModernVisualElement`. |
| Switch's internal `UIPanGestureRecognizer` | NO | YES | YES | Attached to `UISwitchModernVisualElement`. |

The native lens responds to the read-only getter named `lifted`; `isLifted` is not exposed by this build. Both its model state and its presentation bounds are recorded. The slider's expansion gesture is therefore not equivalent to its `UIControl.tracking` state. The native switch implements these operations through internal recognizers and does not call the probe's public control tracking overrides on its standard delayed gesture path.

`touchesShouldBegin` receives the actual hit `_UILiquidLensView` for thumb hits. In the slider's held-then-vertical case, `touchesShouldCancelInContentView:` returns YES, but the slider pan still wins; a YES from this hook is not itself evidence that cancellation happened. None of the tested default admitted-switch paths call this hook after its long press owns the sequence.

## Evidence and reproduction

- [Compact native results](native-summary.json): first visual lift, first public touch delivery, first target actions, public scroll hook calls, final value/offset, and sequence completeness.
- [Original native test log](native-test.log.gz) and [build/source manifest](native-manifest.json).
- [Raw trace index and SHA-256 hashes](native-traces/index.json). Each compressed JSON retains all window events, control events, recognizer inventories/states/settings, and lens model/presentation geometry.
- [Exact v2 source snapshot](fixture-v2/Reference.m) matching that full capture's source hashes. The current fixture additionally supports a small selectable replay and compact stdout summary.

Full replay:

```sh
python3 Demo/ControlScrollComparison/collect.py DEVICE_UUID /private/tmp/control-scroll-native
python3 Demo/ControlScrollComparison/analyze.py /private/tmp/control-scroll-native
```

Focused ten-case replay (both controls: tap, fast vertical, fast horizontal, long hold; additionally slider track vertical and switch held vertical):

```sh
python3 Demo/ControlScrollComparison/collect.py DEVICE_UUID /private/tmp/control-scroll-primary --primary
```

The collector snapshots the source before building, copies only traces updated during that capture, and emits compact observed results on stdout for the REA process-capture record. All probe applications use the isolated bundle identifier `org.nxkit.ControlScrollReference`.

## Primary replay and scheduling limitation

A later focused ten-case replay also returned native XCTest **TEST SUCCEEDED** and retained ten complete physical touch sequences. This is a transport result, **not ten passing behavior comparisons**. Its [summary](primary-runtime/summary.json), [source/build manifest](primary-runtime/manifest.json), [raw trace index](primary-runtime/traces/index.json), and [exact source snapshot](primary-runtime/fixture-snapshot/Reference.m) are retained separately from the full v2 run.

The `switch-tap` outcome differs: the full v2 run toggles, while the primary replay remains off. The point, hit `_UILiquidLensView`, requested 50 ms press, scroll defaults, and recognizer settings match. Actual OS touch timestamps show a 66.67 ms physical hold in **both** runs, but main-thread delivery differs:

| Measurement | Full v2 | Primary replay |
| --- | ---: | ---: |
| Touch-down timestamp to UIWindow delivery | 26.22 ms | 99.72 ms |
| UIWindow down-to-up delivery interval | 58.69 ms | 6.78 ms |
| Display-link callback gap spanning UIWindow down | 32.12 ms | 105.32 ms |
| Native switch value after the sequence | 1 | 0 |

The primary run batches down/up delivery into less than the switch long press's 10 ms minimum. Its long press fails without dispatching its target actions, the lens does not lift, and the value remains zero. This explains why the replay cannot serve as evidence that every 50 ms physical tap is admitted identically under Simulator load. The underlying cause of the delayed delivery is **not established**. The probe currently serializes and writes its growing trace synchronously on the main thread every 150 ms, so instrumentation is a plausible contributor; host/Simulator scheduling is another. No timed save instrumentation or isolated ablation was captured to assign causality. The [exact timestamp comparison](primary-runtime/tap-delivery-comparison.json) preserves the measurements.

The REA wrapper executed this replay but returned `cleanup_incomplete`, an empty resource list, and **no runtime Evidence ID**. The retained filesystem outputs are native runtime artifacts; they must not be presented as a completed REA evidence receipt. This failure does not invalidate the separately completed static investigation below.

## Shipped UIKit static evidence

REA Evidence **`ev_b28bfa8f1111483ee53bd134326affbb49f1f9230bcd03154e27e011053a1c18`** records read-only disassembly of the shipped UIKitCore artifact, with binary and method hashes in [the static manifest](static-manifest.json) and the [REA result](rea-static.json). This is static evidence, not execution of those methods.

In `-[UISwitchModernVisualElement gestureRecognizer:shouldRequireFailureOfGestureRecognizer:]`, the code compares the first recognizer with its own `longPress` and `pan`, then tests the other recognizer with `isKindOfClass:`. The class reference was resolved to **`UIScrollViewDelayedTouchesBeganGestureRecognizer`**. Thus this binary explicitly asks its own switch press/pan recognizers to require the ancestor scroll-delay recognizer to fail. [Method disassembly](disassembly/switch-failure-policy.txt).

The switch's simultaneous-recognition delegate accepts its own long press/pan pair. The scroll-delay recognizer's touch-began implementation checks `delaysContentTouches` and schedules a timer using the scroll view's `_touchDelayForScrollDetection`, or calls `sendTouchesShouldBeginForTouches:withEvent:` directly when delay is disabled. These findings support implementing shared recognizer dependencies and scroll admission instead of unrelated hardcoded control priorities. They do not establish the entire dependency graph or all timing constants on other UIKit builds.

## Inferences and unknowns

The buffering and deferred target actions indicate gesture dependencies beyond mere UIControl hit testing. The runtime fixture **does not directly observe calls** to `requireGestureRecognizerToFail:`, `gestureRecognizer:shouldRequireFailureOfGestureRecognizer:`, or `gestureRecognizer:shouldBeRequiredToFailByGestureRecognizer:`. Class names and state transitions alone do not prove which API established a relationship. The separate static investigation above identifies the switch dependency in one shipped artifact; the full runtime fixture itself does not observe that delegate invocation.

The tests cover a single active finger, default slider/switch styling, one native OS build, configured portrait geometry, and the listed scroll settings. They do not establish multitouch, VoiceOver, RTL, disabled-control, nested-scroll, arbitrary diagonal-velocity, or earlier-OS parity. Frame sampling limits the precision of first lift timing. The initial pre-v2 debug run had a nil-touch-view logging crash during cancellation; those incomplete switch captures are excluded from this retained reference.

## Portable implementation

Core now supplies weak, cycle-checked gesture failure requirements, dynamic
failure ordering, touch/event/press admission filters, and shared responder touch
buffering. Deferred delivery restores the original touch phase, coordinates and
timestamp while retaining its identity and matching event membership. A gesture
can therefore release or cancel pending content without each control inventing
its own scroll timer. Removal, disabling and reentrant callbacks retire pending
attempts safely.

UIScrollView exposes `delaysContentTouches`, `canCancelContentTouches`,
`touchesShouldBegin` and `touchesShouldCancelInContentView`. Its private finite
150 ms gate applies only while an enabled pan can compete, using live scroll
capacity, insets and bounce settings. Standalone controls, disabled or
non-scrollable ancestors, and pan-delegate-rejected contacts add no scroll delay.
The capacity check is the user's requested policy; it is not a claim that every
native UIKit version uses that exact optimization. No repeating timer or idle
display link was added.

UIControl's actual tracking now follows shared responder admission. UIKit
providers can independently show contact feedback: a slider thumb lifts early
and captures movement on either axis, preserving the original down point; a
slider-track hit can yield to scrolling. Switch pickup waits for admission,
then retains ownership, including held vertical movement. Its modern event
ordering and false public tracking/highlight flags remain intact. A completed
pending pan cannot turn into a tap after a reversal, and a slider that rejects
tracking does not reserve the scroll gesture.

The controls use a shared portable tracking recognizer rather than reconstructing
every private UIKit recognizer object. Exact switch intrinsic 10 ms press timing,
full private recognizer-state introspection, generic hardware/controller press
buffering, multitouch controls and physical-device parity remain incomplete.
The user-requested immediate standalone response is preserved. Public gesture
and scroll APIs remain marked Partial in `UIKIT_API_PROGRESS.md`.

The focused end-to-end regression is `NXKitControlScrollArbitrationTests` in the
normal build and `ControlScrollArbitrationTests` in the headless raster build.
It checks the retained native ownership outcomes plus local interruption,
reversal and rejected-tracking continuations. Core's `GestureFailureTests`,
`TouchDeliveryTests` and `ScrollContentTouchTests` cover the shared mechanism.


## Verification

The final **34-case** control/scroll matrix passes on both Raster and iOS
Simulator. Dependency, responder-buffering, scroll hook, legacy gesture,
slider/switch, scroll physics and interactive navigation checks pass in the
corresponding tested targets. The macOS UIKit module and Simulator Demo build
successfully. [Verification manifest](verification.json) records exact source
hashes, compressed test/build logs, scope and remaining limitations.

Cold bundled-shader replays cover **1,941 frames**: 1,921 lens-motion frames and
20 control-gallery frames. Library compilation remains zero and the pipeline
counter stays at its 220-pipeline startup prewarm value throughout rendering.
No render-time library or pipeline compilation occurs in these captures.
These are Simulator correctness/cache checks, not iPhone frame-rate or thermal
measurements.
