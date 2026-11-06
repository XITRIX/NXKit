# Pull beyond an edge, then reverse into the content

The subsequent [fast-impact follow-up](../impact/README.md) retains this inward handoff and calibrates a separate outward/impact response. The measurements below describe the preceding implementation.

Measured 2026-11-01, working tree after `042ac0c`.

[Native UIKit / NXKit overlays](comparison.html) · [results](metrics.json) · [capture provenance](native/sessions/manifest.json) · [calibration](calibration.json)

The preceding implementation kept an inward release in the boundary-return spring after it crossed back into the scrollable range. That spring eventually brought the content back to its old target. It also multiplied inward release velocity by the outward rubber-band compression derivative, losing too much momentum before crossing.

The correction preserves inward release velocity and solves the elastic boundary crossing. At the crossing, it transfers both position and velocity to ordinary free deceleration and advances the remainder of the current frame. The old boundary no longer attracts content which has already escaped into the scrollable range. Outward releases and slow passive returns retain the previous behavior. Each axis has its own handoff.

## Native comparison

Twelve additional scenarios run Apple's actual `UIScrollView` on an iPhone 17 Pro simulator with iOS 27.0. They cover top and bottom edges, shallow/deep overscroll, slower returns, denser reverse paths, and releases which have already crossed into the content. Three XCTest capture batches passed. Exact sources, case configurations and source hashes are retained under `native/`; the test runner's private XCTest input plumbing is unchanged from the original audit. Production has no private UIKit calls.

The before executable was frozen immediately before this correction. It includes the preceding pan/scroll improvements, so its baseline differs from the original report's pre-audit executable. Raw native touches and both timestamps are replayed without spatial registration or per-case offset correction. Native state, before/after replay data and executable hashes are retained.

Offset RMS over all display frames, in points:

| Release | Before | After |
| --- | ---: | ---: |
| Top, still overscrolled | 330.657 | 1.647 |
| Bottom, still overscrolled | 92.867 | 1.299 |
| Top, deeper pull | 24.881 | 3.778 |
| Bottom, deeper pull | 24.883 | 3.790 |
| Bottom, dense deeper pull | 12.359 | 4.790 |
| Top, dense deeper pull | 82.049 | 10.409 |

All fast reversal cases now finish inside the content, including deeper pulls. Slow returns finish at the boundary. Already-crossed releases remain unchanged. The top dense/deep case retains a roughly 16.6-point pan activation difference from UIKit; its offset error remains visible in the report.

## Elastic approximation

The passive return remains critically damped with angular frequency 12.1 s⁻¹. For an inward fling, the portable approximation uses an angular frequency of 13.2 s⁻¹ and base damping 15.5 s⁻¹. Damping increases with remaining stretch relative to inward momentum:

```text
ratio = omega * abs(displacement) / abs(releaseVelocity)
damping = min(2 * omega * 0.999, 15.5 + 4.25 * max(0, ratio - 0.75))
```

The shallow top/bottom captures calibrate the base coefficients. Top deep and top dense/deep captures calibrate the remaining-stretch adjustment; bottom deep/dense, denser shallow paths, slow returns and already-crossed releases provide separate validation. The calibration script fits native post-crossing free motion to estimate boundary velocity; it does not shift the captured positions. These equations approximate the observed behavior and do not claim to reproduce UIKit's private implementation.

Closed-form spring roots provide the exact crossing time for the chosen model. Both damping regimes preserve crossing velocity and consume the same remaining time, so the result does not depend on frame subdivision. A subsequent collision at the opposite boundary uses the normal passive spring.

## Verify

With the headless `ScrollReplay` and `ScrollPhysicsTests` targets built:

```sh
ctest --test-dir BUILD_DIRECTORY -R ScrollPhysicsTests --output-on-failure
python3 Demo/ScrollComparison/check-reversal.py BUILD_DIRECTORY/ScrollReplay
python3 Demo/ScrollComparison/check-native.py BUILD_DIRECTORY/ScrollReplay
```

NumPy is required for the Python checks. Regenerate the calibration and report with:

```sh
python3 Demo/ScrollComparison/calibrate-reversal.py docs/scroll-physics/reversal/native docs/scroll-physics/reversal/calibration.json
python3 Demo/ScrollComparison/compare.py docs/scroll-physics/reversal/native BEFORE_REVERSAL_REPLAY AFTER_REPLAY docs/scroll-physics/reversal
```

Focused routed-touch regression tests reproduce the original failure at both edges and verify continued inward motion after release. Axis-level checks verify the handoff's position and velocity under different frame subdivisions. The existing gesture, indicator, full-screen glass, lens-tab and startup-scale tests also run. Physical-device feel and exact private momentum estimation remain unverified.
