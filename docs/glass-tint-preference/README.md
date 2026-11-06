# Glass tint preference

Working tree after `9acc9a7`, audited 2026-11-04.

The portable trait runs from clear (`0`) to tinted (`1`), with a regular midpoint
of `0.5`. Apple 27+ follows its native system setting; earlier Apple releases,
Switch and custom backends use the midpoint. Applications can override the
value on every platform, then clear the override to follow the system again.

```cpp
auto app = NXKit::UIApplication::shared;
app->setOverrideGlassTintAmount(0.25f); // Main thread; save in app settings.
auto effective = app->glassTintAmount();
app->setOverrideGlassTintAmount(std::nullopt); // Follow system again.
```

The application resolves its scalar immediately. Inherited
`view->traitCollection()->glassTintAmount()` updates before the next frame's
layout, including current controllers, masks, presentations and future children.
Previous collections remain snapshots. Out-of-range and nonfinite values throw
`std::invalid_argument` without altering the current override. A standalone
material can use `UITraitCollection::traitCollectionWithGlassTintAmount(value)`;
that factory doesn't mutate a view's inherited environment.

## Native comparison and corrected behavior

The independent iOS 27.1 fixture crosses five preferences (0/.25/.5/.75/1), both
effect styles, neutral/opaque-blue colors, both themes, two sizes, six uniform
backgrounds and textured content: **30 scenes, 720 glass instances, 480 flat-color
controls and 80 texture-transmission controls**. The requested three preference
positions cover all 24 style/color/theme combinations. Native controls are
retained under [matrix/native](matrix/native); source and reproduction instructions
are under [TintPreference](../../Demo/GlassComparison/TintPreference/README.md).

The fixture overrides only its own volatile UIKit argument domain before the
first getter call, and verifies UIKit's effective value on every capture. It
never calls the persistent system setter. See [native adapter evidence](iOS27-control.md).
It captures the **composited simulator framebuffer**; `drawHierarchy` reconstructs
backdrop filtering differently and was rejected as the blur reference.

Observed native behavior and implementation corrections:

- Regular glass retains a neutral body at 0%. Dark glass on black stays about
  RGB 32/32/32, instead of becoming transparent.
- Explicit accent tints survive every position. The regular blue control on
  black stays about 0/121/255 in dark appearance, including at 0%.
- Clear style keeps its body/color transfer while its frost changes with the
  preference. A system preference of zero doesn't remove the material.
- Regular light glass increases its neutral veil; regular dark glass reduces
  backdrop transmission above the midpoint. The fully tinted endpoint still
  transmits backdrop color instead of becoming a constant opaque fill.
- Frost is independent of the body. Clear and colored glass use the measured
  smaller kernels; neutral regular retains its contextual frost/detail lane,
  progressively suppressing fine detail as tint increases. Caller-specified
  optical overrides retain precedence; icons/children and hit geometry do not fade.

The production body resolver now passes all 480 native controls with mean
channel error **1.80/255**, worst residual **14/255**. Retained-tree captures at the
three requested settings average **1.88, 1.79 and 1.83/255**. Previously the two
endpoints averaged **67.76 and 46.87/255** and fail the new regression checks.
Mean textured contrast-amplitude error is **1.53/255**, worst **9.01/255**. These
are bounded approximation results, not complete pixel parity: directional rim
highlights and detailed textured blur still differ. The native comparison is
from iOS 27.1 Simulator, not a claimed physical-iOS 27.2 visual capture. Explicit
colors other than blue retain the existing tint model and midpoint controls;
the full preference matrix has not been measured for every hue/alpha.

## Application settings and platform bridge

Open **Glass → Glass appearance · app settings**. Follow system or choose a tint
value. Demo persists it in `glass-appearance.txt` beneath its SDL preference
folder. UIKitCore stores no preferences and never changes the global setting.
Custom `BackdropEffect` shaders and caller-owned experimental `LensView` fills
are not rewritten by this preference.

The [read-only bridge investigation](native-evidence.md) establishes UIKit's
nullable `NSNumber*` getter and AppKit's cached inherited `NSGlassTintAmount`.
The bridge uses dynamic lookup/version guards on Apple 27+, validates values,
and falls back to 0.5. It adds no synchronization, disk reads, thread or display
link. Exact external system-change delivery remains unverified. UIKit's private
implementation details must remain guarded; the native fixture's private setter
is deliberately never used by NXKit.

Focused trait, material, grouping, Core boundary, lens, scroll-pocket, navigation,
interactive-pop and tab regression tests pass. They cover validation, precedence,
reset, inheritance, reparenting, snapshots, appearance copies and cache refresh.
The 480 independent controls are checked by `GlassMaterialTests.cpp`; generated
control values must come from native framebuffer captures, never our renderer.
Earlier trait-revision baseline/compile logs remain under `verification/` and
refer to their recorded ABIs. The calibrated revision's build/archive evidence
is recorded separately in `matrix/verification`.

Apple's public [UIGlassEffect documentation](https://developer.apple.com/documentation/uikit/uiglasseffect)
defines the style and color API; the native captures establish the preference's
observed rendering response rather than asserting private formula equivalence.

## Calibrated archives and builds

The frozen material ABI is `4b541699bc48606e83a9699c093df049dfc09c5a03d25c847d0c50b6fc34ba91`.
A fresh cold harvest and matching archive-only replay cover **43,416 Mac frames**:
six scale/theme scroll-pocket profiles, both-theme full navigation/interactive
catalogs, ten fixed preferences and six 161-position continuous sweeps. Every
archive-only CSV reports zero runtime Metal library/pipeline compilations;
explicit failure counters remain zero. The source catalog retains content-addressed
prior raw-state coverage, while all GPU binaries are freshly compiled for this
ABI. The resulting Mac/iOS bundles contain **137 libraries / 110 pipelines**.

Normal macOS and signed iOS builds pass and embed matching metadata; Switch NRO
compiles/packages. The final app is installed on the iPhone. Its bundled-only
stress replay prepares 110 pipelines and reports no misses/runtime pipeline
compilations with runtime compilation disabled; this is a smoke check, not a
full device capture of every setting or a sustained thermal measurement. Normal
launch is restored and the temporary comparison simulator is shut down.

See [verification summary](matrix/verification/summary.json), compressed per-frame
CSVs, build logs and device console evidence under `matrix/verification`. The
native matrix's two compact [dark](matrix/after-dark-matrix.png) and
[light](matrix/after-light-matrix.png) contact sheets show native/NXKit pairs.
