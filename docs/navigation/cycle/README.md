# Complete navigation playground comparison — 2026-11-03

The native UIKit fixture now reproduces the Demo's eight navigation-item
arrangements, four retained tab/navigation stacks, direct arrangement links,
Push next, Pop to collection and item tap feedback. Launching without capture
arguments leaves it interactive. The native table is a small fixture; its root
body is not a pixel reproduction of the Demo's settings/collection content.
The bar item metadata, images, identifiers, ordering and stack operations match.

`CycleRenderer.cpp` exercises the **actual production Demo controllers**. It
does not maintain another portable implementation of their arrangements.

## Coverage and capture protocol

- 288 independent clips: all 8×8 arrangement pairs, push and pop, light and dark,
  plus each arrangement's root entry/exit. Equal arrangements use distinct
  controller instances, so they still exercise navigation titles and content.
- Native iPhone 17 Pro, iOS 27.0, 402×874 points, scale 3; portable raster context
  uses the same size, scale and 62/34-point device insets.
- Every native case gets fresh tab/navigation containers, with its initial
  stack built before window attachment. A real warm-up push/pop precedes the
  matrix. Reusing a bar after a nonanimated reset can leave UIKit material
  animation running and produced a misleading initial state in an early probe.
- Geometry records actual CADisplayLink times. Portable geometry runs at 120 Hz.
  Each matrix clip has ten full-bar screenshots through 750 ms. They include
  moving content beneath the bar. The portable display tree refreshes every
  frame, including ink that was hidden during attachment.
- No per-case onset alignment, time offsets, image registration or pixel
  interpolation is applied to the comparisons. Production uses one globally
  calibrated content onset/clock. Native matrix sampling gaps are at most 66.7 ms.
- A second, denser pass captures every available native rendered frame of the
  normal eight-layout push/Back sequence in both themes (32 clips), and every
  120-Hz portable frame. Frame timestamps, not the display-link's requested
  maximum rate, describe its actual cadence.

All 288 pairs have native/after contact sheets named `matrix-*.png`. Each row
pair is native then NXKit; columns carry actual screenshot timestamps.
[comparison.json](comparison.json) contains a result for every pair, alongside
aggregate and leading-change results. Compressed native/before/after traces
retain the full geometry. Before matrix scores use geometry only: its first
raster export skipped display updates. The before pictures shown in selected
filmstrips were recaptured with the corrected renderer before production changes;
the original before rasters are not used for visual scoring. [provenance.json](provenance.json) records source and
build hashes; the original native matrix build manifest is retained separately.
The 254 clips outside the text-button target retain their original capture;
all 34 text-target clips were refreshed after correcting the retained-ink
replacement rule. That removed branch does not execute in the other clips.
The original captures also precede a one-line restoration of content input
blocking, which changes no geometry, duration or rendering. Per-stage source
hashes are in `capture-source-stages.json`; final builds and regressions include
both corrections.

The small GIFs present screenshot checkpoints at approximately half speed,
with a hold at the endpoint. They are review aids, not continuous compositor
recordings. The dense comparison movies keep a common request-relative clock
and use the nearest actually captured snapshot, showing the timestamp of each
displayed source sample. They do not interpolate pixels or reconstruct missing
compositor frames.

## Measured results

| Measurement | Before | After |
| --- | ---: | ---: |
| Back rectangle RMS, 60 changing-leading clips | 2.81 pt | 0.66 pt |
| Opacity-weighted star rectangle RMS, same 60 clips | 26.15 pt | 3.05 pt |
| Content horizontal RMS, all 288 clips | 45.10 pt | 5.55 pt |
| Right rectangle RMS where material counts agree | 4.13 pt | 2.41 pt |

Mean per-clip right material-count agreement is 98.0%; unmatched counts
remain a reported difference, rather than contributing to the conditional
rectangle score. The 200 unchanged-leading clips keep Back stationary in both
implementations.

The dense 32-clip cycle contains **1,090 native** and
**2,912 portable** frames. Its maximum native sampling gap is
57.5 ms. Back rectangle RMS is 0.47 pt, opacity-weighted star
rectangle RMS is 2.22 pt, and content horizontal RMS is
4.71 pt. These measured subsets are different from the older 20-clip audit.

[Light cycle movie](cycle-light.mp4) · [Dark cycle movie](cycle-dark.mp4) ·
[Leading split](push-4-5-light.gif) · [Leading absorption](pop-5-4-light.gif)

## Differences found and implemented

1. **Unchanged Back stays still.** An unchanged supplemental star also stays
   still. Navigation alone does not invent an optical pulse for either.
2. **Adding a supplemental item is phased.** Back first inflates about its top
   leading edge, shifts slightly, and softens its ink. The new material appears
   later near Back, then travels and grows into its own position. Removing it
   uses a longer absorption phase than an ordinary trailing-group merge.
3. **Root boundaries differ.** A custom root chevron becoming automatic Back
   allows the supplemental item to appear earlier; returning to that custom
   root item delays supplemental absorption. These are separate from retained
   automatic-Back transitions.
4. **Titles travel with content.** The incoming title rides the incoming slide;
   the outgoing title follows the 30% parallax. Their visibility follows the
   same moving boundary, rather than a centered cross-fade. This also applies
   when both controllers have identical title strings.
5. **Content departs quickly and settles longer.** A shared near-critically
   damped clock replaces the previous 350-ms ease-in/out. Completion still owns
   containment, balanced appearance/delegate delivery and queued operations.
   Container-owned hosts preserve application transforms. The rear content
   receives a small dimming overlay that is removed on completion.
6. **Three trailing items use leading title alignment**, including when Back
   is present.
7. **Glass identity follows the surviving visual-leading member.** Removing
   that member falls back to the first surviving item in array order. A new
   capsule then starts near that receiver before separating. A two-item split
   with no removed members first unwraps the retained capsule to its own width;
   its inflation stays anchored at the top trailing edge. Treating all these
   cases as the same center-based resize was visibly wrong.
8. **Ink replacement follows carrier ownership.** A retained matched item can
   change from image to text immediately while the text moving into another
   capsule receives the blurred reveal. A blanket image-to-text fade obscured
   UIKit's retained item and was removed after frame review. Matched
   image-to-image replacement remains immediate. Glass, icons and text remain
   in the same carrier hierarchy.

All changes are private UIKit presentation behavior. The public C++ navigation
APIs and Core item metadata are unchanged. The navigation glass blend spacing
remains 12 points, matching the resting gap.

## Verification and remaining differences

The regression harness checks Back/star phasing, root boundaries, shared
title/content timing, title wiping, fast content departure, three-item carrier
identity, actual SDF joining, the quarter-second two-item gap, resting round
contours, blur isolation, action delivery, interrupted cleanup and model
transform preservation. Container, tab, module composition and Demo suites run
alongside it. macOS and unsigned iOS Demo builds compile and link.

Rectangle scores are presentation-geometry measurements, not optical parity.
Back and star are scored separately; star rectangle error is opacity-weighted.
An absent star is a zero-size point at Back with zero opacity, so the score
penalizes visibility-phase mismatch as well as travel. Matrix aggregates weight
clips equally.
Right rectangles are scored only when material counts agree. Transitions
involving hidden-background hosts are excluded from that right-material score:
UIKit retains a host even where no material is drawn. Their screenshots and
full traces are still retained and reviewed. Title geometry inside UIKit can
omit private sublayer transforms/clipping, so title motion is checked through
rendered captures and the shared content trajectory, not a misleading UILabel
frame score.

### Navigation ink color follow-up

The 2026-11-03 follow-up fixes the inherited accent on ordinary navigation items:
image items, text items, supplemental leading items and automatic Back now use
dynamic `UIColor::label`. An explicit item tint overrides ordinary ink, and
clearing it restores label ink. Prominent items retain white ink and an accent
glass fill, selected from their item tint or the inherited bar tint. Custom views
retain their own content styling.

![Native UIKit, previous NXKit and corrected NXKit in both themes](label-ink/comparison.png)

These six static comparisons use the existing native iOS 27.0 oracle and fresh
production Demo renders: source text buttons, settled Back plus supplemental
star, and settled prominent Add. The native fixture explicitly sets a blue bar
tint. The previous column uses the last pre-color-fix production capture. Each
column selects its first or final image from capture metadata, rather than
assuming dense and checkpoint captures share filenames. Original full-resolution
PNGs, actual sample times and SHA-256 hashes are in
[`label-ink/sources.json`](label-ink/sources.json). Other material/color differences
remain visible; this validates ordinary ink, not complete optical parity.

Reproduce the corrected captures with the existing renderer:

```sh
NavigationCycleRenderer /tmp/navigation-label-leading push-4-5-
NavigationCycleRenderer /tmp/navigation-label-prominent push--1-1-
```

`UIKitNavigationTests` verifies ordinary ink independently from ancestor/bar tint,
live explicit item tint and clearing, plain/prominent style changes, neutral Back
alongside tinted siblings, prominent fill/white ink, and theme resolution at
1×/2×/3×. The suite and macOS/unsigned iOS Demo builds pass. The earlier animation
movies, matrix images and their source manifests are preserved as historical
pre-color-fix evidence.

Residual capsule phase/anisotropy, ink blur, rim and material differences remain.
The native modern items use label-colored ink in this fixture despite the bar's
blue tint. The original portable captures below still have inherited blue ink;
the subsequent [color follow-up](#navigation-ink-color-follow-up) corrects that default.
Full appearance customization, full pixel
parity, physical-device GPU rendering, interactive pop/cancellation, RTL,
Dynamic Type and reduced-motion behavior are not established here. These APIs
remain Partial in the compatibility tracker. The previous smaller
[motion audit](../motion/README.md) is historical calibration evidence; this
fresh-container matrix supersedes it for current motion claims.

## Reproduce

From the repository root, with an available iOS 27 simulator:

```sh
python3 Demo/NavigationComparison/build-cycle.py /private/tmp/navigation-cycle
xcodebuild -project /private/tmp/navigation-cycle/Interaction.xcodeproj \
  -scheme Interaction -configuration Debug \
  -destination 'generic/platform=iOS Simulator' \
  -derivedDataPath /private/tmp/navigation-cycle/build CODE_SIGNING_ALLOWED=NO build
xcrun simctl install DEVICE /private/tmp/navigation-cycle/build/Build/Products/Debug-iphonesimulator/InteractionReference.app
xcrun simctl launch DEVICE org.nxkit.NavigationCycleReference --capture
```

Copy `Documents/cycle` from the app's data container after `cycle.json` is
written; that file is committed only after all PNG writes finish. Launch with
`--capture --sequence --every-frame` for the dense 32-clip cycle, or
`--capture --case push-4-5-` for a focused light/dark pair. Terminate and relaunch
without arguments to use the native Demo manually.

```sh
cmake -S Demo/GlassComparison/RasterTests -B /private/tmp/navigation-raster \
  -DNXKIT_RASTER_CONTROLLER_TESTS=ON
cmake --build /private/tmp/navigation-raster --target NavigationCycleRenderer UIKitNavigationTests -j6
/private/tmp/navigation-raster/NavigationCycleRenderer /private/tmp/cycle-after
```

For four processes, pass `OUTPUT "" SHARD 4` for shards 0–3, then use
`merge-cycle.py OUTPUT SHARD0 SHARD1 SHARD2 SHARD3`. Append
`--every-frame --sequence` after the shard arguments for dense capture.
For a scoped rerender, append `--target 4` after `--dense-sequence` and apply
its completed outputs with `update-cycle.py MATRIX PATCH0 … PATCH3`.
Alternatively, `--dense-sequence` captures the full matrix and its dense cycle
in one pass; the comparison tools select the appropriate checkpoint frames.
`compare-cycle.py NATIVE BEFORE AFTER OUTPUT` requires NumPy and Pillow and
produces every-pair metrics, traces, contact sheets and selected filmstrips.
Dense movies use `compare-cycle-dense.py NATIVE OUTPUT SHARD0 … SHARD3` and
require FFmpeg. The native fixture calls only public UIKit/QuartzCore APIs;
runtime class names are read-only hierarchy observations.

Apple references: [navigation bar](https://developer.apple.com/documentation/uikit/uinavigationbar),
[supplemental leading items](https://developer.apple.com/documentation/uikit/uinavigationitem/leftitemssupplementbackbutton),
[navigation controller](https://developer.apple.com/documentation/uikit/uinavigationcontroller).
