# Tab lens transfer against UIKit

The Demo tab lens now lifts on contact, travels as an enlarged clear lens,
then lowers near the selected tab. A quick first-to-fourth tap no longer
starts lowering on touch-up while the lens is crossing the bar. This is a
Demo composition using `LensView`, not a new public `UITabBar` implementation.

Apple describes temporary elevation into a transparent lens during control
interaction in [Meet Liquid Glass](https://developer.apple.com/videos/play/wwdc2025/219/).
The timings below come from actual `UITabBar` behavior, rather than from that
presentation or a guessed animation duration.

## Native observations

Six independent OS-touch cases ran on an iPhone 17 Pro simulator, iOS 27.0
(24A434), 3×: first→fourth, fourth→first, adjacent, reselect, dark first→fourth,
and an 817 ms held fourth-tab press. The fixture leaves UIKit in control of
selection geometry and movement. Public XCTest tap/press APIs deliver input.
A passive window observer records actual touch timestamps; a display link
records the model and presentation layer tree. Native-only research code
identifies the lens by its class name; production has no private UIKit calls.

The phone platter is 360×62 points; the resting lens is 95×54. Contact raises it
to 111×70 (eight points on every edge), with separate selected-ink enlargement
and motion-driven elasticity. Its center moves horizontally; the floating
appearance comes from enlargement, clear optics, exterior contour/shadow and
elastic deformation, rather than translating the center upward.

| Input | Actual touch duration | Native lowering starts after touch start | Distance from destination when lowering starts |
|---|---:|---:|---:|
| First→fourth | 66.7 ms | 329.5 ms | 7.51 pt |
| Fourth→first | 66.7 ms | 314.5 ms | 6.05 pt |
| Adjacent | 66.7 ms | 265.5 ms | 7.59 pt |
| Reselect | 66.7 ms | 267.3 ms | 0 pt |
| Dark first→fourth | 66.7 ms | 315.7 ms | within 8 pt |
| Held fourth tab | 816.7 ms | 833.1 ms | already arrived |

This supports a minimum raised interval of about 200 ms from activation,
followed by lowering once contact has ended and the center is within roughly
eight points of its target. This is an empirical policy, not a documented
private UIKit algorithm. A held press stays raised after arriving and lowers
on release, without an additional minimum interval.

## Implementation and comparison

`LensTabBarView` keeps contact, committed selection, travel and lift separate.
It preserves the approach animation when the ordinary tap action commits its
selection, instead of restarting it. A short-lived display link checks the
minimum interval and presented distance, then releases itself on lowering,
cancellation, hiding or hierarchy changes. It uses weak ownership. A finger
can catch a travelling lens without applying its elastic drift a second time.
Nonanimated selection cancels the automatic lift pulse.

The center curve is an independent damped-step approximation (damping .85,
angular frequency 15.8/s, exact end at .6 s). Small overshoot is intentional.
The existing lift geometry, selected-item scaling, optics, shadow and elastic
motion remain separate from this placement animation.

The [raw comparison](native/comparison.json) retains all six native layer trees
as gzip JSON, focused CSVs and actual NXKit routed-touch samples. Comparing
center trajectories from .1–.7 s, after a per-case temporal alignment:

| Transfer | Center RMS error | Aligned lowering difference |
|---|---:|---:|
| First→fourth | 0.36 pt | −0.2 ms |
| Fourth→first | 0.30 pt | −6.2 ms |
| Adjacent | 0.12 pt | −2.2 ms |
| Dark first→fourth | 0.31 pt | −6.4 ms |

Position fits use 50–71 ms alignment offsets. These absorb native transaction,
dispatch and sampling phase; they are **not** an end-to-end touch-latency score.
Stationary reselect uses observed lift activation for alignment and lowers
8.3 ms later than native. Portable traces advance at 120 Hz; real display-link
ordering can introduce another frame of phase difference.

Separate light/dark screenshot runs confirm the raised lens composition, but
snapshot rendering takes about 60–80 ms and perturbs delivery. Their raw
snapshot start/end times are retained in `native/snapshots-*.json.gz`; these
runs are excluded from timing fits. The illustrative cropped images below
show different in-flight moments, not synchronized pixel comparisons.

Native light / NXKit light:

![UIKit in flight](native/in-flight-light.png)
![NXKit in flight](portable/in-flight-light.png)

Native dark / NXKit dark:

![UIKit in flight, dark](native/in-flight-dark.png)
![NXKit in flight, dark](portable/in-flight-dark.png)

Exact optical and elastic parity is still incomplete. The retained portable
long-jump trace reaches horizontal scale 1.25 versus native 1.161; its vertical
compression and return also differ. This patch corrects the lift/travel/lower
sequence and position curve; it does not claim to reconstruct Apple's full
private lens physics or shaders. Physical-device timing, reduced-motion
behavior and GPU pixel parity remain unverified.

## Verification and reproduction

Five raster regressions pass: `LensTabTests`, `LensTests`, `LensMotionTests`,
`LensRefractionTests` and `GlassScreenTests`. After the in-flight catch fix,
the two affected suites were rebuilt and passed again. Routed touches cover
first/fourth, reverse, adjacent, reselect, held press, travel catch, cancellation,
rapid retarget, nonanimated interruption and detachment, including exact final
geometry, single action delivery, settled deformation and pulse disposal.
The iOS Debug Demo builds and links with the current device Metal archives.

```sh
python3 Demo/GlassComparison/capture-lens-motion.py DEVICE /tmp/tab-native docs/liquid-glass-lens/tab-transfer/cases.json
cmake --build /tmp/nxkit-container/raster-tests --target LensTabTests TabTransferRenderer
mkdir -p /tmp/tab-portable
/tmp/nxkit-container/raster-tests/LensTabTests /tmp/tab-portable
python3 Demo/GlassComparison/analyze-tab-transfer.py /tmp/tab-native /tmp/tab-portable /tmp/tab-comparison
/tmp/nxkit-container/raster-tests/TabTransferRenderer /tmp/tab-portable/light 0
/tmp/nxkit-container/raster-tests/TabTransferRenderer /tmp/tab-portable/dark 1
```

The raster build path is the local configured comparison build, not a required
repository path. Its CMake source is `Demo/GlassComparison/RasterTests` and it
requires matching Skia raster headers/archives. Native capture manifests
retain the exact configurations and fixture source hashes; the timing fixture
sources are archived beside them. Runtime logs remain in
`/private/tmp/nxkit-tab-transfer`.
