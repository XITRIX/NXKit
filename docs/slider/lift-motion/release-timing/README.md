# Lowering during slider value retirement

2026-11-06, working tree after `bd143cc`.

The former release path waited until its 400ms projected-value animation ended
before lowering the knob. Native UIKit starts the return toward resting geometry
while value movement continues. The existing small-lens geometry/fill/optical
springs already represented that return; their activation was delayed incorrectly.

`deferEndTracking` now starts lowering at finger-up, preserving the minimum lifted
interval for short contacts. Value movement and terminal event delivery remain
separate: tracking stays active until the projected value settles, then sends one
touch-up followed by the final valueChanged event. Reaching that terminal point
does not restart lowering. Cancellation still clears pending minimum-lift holds.

![Native and portable release curves](comparison.png)

31 independent native presentation samples from the retained real OS-touch
velocity-200 and velocity-400 captures are replayed at both 60 and 120Hz. Maximum
width residual is 0.286pt after the previously measured 16.5ms release commit
alignment. This is phase-aligned curve agreement, not input-to-display latency.
The touch streams include native stationary holds that preserve the last moved
velocity; no synthetic UIKit touch objects were used for native capture.

Inputs remain under `docs/slider/native/velocity/`; generated C++ expectations
retain SHA256 of each native frame/touch file. The timing tests also assert that
visible size does not jump at finger-up, lowering overlaps changing values, and
size is nearly resting before deferred tracking completes. Existing stationary,
short-tap, cancellation and event-order checks remain in the slider target.

```sh
/usr/bin/python3 Demo/SliderComparison/generate-release-timing.py --check
xcrun simctl spawn SIMULATOR /absolute/path/NXKitSliderTests.app/NXKitSliderTests --release-timing /tmp/release
/usr/bin/python3 Demo/SliderComparison/compare-release-timing.py /tmp/release
```
