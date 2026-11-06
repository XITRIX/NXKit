# Pop-gesture title cleanup — 2026-11-04

The supplied recording shows “Separated” reappearing over “Accent” after the interactive pop has finished. Button-driven pops do not use this failing handoff.

The gesture retains its title slide on the shared content clock. When the half-second settlement ends, the controller discards those animations. The outgoing title carrier remained attached until the independent 0.97-second glass retirement finished; its model transform was still its original position. Removing the slide therefore exposed the old title again during that gap.

The committed bar handoff now records a weak reference to that retiring title. Interactive completion detaches it immediately, while glass continues its optical tail. Cancellation keeps the source title and removes the destination preview. Subsequent bar refreshes clear the reference when superseding the previous transition. Ordinary push/pop animation and application title-view transforms retain their existing behavior.

## Regression

The routed-touch regression previously checked 60 release frames at 120Hz, then forced all pending animations to finish. That skipped the natural completion gap. It now checks **145 frames per case**, including the half-second boundary and the following material cleanup, without forcing completion during sampling.

Across 120 cases this is **17,400 frames**: 7,200 release frames and 10,200 completion/tail frames. Cases cover light/dark, 1×/2×/3×, ordinary/source-custom/destination-custom/both-custom titles, paused releases, forward flicks and reversed cancellations. Post-completion assertions check the chosen controller, obsolete carrier detachment, destination carrier ownership and stationary title position. The earlier title/content attachment and reveal-boundary checks remain.

[Before output](before-tests.log.gz) contains **4,104 failures** of obsolete title detachment, reproducing the gap. The other post-completion assertions pass. Both `InteractiveNavigationTests` and `UIKitNavigationTests` pass after the fix ([test output](tests.log)).

The retained images show the actual bar at 0.5, 0.6, 0.8 and 1.0 seconds after committed release. Only “Target” remains; these are portable raster captures, not new native UIKit captures. Set `NXKIT_NAVIGATION_POP_COMPLETION_PICTURES` when running the interactive test to regenerate them.

![Settled title in dark mode](images/dark-72.png)

## Builds and GPU checks

[Verification](verification.json) records frozen source hashes, source snapshots, build logs and exact embedded bundle hashes. Fresh ordinary push/pop smoke captures supply current raw shader states; prior exact raw edge/private-effect states retain provenance. The current offline bundles are freshly baked for the source fingerprint.

The strict Metal sweep covers all eight interactive arrangements, commit/cancel, their full cleanup tails and both dark 3×/light 2× profiles. It requires archive hits and zero runtime Metal library/pipeline compilation. macOS and unsigned iOS Demo builds include matching bundles. [run-metal.py](run-metal.py) provides `cold` and `strict` modes with explicit local paths; merge cold states using `../../scroll-edge-effects/scroll-pocket/merge_catalog.py --navigation-only --navigation-smoke`, configure both ordinary projects, build them, then run strict verification.

No new physical-device or native UIKit run was performed. The supplied recording and retained UIKit references establish the behavioral target; absolute native spring timing and optical parity remain outside this fix. No performance/thermal comparison is claimed.
