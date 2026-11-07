# Elastic retirement after lowering — 2026-11-07

Elasticity remains active for the whole lifted state, including long holds and
continued deceleration after the finger releases. Lowering starts a two-second
fade to exact identity, even if the knob or its ancestors keep moving. Repeated
lowered-state, layout and control-lift updates cannot extend the fade. Re-lifting
cancels retirement and restores active elasticity without a transform jump.
Contact-only clients that never lift begin retirement on `endInteraction`.

This corrects the previous lift-start timeout, which misinterpreted the user's
request. The two-second post-lowering envelope is an intentional portable
behavior choice, not a claim that UIKit exposes or uses this exact timeout.

Only the additional elastic affine fades. Caller-owned position, control
deceleration, lifted geometry, glass material, refraction and shadow state
continue on their existing clocks. Slider, TabBar, Switch and custom clients
use the same LensView policy for contact and view-motion input.

The retirement weight is `entryWeight * (1 − smootherstep(t / 2s))`, where
`t` starts at the transition from lifted to lowered. Its first two derivatives
reach zero smoothly, avoiding a transform snap at the deadline. Force history
is then cleared and the motion sampler stops. Interrupted retirement retains
its current weight and renews strength over 80ms on re-lifting. While lifted,
time alone never reduces elasticity. Hiding, disabling or reparenting still
cancels stale interaction history immediately.

The measured underlying transfer reconstruction is unchanged. Historical
`LensMotionTests` and native fit reports describe that raw transfer; this public
view envelope is a separate policy. The preceding
[native activation investigation](../relative-motion/README.md) remains evidence
for resting versus lifted/global motion, rather than for this timeout.

## Verification

The 32 response cases cover small/large × contact/view-motion × 60/120Hz ×
short/three-second holds × ordinary/control-driven lift. They require continued
elasticity beyond two seconds while lifted, no early retirement after contact
ends during view-motion deceleration, fading during continuing movement after
lowering, exact identity by two seconds after lowering, no deadline renewal by
repeated lowered updates, pose continuity on interrupted re-lifting, continued
response during the next long hold, and cancellation on hide/re-show. The
existing 96 activation cases and Slider/Switch behavior suites also pass on
Raster and Simulator (the Slider/Switch suites run on Simulator).

Current correction logs are retained under `release-verification/`.
`verification/` contains the historical superseded lift-start policy's logs.
Simulator Demo and macOS UIKit builds pass. The refreshed offline package passes
cold motion/gallery replays without additional shader or pipeline compilation
during rendering.
The corrected signed build was installed and launched on iPhone Pro XITRIX on2026-11-07.
