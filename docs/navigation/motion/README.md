# Navigation glass motion — 2026-11-03

Historical calibration audit: the [complete playground audit](../cycle/README.md)
supersedes these motion scores for the current implementation. It uses fresh
containers for every case and covers Back, titles, content and all eight Demo
arrangements, including different two-item and three-item split behavior.

The navigation bar now uses overlapping SDF participants for glass separation
and absorption, a fitted spring for capsule geometry, a separate inflation and
undershoot pulse, and a foreground-only Gaussian blur for entering/leaving ink.
All presentation code remains private to UIKit; Core item metadata is unchanged.

The subsequent [spacing correction](spacing/README.md) reduces the navigation
container's blend distance from 20 to 12 points. Both implementations already
had the same 12-point layout gap. The larger kernel kept a neck alive too long
and pulled the facing contours outward even after they disconnected. The
updated comparisons below use the corrected blend distance.

## Native observations

The public UIKit fixture captures ten transitions in both themes on the iPhone
17 Pro simulator, iOS 27.0: shared → split, split → shared, grow, shrink,
matched-identifier symbol replacement, unmatched replacement, image → text,
plain → prominent, and reverse-stack split/join. The two replacement cases
matter: UIKit updates the matched identifier's image immediately; an unmatched
item receives a blurred reveal and a material pulse.

During splitting, the original capsule stays full-sized initially. The new
surface begins at approximately 20% scale within the source silhouette, then
grows and moves through it. The union develops a narrow neck before separating.
The retained surface inflates from 44 points to roughly 53 points high, then
undershoots before settling. During joining, redundant material collapses into
the receiving surface early, before the receiver expands and reveals the moved
item. These observations refine the overlap technique described in Apple's
[WWDC navigation and toolbar session](https://developer.apple.com/videos/play/wwdc2025/284/).

The first un-warmed native push stalled for 329 ms. Its trace and initial frames
are retained under [native-cold-start](native-cold-start). The measured matrix
uses a real warm-up push/pop before capture. No time shift is fitted to a case.
Capture rejects native sampling gaps greater than 100 ms during the measured
interval. A separate [compositor recording](native-video-validation/join-compositor.mp4)
confirms the displayed join sequence; its lower frame rate is not used to fit
spring parameters. Private class names and layer descriptions are read-only
observations, with no private API invocation.

## Implementation

- Retain source geometry while splitting, and allow nearby surfaces to form a
  smooth SDF union. The blend reach is bounded by the 12-point resting gap, so
  separated capsules keep independent round contours.
- Animate bounds, position and item layout on a common delayed spring. A second
  pulse supplies the material's inflation and rebound; a faster birth spring
  supplies the new capsule's growth and directional overshoot.
- Move redundant neutral material into the receiving capsule and remove it
  when fully covered. Keeping an old 44-point circle alive through the receiver's
  undershoot would incorrectly flatten the rebound. Tinted retiring material
  also fades its old tint during absorption.
- Blur and fade only foreground ink. Its capsule, siblings, hit testing and
  custom controls remain independent of the filter. Matching identifiers update
  images/text in place; newly created or reparented items get the reveal.
- Give retirement its own animation clock. Replacing implicit geometry with
  explicit spring animations cannot prematurely run a cleanup completion.
  Retargeting removes superseded participants and starts at presented geometry.
  Controls accept input during the remaining optical tail of a completed push.

## Frame comparisons

Each filmstrip reads **UIKit / before / after**, with actual capture timestamps.
Animated previews include endpoint holds; their nearest-frame sampling is for
inspection, while numeric geometry uses the fixed grid described below.

![Split comparison](split-dark.gif)

![Join comparison](join-dark.gif)

[Split frames](split-dark.png) · [Join frames](join-dark.png)
· [Grow](grow-dark.png) · [Shrink](shrink-dark.png)
· [Text](text-dark.png) · [Accent](accent-dark.png)
· [Pop split](pop-split-dark.png) · [Pop join](pop-join-dark.png)

Matched versus unmatched replacement:
[native matched](native/replace-dark.webp),
[native unmatched](native/replace-unmatched-dark.webp),
[portable matched](after/replace-dark.webp),
[portable unmatched](after/replace-unmatched-dark.webp).

The [machine-readable comparison](comparison.json) reports every case, including
the two unmatched replacement clips added after the baseline capture. Native
geometry is interpolated at 60 Hz over the first 650 ms, producing 40 samples
per clip. Rectangular RMS error uses x, y, width and height of the retained
trailing capsule. The dark silhouette score compares thresholded whole trailing
regions without image registration, geometric normalization or color fitting.
It measures shape/timing, rather than material color accuracy. Connectivity
agreement is reported both over all dark clips and over the six join/split cases.

Across the 18 cases present in both the earlier prototype and the current
implementation (720 geometry samples each):

| Measurement | Before | After |
| --- | ---: | ---: |
| Retained capsule rectangle RMS error | 10.30 pt | 1.33 pt |
| Mean dark silhouette IoU (360 rasters each) | 81.4% | 96.5% |
| Connectivity agreement, six join/split cases | 68.8% | 98.3% |

Exact native pixel parity is not claimed. Remaining differences include the
native birth capsule's asymmetric stretch, residual phase/shape differences,
platform symbol raster metrics, and some tint/blur profiles. Captures cover the
reference phone and inline bar, with automatic Back; arbitrary custom leading
arrangements, interactive pop, large titles, RTL and tablet layouts need their
own motion audits. Physical-device execution is not established by the builds.

## Verification and reproduction

`UIKitNavigationTests` checks an actual rendered neck and its eventual gap,
surface inflation/undershoot and final transform cleanup, immediate matched
replacement, blurred unmatched replacement, foreground raster softening with
unchanged sibling pixels, input during the spring tail, and interrupted
retirement. It also checks detachment at the one-third-second native checkpoint
and resting contours against the same capsules with merging disabled, within
one pixel at 3×. Existing tests retain custom-control actions, contact affines,
disabled/cancelled presses, identifier reorder and containment coverage.

The navigation suite and seven focused Core/layout/Demo/tab/gesture/container
suites pass: [regression output](regressions.txt). macOS Metal Demo and unsigned
iOS Debug Demo compile and link; hashes are recorded in
[`apple-build.json`](../../skia-upgrade/apple-build.json).

```sh
python3 Demo/NavigationComparison/build-motion.py /tmp/navigation-motion-native
xcodebuild -project /tmp/navigation-motion-native/Interaction.xcodeproj \
  -scheme Interaction -sdk iphonesimulator \
  -destination 'generic/platform=iOS Simulator' \
  -derivedDataPath /tmp/navigation-motion-native/build \
  CODE_SIGNING_ALLOWED=NO build
```

Install and launch `InteractionReference.app` with bundle identifier
`org.nxkit.NavigationMotionReference` on an already booted simulator. Documents
receives `motion.json` and relative PNGs after roughly 45 seconds. Copy the entire
directory after JSON appears; its write follows all asynchronous PNG writes.
`--case split` restricts capture to that case in both themes.

Build the raster `NavigationMotionRenderer` target using the matching Skia
archives, as documented in `Demo/GlassComparison/RasterTests`. Its arguments are
an output directory and an optional case name; it writes deterministic 60 Hz
frames. The initial source screenshot precedes the animation request in both
fixtures. To reproduce the analysis with before/after output directories:

```sh
python3 Demo/NavigationComparison/compare-motion.py \
  /tmp/native /tmp/before /tmp/after /tmp/comparison
```

Compressed native/before/after traces, source hashes and selected lossless
replacement frames accompany this report. Full-resolution raw PNGs are generated
by the fixtures; the repository retains the filmstrips and animated comparisons.
In Demo, open **Full-screen iOS layout → Navigation → Navigation morphing**;
use Push next, the arrangement links and Back to compare different transitions.
