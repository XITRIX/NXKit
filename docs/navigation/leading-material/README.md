# Leading navigation material overlap — 2026-11-03

The supplemental leading glass faded while it overlapped Back. Material ownership
then replaced part of Back with a translucent surface, exposing the black
backdrop as a moving dark hole. This was distinct from UIKit's intentional brief
foreground blur, and was especially visible after ordinary ink became `.label`.

The navigation transition now keeps that glass opaque, grows it from a subpixel
surface on push and shrinks it into Back on pop. Incoming icon opacity follows
its own reveal phase; it no longer accumulates on Back before separation. Back's
blur returns to zero sooner. The subpixel scale remains invertible because exact
zero scale currently decomposes to unit scale in Core. General translucent
material ownership remains outside this navigation fix.

## Captures

Each sheet shows actual request-relative sample times. The nearest native frame
is selected without shifting its clock or interpolating pixels. Fresh portable
captures contain 91 geometry samples and ten rendered checkpoints per clip.
Native cycle and pre-fix dense evidence are reused, with their original times.
Root push has native/after panels; the other cases include a before panel.

- [Push, dark](push-4-5-dark.png), [pop, dark](pop-5-4-dark.png)
- [Push, light](push-4-5-light.png), [pop, light](pop-5-4-light.png)
- [Root push, dark](push--1-5-dark.png), [root pop, dark](pop-5--1-dark.png)
- [Root push, light](push--1-5-light.png), [root pop, light](pop-5--1-light.png)

The glass no longer develops a dark interior patch. Brief glyph blur is retained,
as seen in native UIKit. Residual phase, shape, rim and blur differences remain;
these are focused artifact checks, not a claim of full animation parity. The
native reference is the existing public-API iOS 27.0 cycle fixture. Rendering is
portable raster at 3×, not physical-device Metal execution.

Selected source-image hashes, source hashes and actual timestamps are retained
in `sources.json`; compressed per-clip traces accompany the sheets. Earlier
cycle and color reports remain historical evidence.

## Reproduction and regressions

Build the existing `NavigationCycleRenderer` and `UIKitNavigationTests` targets
in `Demo/GlassComparison/RasterTests`, then run:

```sh
NavigationCycleRenderer /tmp/leading-push push-4-5-
NavigationCycleRenderer /tmp/leading-pop pop-5-4-
NavigationCycleRenderer /tmp/leading-root-push push--1-5-
NavigationCycleRenderer /tmp/leading-root-pop pop-5--1-
ctest --test-dir BUILD -R '^(UIKitNavigationTests|GlassScreenTests)$' --output-on-failure
```

The navigation regression hides glyphs and compares the overlapping material
against Back alone, checking that a five-by-five interior probe loses no more
than 4/255 intensity during both separation and absorption. Other assertions
check subpixel delayed birth, independent ink reveal, geometric disappearance,
root-specific timing, sharp Back at completion and nonanimated interruption.
Both focused suites pass. macOS and unsigned iOS Demo builds compile and link;
their binary hashes are recorded in `sources.json`.
