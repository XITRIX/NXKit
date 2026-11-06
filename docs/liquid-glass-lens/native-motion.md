# Native lens touch motion

This evidence comes from **iPadOS 27.0 (24A434), 2×**, running UIKit's own
`UITabBar`, `UISlider`, and `UISwitch`. The fixture does not move or transform
their lenses after initial control setup. XCTest delivers actual OS touch
events, and a window observer records the `UITouch` timestamps and stage
coordinates before UIKit handles them. Native model and presentation layers
are then sampled on a display link. Median sample spacing is about 16.67ms.

The [retained traces](native/motion/manifest.json) include compressed full
layer trees, active-lens CSVs, touch CSVs, summaries, and contact/release images.
The complete session, runner logs, and illustrative videos are retained at
`/private/tmp/nxkit-lens-motion-20261028`. These are native observations, not
measurements of NXKit's reconstructed motion.

## What moves

Two effects coexist. UIKit changes the native control lens's **model bounds**
when contact lifts it, while a separate **presentation transform** stretches,
compresses, and rings around identity. Control placement and selection also
move the model center. Subtracting all model changes would incorrectly remove
part of the native response; treating model placement as the finger target
would also confuse slider tracking and tab selection with elasticity.

| Actual control / input | Rest model size | Largest model size | Presentation horizontal scale | Presentation vertical scale |
|---|---:|---:|---:|---:|
| Slider, right 200pt/s | 37×24 | 57.35×37.20 | .960–1.045 | .955–1.041 |
| Slider, right 700pt/s | 37×24 | 57.35×37.20 | .892–1.119 | .880–1.109 |
| Slider, left 200pt/s, dark | 37×24 | 57.35×37.20 | .961–1.043 | .956–1.039 |
| Slider, continuous reversals | 37×24 | 57.35×37.20 | .933–1.071 | .928–1.068 |
| Tab selection, right 200pt/s | 84×36 | 115.50×52 | .956–1.061 | .938–1.060 |
| Tab selection, continuous reversal | 84×36 | 115.50×52 | .909–1.093 | .873–1.120 |
| Switch, right 80pt/s | 37×24 | 58×38.33 | .992–1.010 | .990–1.008 |

These are observed extrema for the declared paths, not universal constants or
an automatic size threshold. The tab lens also changes width for different
selected items. Its width range includes both selection geometry and lift.
The switch has its own discrete tracking behavior. Slider scale oscillations
continue after the finger stops and after release; the slow left/right pair
has similar deformation magnitudes. Faster motion causes a much larger
response. This supports a motion-driven elastic state rather than a fixed
pressed transform, but does not identify Apple's internal force equation.

The private native lens has `_UIFlexInteraction` and
`_UIUpdateLinkViewInteraction` attached. Names and layer outputs establish
composition and behavior, not private implementation details.

Separate diagnostic replays inspect exposed Core Animation metadata at each
layer's first nonidentity presentation transform. Both active lens roots
report **empty `animationKeys`** then; no public `CASpringAnimation` or spring
coefficients were observed. The tab punchout has a private
`CAMatchMoveAnimation` named `_UILiquidLensView.punchout.matchPosition` with
nonfinite duration, without public oscillator parameters. This negative
result rules out claiming a directly measured Core Animation spring law; it
does not rule out UIKit's private flex state driving the presentation tree.

A later, separate read-only diagnostic records the actual flex and integrator
getters during slider reversal and tab dragging. Both report:

| Native integrator getter | Observed value |
|---|---:|
| `hysteresisTimeInterval` | 0.05 seconds |
| `maxNumberOfSamples` | 5 |
| `minimumRequiredMovement` | 0 |
| `prefersDirectionlessAcceleration` | true |
| `useSampleWindowForDifferentiation` | false |

The [exact runtime descriptors](native/motion/flex-runtime-classes.json)
show a `_UIVelocityIntegrator` and separate debounced scale/drift numeric
objects. The `motion-*-flex-diagnostic` records retain their measured values
and focused `flex.csv` tables, together with native position, velocity,
acceleration, and offset. `shouldUsePresentationLayers` changes from false to
true during activation. Getter names establish observable state, not the
exact filtering, hysteresis, force, or integration algorithm. In particular,
50ms is an observed integrator setting and must not automatically be modeled
as a pure transport delay. Integrator positions use window-like coordinates,
whereas touch points are stage-local; `stageInWindow` records the translation.

## Reversals, release, and interruption

The reversal runner creates one continuous OS contact. The actual received
touch stream is authoritative: the requested offsets and times in each
configuration are inputs, and delivery may be quantized or delayed. The
slider path holds, moves right, holds, reverses left, holds, and moves right
again before lifting. The tab path reverses across selection positions.
Each trace continues for at least 1.5 seconds after the received touch ends.

The `slider-cancel` control disables `userInteractionEnabled` at 0.7 seconds.
That is an explicitly logged **native control interruption**, not a claimed
OS cancellation. The recorded final touch phase is `Ended` (3), rather than
`Cancelled` (4). A genuine OS `UITouchCancelled` trajectory remains unmeasured.

The separate `tabs-snapshots` replay records UIKit-rendered contact/release
images with actual start/end capture times. Image capture perturbs timing,
so it is excluded from coefficient estimation. Passive simulator videos
illustrate the uninstrumented replays; their clock is not calibrated against
`UITouch.timestamp` and they must not supply precise timing measurements.
The historical `contact-*` filenames denote elapsed checkpoints from touch
start, and later checkpoints can follow release. Actual snapshot times and
the received touch stream determine the phase, rather than the filename.

## Reproduce

`Demo/GlassComparison/build-lens-motion.py` creates an isolated app and test
runner. `capture-lens-motion.py DEVICE OUTPUT CASES_JSON --video` builds and
runs them, retains raw traces and source hashes, and never resets simulator
services. `analyze-lens-motion.py TRACE OUTPUT` extracts the active lens by
observed movement without subtracting an assumed target trajectory.

Straight drags use public `XCUICoordinate` press/drag/hold APIs. Continuous
reversals require XCTest's native event-path interfaces, confined to this
comparison runner. The selected constructors/selectors and callback signature
were checked against the running runtime and recorded in the test log. Primary
research references are the [Appium input-event guide](https://github.com/appium/appium-xcuitest-driver/blob/master/docs/guides/input-events.md),
the [published XCTest pointer-path declaration](https://github.com/appium/WebDriverAgent/blob/master/PrivateHeaders/XCTest/XCPointerEventPath.h),
and [its current event-synthesis call site](https://github.com/appium/WebDriverAgent/blob/master/WebDriverAgentLib/Utilities/FBXCTestDaemonsProxy.m).
Only API facts informed the independently written test plumbing; no Appium,
Telegram, or Apple implementation was copied into portable NXKit.

The original manual-lens cosine center paths remain historical, explicitly
**authored input** evidence in [research.md](research.md#lifted-appearance-and-motion).
They are not substitutes for these native touch traces. A caller-prescribed
center path can excite native lens response—Telegram's published wrapper even
[reapplies center while native bouncing runs](https://github.com/TelegramMessenger/Telegram-iOS/blob/6ad963e5b62d354da79040f388ae2b9132fb17b8/submodules/TelegramUI/Components/LiquidLens/Sources/LiquidLensView.swift#L388)—
but must remain distinguished from native control tracking.

## Limits

This dataset covers a portrait simulator, two native lens styles through
system controls, three control geometries, one dark-theme slider case,
straight horizontal drags, reversals, release, and a declared interruption.
It does not establish arbitrary diagonal/vertical contact behavior, OS
cancellation, device touch latency, reduced-motion behavior, or a universal
spring law. Native control lift-size changes are not an instruction for a
portable `LensView` to secretly rewrite caller-owned bounds. Mesh/SDF-only
deformation cannot be ruled out by affine traces; the retained images provide
an independent visual check.
