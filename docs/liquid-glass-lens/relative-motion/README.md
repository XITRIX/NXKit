# Lens motion activation — 2026-11-07

The proposed superview-relative motion rule does **not** match the observed
UIKit implementation. The actual discrepancy is activation at rest.

An authored native app instantiates real `_UILiquidLensView` objects in both
small/large styles. Nine independent cases isolate stationary placement, lens
position, parent position, grandparent position, parent transform, scroll
content offset, parent bounds origin, lens position animation, and ancestor
position animation. The fixture moves the UIKit views/layers; it never assigns
an elastic transform or injects integrator values. A display link samples the
native presentation transform and read-only flex/integrator getters.

| State | Own movement | Ancestor/scroll movement | Native resolved activation |
| --- | --- | --- | --- |
| Initially resting | Identity elastic transform | Identity elastic transform | 1 |
| Lifted | Elastic response | Same response for the same displacement | 3 |
| Lowering while still moving | Continues responding | Continues responding | 3 until settling |

Resting results are zero affine error for all 18 style/movement combinations
on both iPadOS 27.0 at 2× and iOS 27.0 at 3×. Lifted cases on iPadOS 27.0 show
matching own/parent/grandparent/translated-parent responses: maximum translation
component approximately 2.71pt for small and 4.42pt for large. Lens and ancestor
Core Animation position runs also agree (approximately .502pt / .824pt). These
are stimulus-specific outputs, not universal constants or fitted coefficients.
The velocity-integrator position varies in global coordinates in the active
cases. UIKit therefore does not discard ancestor motion from active flex.
Positive parent bounds-origin movement produces the opposite global movement:
its signed x drift mirrors lens translation while directionless strain agrees.
`analyze.py` verifies those full-frame series, rather than comparing only extrema.

In the separate lowering run, the public private-API `lifted` getter changes
to false at 1.518s, while resolved flex activation stays 3 until 2.784s. Movement
ends at 2.2s. This supports retaining an elastic settling phase instead of
switching off flex immediately on release. It does not recover UIKit's exact
pending-unlift timer or activation algorithm.

NXKit previously initialized motion sampling and woke its display link from
every lens frame/bounds assignment, including unlifted control layout. It
also kept a stale global position for a later lift. `viewMotion` now samples
only while lifted or while material/elastic state is settling. Fully resting
geometry changes do not wake it; settling clears its motion history before a
later lift rebases at the current presentation pose. Contact-driven explicit
interaction remains a documented portable extension. No superview-relative
coordinate substitution is made.

## Evidence and reproduction

All successful captures exit 0. REA's process capture is marked truncated;
the complete result summaries and compressed frame traces are retained
separately, with source/result SHA-256 hashes. Read-only getters are checked
against runtime signatures. `rea-*.json` retain compact terminal capture and
provenance, rather than duplicated terminal redraws/process samples.

- Rest 2×: `ev_dc36e4ba3f318f2c0c09527dddd140d208359bec38ea73c419f33db8da4e1d0b`.
- Rest 3×: `ev_d28c2615c0dda78605103a6e4f0068e0fefeb5c6ec366e68bb1953da857d9891`.
- Lifted 2×: `ev_55a194f2afec4dd929a3cb7f67e6e3026738384511b811c94915ee512ecb8784`.
- Lowering 2×: `ev_aef03bf741af42ad95341000fb94d7bf0bf4965e7b8e110e8f0c534f8bb13a04`.

```sh
python3 Demo/GlassComparison/capture-lens-relative-motion.py <simulator-uuid> /tmp/lens-rest resting
python3 Demo/GlassComparison/capture-lens-relative-motion.py <simulator-uuid> /tmp/lens-lift lifted
python3 Demo/GlassComparison/capture-lens-relative-motion.py <simulator-uuid> /tmp/lens-lower lowering
```

Coverage is limited to these iOS/iPadOS 27.0 Simulator builds and authored
native lenses. It does not prove physical-device, other OS, or every control
provider's activation policy. Diagnostic sampling is not a performance test.
The existing real-touch native control traces remain separate evidence.

Portable regressions cover both styles, light/dark, 1×/2×/3×, own/ancestor
movement, parent transform/bounds, scrolling, own/ancestor presentation
animation, lowering, reactivation and stale-history prevention (96 cases),
including immediate lowering/re-lifting without an intervening display tick.

## Verification

All 96 portable activation cases and the slider/switch behavior suites pass.
The final 2/3/4/5-tab motion and control-gallery replays pass from private cold
caches with zero library/pipeline compilations during rendering. Simulator
pipeline-object preparation during context creation is outside those counters.
Simulator and iPhone RelWithDebInfo app builds pass with matching offline shader
packages (ABI `4d43f8ae5bf6cce79f9f99d16f4466ed8d908cea85d3c8927663af2033e3b4db`).
The signed app was installed and launched on iPhone Pro XITRIX. That installation
is not an additional native-UIKit measurement or a physical frame/thermal test.
Logs and package checks are retained in `verification/`.

The later [finite-response policy](../finite-response/README.md) adds the
user-requested two-second wobble fade even during ongoing movement. The native
observations above remain unchanged; that timeout is an intentional divergence.
