# Interactive navigation pop — 2026-11-03

The [pop-title settlement follow-up](../pop-title-settlement/README.md) corrects committed title/content divergence after finger release. The original glass-only tracking comparison below did not score that relationship during settlement.

The [stable Back follow-up](../stable-back/README.md) adds an explicit automatic
Back identifier and unchanged-presentation regressions. Source and packaged-build
hashes below describe the preceding interactive-pop revision; the follow-up has
its own current verification record.

Core's `UINavigationControllerInterface::interactivePopGestureRecognizer()` now
matches UIKit's readonly, nullable gesture capability. `UIViewController`'s shared
navigation discovery exposes it without depending on a component kit. NXKit's
console navigation inherits the null/no-installed-gesture default. UIKit installs
one enabled leading-edge recognizer on its container view.

```cpp
if (auto navigation = controller->navigationController()) {
    if (auto gesture = navigation->interactivePopGestureRecognizer()) {
        gesture->setEnabled(false); // true to re-enable UIKit's installed gesture
    }
}
```

Application `onStateChanged` observation preserves the framework's internal
navigation action. The weak gesture delegate can veto recognition through
`gestureRecognizerShouldBegin`; disabling/resetting in that callback cannot
resurrect an abandoned attempt. The default recognizer requires visible, idle,
non-root navigation, an available Back configuration, and horizontal travel from
the leading 24-point strip. Its one-finger attempt cancels safely on a second
contact. A touch already owned by active recognition cannot become a delayed tap
when it cancels the gesture.

## Presentation and state

The destination becomes the logical top when tracking starts; the outgoing child
remains contained until completion. Content and titles follow finger translation
with rear parallax, dimming and a moving title visibility boundary. The source
icons and tint stay in place while glass partially reshapes towards the destination.
The layout preview measures copies of item metadata, never application-owned
custom controls. Shared identifiers/reused objects preserve identity; anonymous
items fall back to visual order. Newly introduced leading material previews as
an extension of Back.

On commitment, the already-expanding carrier continues the join/split. Existing
SDF material interaction and blurred ink replacement run from the release pose;
Back keeps its widened source geometry and the supplemental leading item uses
its interactive reveal phase. Moving titles render behind the glass. The shared
icon capsule's one-point excess width was corrected against the native fixture.
On cancellation, one reversible clock returns content, titles and the original
glass, then restores the original stack. The clock captures navigation's new
animations after application content loads, so application animations started in
`loadView` keep normal time. Host transforms preserve application content transforms.
Queued stack changes drain after settlement; resize and bar visibility changes
can force safe completion/cancellation.

UIKit 27's timed oracle reconciles cancelled appearance **after** the visual
return, with the original stack already restored:

1. Gesture begins: source will-disappear, destination will-appear, delegate
   will-show destination.
2. Cancelled return completes: destination will-disappear/did-disappear, source
   will-appear/did-appear, delegate will-show/did-show source.
3. A committed pop detaches the source before its did-disappear, then delivers
   destination did-appear and delegate did-show.

## Comparison evidence

The native public-API fixture performs 32 real edge swipes: eight adjacent
arrangements × light/dark × completion/short-swipe intent. It records 2,663
presentation frames, source/destination content frames, logical stack state and
material carriers. PNGs occur every fourth display-link sample. Two additional
timestamped lifecycle/delegate captures establish the cancellation ordering above.
Two further repetitions check the single unexpected completion described below.
No private UIKit methods are called; runtime class names are observation labels.

The portable replay uses the real containers and recognizer. During tracking it
feeds the **observed native content presentation progress** into the pan. This
isolates bar poses from OS input/display latency; it does not prove touch latency
or full spring/optics parity. After release, portable settlement runs on its own
clock with no fitted per-case time offset. The renderer draws the full window
into the same 80-point bar crop, rather than rendering the glass in isolation.

Across all 32 clips, carrier rectangle coordinate RMS error during tracking is
**0.525 pt**, held-pose RMS is **0.239 pt**, and maximum coordinate error is
**6.60 pt**. Hidden-background slots are included as logical carriers. No tracking
frames are omitted for carrier-count mismatches. Full data and per-case scores
are in [comparison.json](comparison.json), [native traces](native-traces.json.gz)
and [portable traces](portable-traces.json.gz).

One initial `6-5-dark-cancel` recording committed despite the short-swipe intent;
the replay cancelled. It also contains the largest delayed native bar response.
That recording remains in the statistics and its clip is explicitly annotated;
we do not fit the implementation to that anomalous outcome. Two fresh native
repetitions cancelled as intended at 101-point translation and 248 pt/s reported
release velocity. Their paired clips and traces are in [repeats](repeats/comparison.json).
The original matrix has 31 agreeing final outcomes; both repetitions agree.

Examples (UIKit left, portable right):

- [Leading item appears, dark, commit](6-5-dark-finish.gif)
- [Leading item disappears, dark, cancel](5-4-dark-cancel.gif)
- [Prominent action joins, dark, commit](1-0-dark-finish.gif)
- [Text items become a shared group, light, commit](4-3-light-finish.gif)
- [Repeated leading-item cancellation](repeats/leading-repeat-0-dark-cancel.gif)

Every matrix case has its own GIF and five-frame contact sheet in this directory.
The corresponding raw native traces retain timestamps, identifiers, runtime
class names and appearance events. The two [timed lifecycle traces](native-lifecycle.json.gz)
preserve the callback ordering. [Verification metadata](verification.json) records
current source and artifact hashes, regression results and strict Metal counters.

## Verification and reproduction

`InteractiveNavigationTests` covers routed touch input, tracking/reversal/holding,
commit/cancel, root boundaries, delegate veto/reset, observation, disabling,
second contacts/delayed taps, custom-view ownership, application transforms and
animations, resize/bar visibility, queued changes and cancelled appearance timing.
It is available as `UIKitInteractiveNavigationTests` under `UIKIT_BUILD_TESTS`
on Apple platforms without NXKit linkage, and in the shared headless harness
with a mixed-kit null-gesture assertion. The broader headless suites also check
navigation, tabs, gesture arbitration, scrolling, optics and module boundaries.
All 33 C++ regression suites and six catalog/embedding tests pass for this patch.
The separately built UIKit-only executable also passes with no NXKit library in
its link command. The macOS Demo and unsigned iOS Debug Demo build successfully.

Fresh-process Metal checks use the packaged archive, an empty writable cache,
`NXKIT_METAL_BUNDLED_ONLY=1` and `NXKIT_METAL_REQUIRE_ARCHIVE_HIT=1`. Both profiles
complete three passes over automatic and interactive transitions: **34,994 frames
total**, with **zero runtime library compilations and zero runtime pipeline
compilations**. The current catalog has 68 MSL libraries and 54 native pipeline
descriptions. Both app bundles match their baked outputs byte-for-byte. These
GPU checks ran on Apple M1 Max; the iOS package was built, not executed on device.
Raw counters and test/build summaries are retained in [verification](verification/).

Generate/run the native fixture on a dedicated simulator:

```sh
python3 Demo/NavigationComparison/build-interactive.py /tmp/native-interactive
xcodebuild -project /tmp/native-interactive/Interaction.xcodeproj -scheme Interaction \
  -destination 'platform=iOS Simulator,id=SIMULATOR_UUID' \
  -derivedDataPath /tmp/native-interactive/build -parallel-testing-enabled NO test
```

Copy the app container's `Documents/interactive` captures. Build the replay
with `Demo/GlassComparison/RasterTests`, then run:

```sh
python3 Demo/NavigationComparison/schedule-interactive.py /tmp/native-captures schedule.csv
InteractiveReplay schedule.csv /tmp/portable-interactive
python3 Demo/NavigationComparison/compare-interactive.py \
  /tmp/native-captures /tmp/portable-interactive docs/navigation/interactive
```

The Apple catalog covers the actual full-screen Demo in both themes/scales:
`MetalNavigationBenchmark --catalog --interactive --full` runs automatic pushes/
pops plus gesture cancellation at 72% and commitment at 100%, sampled at 120 Hz
for three passes. Add `--light --2x` for the other profile. Compatible additional
raw analytic-clip/MSAA states from the earlier capture are retained by their exact
MSL hashes and complete Metal descriptors. All libraries and GPU binaries are
compiled afresh under the current ABI; no old binary archive is relabelled.
Ordinary source edits now trigger CMake's catalog ABI check, including changes to
existing shader-generating input files rather than only glob additions/removals.

## Remaining work

Release dynamics and private SDF/blur optics remain approximations. The replay
measures bar poses at matched content progress, not end-to-end touch latency.
Native capture scheduling can delay material updates relative to content; its
outliers remain visible in the raw data. Full-content/RTL gestures, interrupting
an already-running automatic transition, custom transition-controller APIs and
complete custom-title/accessory motion fidelity remain unimplemented. Physical
iPhone/Switch execution of this change is unverified; the user's paused device
session was never resumed or replaced.

References: Apple's [interactivePopGestureRecognizer](https://developer.apple.com/documentation/uikit/uinavigationcontroller/interactivepopgesturerecognizer)
and [gestureRecognizerShouldBegin](https://developer.apple.com/documentation/uikit/uigesturerecognizerdelegate/gesturerecognizershouldbegin(_:)).
