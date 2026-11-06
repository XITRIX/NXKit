# Lens contact and elastic geometry

This work measures deformation independently from model placement. The former
fixture prescribed a lens center path but did not isolate native descendant or
visible-silhouette deformation, so that path alone cannot establish UIKit's
velocity response. Actual XCUITest contacts provide the preferred input. A
controlled native model-center path is also valid input if the native internal
presentation or visible deformation is measured independently rather than
authored as output.

## API and coordinate ownership

Interaction is opt-in with `setInteractive(true)`. A containing control owns the
lens's model frame, selected item, lifted state, and hit testing. It forwards
contact through `beginInteraction(location)`, `updateInteraction(location,
velocity)`, and `endInteraction(velocity, cancelled)`. Locations and velocities
use the lens superview's coordinates, in points and points per second. The
one-argument update estimates velocity using a monotonic clock; explicit
velocity is preferred for recognizers and deterministic trace replay.

Direct touches use a passive observer and the same hooks. The observer does not
cancel descendant controls or prevent ancestor recognizers. Explicit input works
when the lens itself has ordinary user interaction disabled, as in the Demo's
selection control. Disabling interaction, hiding/removing/reparenting the lens,
and cancelling contact must not leave deformation or a live contact behind.

The elastic affine belongs to a private root layer. The application frame and
application transform are not overwritten. The resting fill, optical backdrop,
selected-source output, contact shading and final capsule therefore share one
presentation transform. `interactionTransform()` returns this extra local
transform so the containing control can align its normal-content punchout using
the actual presentation position, anchor and application transform.

The full selected source stays in its stationary underlying row. Its existing
source-to-owner conversion includes the inverse of the root elastic transform;
the source is not first stretched with the lens and then sampled again. Output
clipping occurs after source sampling.

## Implemented response and evidence

The completed opt-in path has two exclusive input modes. `contact` is the
portable fixed-knob extension; `viewMotion` uses global presented placement,
including ancestor movement and application transforms while excluding the
lens's own elastic affine. Changing mode clears its history. Explicit contact
hooks remain responsible for contact lifetime in either mode.

Actual UIKit touch trajectories constrain separate unsigned strain and signed
drift responses. The analytic finite-window damped model preserves reversal
history, remains bounded for extreme inputs and settles exactly after release.
It fits the combined displayed response rather than copying a private UIKit
integrator. Native 50ms/five-sample getter observations and the independently
fitted response window/phase remain separate in the
[recorded provenance](interaction/replay-metadata.json).

The [motion comparison](motion/README.md) retains all eight scored trajectories,
shared caller placement/lift/ancestor inputs and explicit residuals. The model
improves every path over rigid geometry; tab reversal remains 0.872pt RMS with
a 3.793pt peak coordinate error. Contact-driven fixed-knob behavior is an NXKit
extension. Native diagonal/vertical response, OS cancellation, other runtimes,
full control tracking and GPU parity remain unverified.

Regression tests cover the actual root transform and stationary source in both
styles, fractional bounds, fixed/moving exclusive inputs, ancestor movement,
release/cancel, disable/hide/reparent, finite input, analytic display subdivision
and retained native replay. Integrated tab tests use real event routing with
both interactive material observers and check actions, settling and punchout
alignment. Production contains no private UIKit calls.
