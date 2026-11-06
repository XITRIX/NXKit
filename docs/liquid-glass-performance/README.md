# Liquid Glass performance review

[Before/after pixels and measurements](comparison.html) · [Raster measurements](metrics.json) · [Device CPU samples](device-profile.json)

The shared regular/clear glass, containers, foreground vibrancy, lens source capture, interaction display links and application frame pacing were reviewed. The full-screen iOS Demo was the measured workload, following the reported heating. The regular/clear/tinted/corner/shadow/container/gallery regression harnesses cover the shared rendering paths.

## Findings and fixes

1. **Idle pacing retained the last interaction rate.** The run loop set its target rate only when presenting. After a 120 Hz interaction, a clean screen could keep polling events, display links, traits and the entire view tree at that rate. It now resolves requests after this iteration's callbacks/animations and updates pacing before either render or idle branches. A clean screen returns to the existing 30 Hz idle target. Touch/animation requests remain supported.
2. **A stationary lens repeatedly invalidated itself.** Its elastic-transform setter dirtied the layer even when the transform was identical. A lifted lens follows its surrounding view motion with a display link, so stationary polling could repaint the entire screen. Equal transforms now leave the tree clean. The full-screen regression settles a lifted lens, polls it again and checks that its layer needs no display.
3. **Hidden console chrome kept polling.** Navigation status/actions display links ran even when the full-screen demo hid the NX bars. Periodic refresh now checks window attachment and the presentation visibility/opacity of ancestors. Manual refresh/action resolution remains available, and polling resumes on reveal. Tests cover visible, hidden, revealed and detached widgets.
4. **Resting vibrancy used the lifted compositor.** At zero lift, the selected source now uses the same direct foreground blender as ordinary glass children. This removes a compatible source-surface allocation/copy and a backdrop image-filter pass per rendered resting lens. Lifted/partial lenses retain device-aligned source capture and per-ray RGB compositing. The color coefficients, optics and source ownership are unchanged.
5. **Containers shaded gaps between distant members.** A navigation container's Back button and trailing pair previously shared one rectangular filter region across the intervening space. The same shared material/union program now runs over disjoint bounded regions when safe. Regions include sampling reach, possible merge expansion and elevation before partitioning; touching/near items and wide explicit sampling overrides retain one common pass. Actual save-layer recording tests check disconnected, joined and wide-sampling cases. Existing shared sizing, merged joins, tint order and shadows remain covered by the container suite.

## Connected iPhone evidence

A 16.13-second Time Profiler capture attached to the existing installed Debug demo on the connected **iPhone 15 Pro Max, iOS 27.2 (24B5110a)**. It collected 882 one-millisecond main-thread sample weights. Inclusive samples included 478 ms in `drawAndLayoutTreeIfNeeded` and 237 ms in navigation-action refresh. There were no sampled `skiaRender`, glass/lens draw or GPU-submission stacks during that idle interval. Inclusive values overlap; they are not independent totals or precise elapsed timers.

This identifies avoidable idle CPU work, rather than establishing that glass continuously rendered throughout the device capture. The raw trace remains at `/private/tmp/nxkit-glass-before.trace`; only its aggregate and source/capture hashes are retained here. The running device app was not replaced by this review.

## Repeatable rendering and pixel checks

`Demo/GlassComparison/GlassPerformance.cpp` uses the actual Demo composition, a 402×874-point viewport at 2×, both themes, system/blue backgrounds, and resting/held selection. The retained Skia raster archive is a Debug CPU build; NXKit is optimized. Quick mode measures one direct frame per scene. The host had concurrent workloads, so these wall times are indicative CPU raster measurements, **not iPhone GPU frame times**.

| Scene | Before raster ms | After raster ms |
| --- | ---: | ---: |
| Light system, initial rest | 18,268 | 7,823 |
| Light blue, warmed rest | 18,751 | 9,596 |
| Light blue, held | 21,167 | 10,825 |
| Dark system, initial rest | 20,718 | 9,847 |
| Dark blue, warmed rest | 19,078 | 9,256 |
| Dark blue, held | 22,469 | 11,032 |

The warmed blue resting images differ by at most **1/255** per channel. Both held images match exactly. Initial neutral captures retain small selection-edge differences: mean 0.048/255 light and 0.010/255 dark, maximum channel errors 25/27. Full images and ×8 differences are retained; these initial frames are not described as pixel-identical.

The benchmark first drives 64 application iterations against an empty canvas clip to avoid expensive CPU raster work during invalidation diagnostics. Its `clippedPresents` count measures invalidation under that artificial condition; **it is not an actual idle frame-rate measurement**. Initial source setup can remain invalidated until a real draw. Once warmed, held cases changed from 60 submissions per 60 clipped ticks to zero. The separate application-scale test verifies the clean render branch does not acquire/present a framebuffer, current display-link requests take effect, and pacing drops from 120 to 30 when they end.

## Verification and remaining limits

The focused scale/pacing, full-screen actions/visibility/settled-lens, lens colors/alpha/lifecycle, fractional Retina sampling, motion replay, gallery, container, regular/clear material, appearance, rim, corner and shadow tests are exercised. The iOS Demo builds against the device SDK with signing disabled. See `validation.json` for final command outcomes.

No glass resolution reduction, frozen background cache or frame-rate cap on active interactions was introduced. Backdrops remain live while scrolling and the existing optical coefficients remain in use. Lifted lenses still need source capture and refracted-channel compositing; merged geometry and wide-context regular glass remain relatively expensive. All CPU views are still traversed at the idle target; this review does not add an event-driven sleep architecture.

**Post-change device GPU cost, energy use and temperature reduction have not been measured.** A physical before/after thermal run must use the same optimized build, screen brightness, scene, scroll/drag workload and initial thermal state, with Metal capture/HUD overhead controlled. Switch/Dawn performance and device-scale 3× GPU performance remain unverified.

```sh
cmake --build /path/to/raster-tests --target GlassPerformance
/path/to/raster-tests/GlassPerformance /tmp/glass-performance --quick
# Omit --quick for repeated full-screen frames and all gallery/lens scenes.
```

The before renderer is identified in `provenance.json`; its pixels and timing data are retained. The final benchmark records its clipped-loop diagnostic under that explicit name; the frozen before renderer used the older `idlePresents` field name for the same clipped procedure.
