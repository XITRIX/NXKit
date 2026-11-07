# Knob motion analysis

The immutable traces referenced by [the native audit](README.md) establish
separate geometry responses for programmatic movement and touch elevation.
The control does not reuse the native `SwitchWellPosition` image animation for
the knob.

For programmatic `setOn:YES animated:YES`, the presentation position follows a
critically damped response with `duration = 0.3` in the shared oscillator's
`omega = 2π / duration` convention. For equally spaced native presentation
samples, the error sequence has the repeated characteristic root
`exp(-2π × dt / 0.3)`. Consecutive three-sample windows recover the 0.3 value
to floating-point precision. Apart from the first diagnostic callback, the
capture shares a constant 0.063599125 second phase from operation dispatch to
the sampled display clock. This phase is comparison alignment, not a delay
inserted into the control. The knob stays 37 × 24 throughout.

The native trace retires the residual position tail between 0.328305 and
0.344972 seconds, then reports the exact endpoint. The portable implementation
uses one third of a second for this bounded completion. This matches all
retained observations; the exact native cutoff constant remains unexposed.

Touch lift uses the common small-lens geometry response: duration 0.27,
bounce 0.375 and retarget impulse 0.032. Eleven presentation-frame samples
match within 0.0085 points after the observed commit-phase alignment.

Touch lowering uses duration 0.5, bounce 0.3 and retarget impulse 0.016. Its
**presentation bounds** follow this response with a single −0.0012307 second
display-clock phase. Using presentation frame width instead initially suggested
a different spring: the frame also contains the native view-motion affine
deformation while the knob moves to the other endpoint. The bounds response
matches the retained native samples to approximately one millionth of a point
before C++ float conversion. Position, bounds, and view-motion deformation must
be evaluated separately.

The [extractor](../../Demo/SwitchComparison/generate-motion.py) retains 31
programmatic position and 27 lowering-bounds samples in the regression header.
It only extracts and phase-aligns immutable captured observations; it does not
generate observations from the implementation. `NXKitSwitchTests` checks these
samples against the actual control's geometry updates.

Fluid well artwork, drag thresholds, velocity-dependent release and interruption
matrices remain partial. Diagnostic native sampling is not a performance test.
