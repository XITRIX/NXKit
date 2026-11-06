# Standalone lenses and a composed tab bar

[Open the pixel viewer](comparison.html). The same live Demo `LensTabBarView` supplies the NXKit scene: existing regular glass for the platter, and an independent `LensView` for its selection. The native fixture hosts public regular glass and a manually instantiated `_UILiquidLensView`. Its real UITabBar, UISwitch and UISlider provide additional hierarchy/geometry observations above the compared crop; they are not silently replaced by the manual composition.

There are **46 static scenes**: 16 resting and 30 lifted, both appearances, gray/lines/colored waves/primary-color backgrounds, both explicit lens variants, selected slots 0/1/2, and 80×48 / 116×56 / 180×64 manual sizes. Each scene also isolates three bare lenses at 48×48, 80×80 and 120×64. Native motion traces and typography exports are excluded from these static aggregates, but remain part of the 52-case native fixture. The actual compact UITabBar's measured content-width selection rectangles are separately checked by LensTabTests and reproduced by the compact live Demo.

Native frames are injected directly. Full inputs are 1440×1120 pixels at 2×; the fixed displayed crop is stage y230–510 points. No translation, color correction or exposure fit is used. Source images and native framebuffer crops are sRGB. Native RGBA icon/label exports hold typography constant for optical measurements; the live Demo uses its own portable icons and labels. The fixture caller supplies the resting black 10% fill and eight-point lift growth; these are declared inputs, not recovered private UIKit defaults.

These centered controls are a smoke test for the live Demo composition, not a complete perimeter test. Their centered symbols originally led to the incorrect conclusion that a sharp selected overlay was sufficient. The [edge refraction comparison](../refraction/README.md) disproves that conclusion: deep interior ink stays fixed, while perimeter ink is remapped, split, or removed.

The current Demo keeps the full selected row underneath the lens and supplies it as a weak optical source. The normal row is punched out beneath the lens. The source is sampled before the completed output is clipped, and it is drawn once. The following results have been regenerated from the final production archive; the richer refraction report is the primary test of edge behavior.

| Mean absolute sRGB channel difference, 0–255 | Body | Rim | Optical band |
| --- | ---: | ---: | ---: |
| Bare lenses, resting | 0.166 | 0.181 | 0.160 |
| Bare lenses, lifted | 1.262 | 12.851 | 5.132 |
| Manual selection over glass, resting | 1.388 | 1.720 | 1.512 |
| Manual selection over glass, lifted | 2.263 | 10.971 | 4.238 |

Metrics equally weight each element and case. The body excludes `min(12pt, 20% of the short side)` from the perimeter; rim covers 0.5–2.5 points inside; the optical band covers 2.5–12 points inside. These masks differ from the earlier four-point standalone model study in [research.md](../research.md), so those values are not interchangeable. The composed measurement includes selected typography and the underlying regular material. [All per-case values](metrics.json) and [renderer/archive/fixture/asset hashes](provenance.json) are retained.

The high lifted-rim errors remain visible in the overlays. The richer source tests now verify refracted and spectrally separated selected ink. Bare-lens caustics and perimeter lighting remain approximations; exact private portal behavior and velocity-dependent stretching are not reproduced exactly. The oracle is iPadOS 27.0 build 24A434 and the NXKit output is the production retained raster renderer; physical iOS 27.2, Metal/Dawn parity and native input timing remain unverified.

## Reproduce

```sh
python3 Demo/GlassComparison/build-lens.py NATIVE_RUN
python3 Demo/GlassComparison/capture-lens.py DEVICE NATIVE_RUN
python3 Demo/GlassComparison/build-lens-raster.py RASTER_BUILD RENDERER_RUN
python3 Demo/GlassComparison/compare-lens.py NATIVE_RUN RENDERER_RUN/LensRenderer REPORT --content=NATIVE_RUN/native/content
```

The capture driver writes native typography exports into `NATIVE_RUN/native/content` after the two explicit export cases. The `--content` option uses those fresh exports; omitting it uses the retained report assets under `docs/liquid-glass-lens/native/content`. The native fixture uses research-only private API and does not link into the shipped application. The raster adapter copies its core archive before linking, preventing a later rebuild from silently changing that oracle. Run LensTests, LensTabTests and LensRefractionTests in the retained raster project for state, interruption, presentation/scale, source ownership, fractional pixel phase, alpha composition, live text, perimeter copies, channel transport, compact geometry, selection and moving punchout regressions.
