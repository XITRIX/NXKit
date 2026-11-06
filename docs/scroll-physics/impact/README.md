# Fast swipes into scroll boundaries — 2026-11-04

The reported “wall” was reproducible. The scroll view discarded all outward release velocity whenever the last drag sample was already at/beyond a boundary. A nearly identical swipe released just inside the range kept its momentum. Its collision also reused the stiffer spring calibrated for passive pulls.

Apple describes bouncing as travel [past the content edge and back](https://developer.apple.com/documentation/uikit/uiscrollview/bounces). The numerical target here comes from **13 fresh public UIKit captures on iOS 26.0 (23A343)**, driven by actual XCTest OS contacts. The standalone native reference contains no NXKit physics. The runner uses the existing private XCTest input plumbing; production uses no private UIKit calls.

## Correction

For an outward release already stretched by `d` points, the portable handoff uses:

```text
physical outward velocity = max(0, releaseSpeed − 11 × d)
```

This gives the critically damped edge response a `t·exp(−omega·t)` coefficient equal to `max(omega·d, releaseSpeed)`. A strong fling retains excess momentum; a gentle deep pull remains a passive return. Inward momentum keeps the preceding reversal behavior. A strong fling can retain its cached velocity after a hold, as UIKit's new 40–400ms hold captures demonstrate; an arbitrary velocity-expiry timer would be incorrect.

Free-flight impacts and active outward edge motion use the separately calibrated angular frequency **11 s⁻¹**. Passive returns retain **12.1 s⁻¹**, and inward reversals retain their existing coefficients. These equations approximate observed behavior; they are not a claim about UIKit's private implementation.

The [frequency calibration](calibration.json) follows independently from top/bottom free-flight captures: with exponential deceleration `k`, impact speed is `releaseSpeed − k × distanceToEdge`, and peak stretch is `impactSpeed / (e × omega)`. Those two captures give approximately 11.0 s⁻¹; the two near-edge cases provide additional checks. Release speed is the captured public delegate velocity converted to points/second, not the nominal touch-path speed.

Disabled bouncing still stops at the boundary. Clamping/rebasing a drag after a viewport change also clears its old scroll-release velocity, so a subsequent stationary lift cannot revive the pre-resize swipe. No public API or rubber-band drag-compression coefficient changed.

## Measured behavior

| Case | UIKit peak | Before peak | After peak |
| --- | ---: | ---: | ---: |
| Top, released during free flight | 72.00pt | 65.67pt | 72.00pt |
| Bottom, released during free flight | 66.00pt | 60.00pt | 66.00pt |
| Top, released near the edge | 73.33pt | 66.33pt | 73.00pt |
| Bottom, released near the edge | 84.00pt | 76.67pt | 84.00pt |
| Bottom, already past the edge | 92.67pt | 7.00pt | 92.33pt |
| Top, deeper stretch | 136.00pt | 95.00pt | 132.33pt |

[All 13 overlays](comparison.html) retain raw coordinates/timestamps without fitted registration. Case-balanced full-waveform offset RMS improves **19.62→6.85pt**. The four free-flight/near-edge peaks agree within 0.34pt. Some captures retain late activation/coalescing differences: three outgoing-release positions differ by 16–24pt, with up to 12.67pt residual peak error and up to 11pt full-waveform RMS. These differences remain visible in [metrics](metrics.json); this is not complete UIKit parity.

The original four-edge routed-touch regression fails before the change ([output](regression-before.log)). The final physics and indicator suites pass, including gentle deep pulls, disabled bouncing, exact settling, diagonal/frame-subdivision behavior, geometry changes and reentrant timer replacement. All **14 prior native checks plus 12 reversal checks** still pass. The [13 new impact/hold checks](impact-checks.log) guard waveforms, peak stretch and exact final offsets.

## Evidence and reproduction

Two passed native sessions, exact source snapshots, case configurations, source hashes and test logs are retained under [native/sessions](native/sessions/manifest.json). The first attempt on an existing iOS27 simulator failed to launch its test runner and is excluded. The iOS26 captures used a fresh isolated simulator, removed after collection.

Build `ScrollPhysicsTests`, `ScrollIndicatorTests` and `ScrollReplay` in the headless raster harness, then run:

```sh
ctest --test-dir BUILD_DIRECTORY -R '^(ScrollPhysicsTests|ScrollIndicatorTests)$' --output-on-failure
python3 Demo/ScrollComparison/check-native.py BUILD_DIRECTORY/ScrollReplay
python3 Demo/ScrollComparison/check-reversal.py BUILD_DIRECTORY/ScrollReplay
python3 Demo/ScrollComparison/check-impact.py BUILD_DIRECTORY/ScrollReplay
```

The preceding check scripts require NumPy. Regenerate native captures with `capture-native.py SIMULATOR OUTPUT Demo/ScrollComparison/impact-cases.json`. Replay/overlay generation uses `compare.py NATIVE BEFORE_REPLAY AFTER_REPLAY OUTPUT`; hashes identify the actual before/after executables.

[Build verification](verification.json) records source and artifact hashes for the macOS and unsigned iOS Demo. This is CPU motion code; the Metal source fingerprint and offline bundle are unchanged. No shader re-harvest or GPU timing/thermal claim is made. Physical-device feel, other UIKit releases and exact private physics remain unverified.
