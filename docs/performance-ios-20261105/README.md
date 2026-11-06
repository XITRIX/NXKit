# Full-screen iOS Demo performance — 2026-11-05

The subsequent [simulator optimization report](optimization/README.md) records
the renderer changes, fidelity checks and final before/after measurements.
The physical-device numbers below describe the earlier baseline.

Working tree after `50a7240`. Physical iPhone 15 Pro Max (A17 Pro), iOS
27.2/24B5110a. Optimized, symbolized `RelWithDebInfo`, Metal validation disabled,
no debugger or Instruments attached during the clean timing runs. The shader
ABI remains `738e5b4724d6d1d013338237256273513fbc434f6b21ba76b73abb1dcf4c8e96`.

## Measurement

`NXKIT_IOS_DEMO_REPLAY=1` launches the existing full-screen UIKit Demo directly:
2 s warmup, 12 s list scrolling, eight navigation push/pop pairs over 16 s,
12 s more scrolling, then idle. Each scroll position follows elapsed wall time
and the live content/viewport/inset geometry. Slow rendering does not shorten
the distance traveled. Navigation uses the real controller animations, rather
than synthetic offscreen frames. The replay stops after 42 s. It exercises
all eight navigation-item arrangements; it does not reproduce finger-driven
interactive pop or every Demo destination.

`NXKIT_FRAME_METRICS=1` records private diagnostics in
`Documents/frame-metrics.csv`. Normal app launches install no observer or queue
wrapper. The renderer's private CPU scope measures layout, layer drawing,
submission/presentation, drawable acquisition and total frame wall time. Idle
iterations are recorded separately from submitted frames.

An opt-in forwarding Metal command queue adds completion handlers to Graphite
command buffers. It preserves the native command-buffer descriptor, queue,
retained-reference policy, asynchronous submission and render passes. GPU time
uses `GPUEndTime - GPUStartTime` after completion, as documented by
[Apple](https://developer.apple.com/documentation/metal/mtlcommandbuffer/gpustarttime).
These are command-buffer execution intervals; overlapping buffers and CPU/GPU
pipelining mean they must not be added to CPU time or inverted to claim FPS.

Actual delivered FPS is computed from drawable `presentedTime` callbacks,
using `(presented frame count - 1) / elapsed presented time`. It is not the
requested refresh rate, display-link callback count or inverse CPU duration.
The analyzer resolves asynchronous rows by frame identifier and reports
presentation intervals and CPU/GPU percentiles separately. Navigation averages
include the time between pushes/pops, so scrolling is the primary continuous
FPS test. The console export happens only after motion has settled.

## Reproduction

Build/install `ThorVGApp Performance`, then run:

```sh
python3 Demo/scripts/measure_ios_demo.py --device <UDID> --output /tmp/ios-paced
python3 Demo/scripts/measure_ios_demo.py --device <UDID> --output /tmp/ios-uncapped --uncapped
python3 Demo/scripts/measure_ios_demo.py --device <UDID> --output /tmp/ios-no-glass --mode no-glass
python3 Demo/scripts/measure_ios_demo.py --device <UDID> --output /tmp/ios-no-pockets --mode no-pockets
```

Each command limits console capture to 60 s, accepts an expected time-limit
exit only with a complete marked CSV and all replay phases, then restores
ordinary Demo launch in `finally`. A host watchdog bounds stalled launches.
`NXKIT_FRAME_METRICS_CONSOLE=1` provides the marked export after 46 s, avoiding
the unreliable wireless CoreDevice file-transfer service. Plain file logging
also works for manual captures and for `NXKIT_GLASS_STRESS_REPLAY=1`.

Diagnostic isolation controls retain the real bars, content, geometry and
navigation. `no-glass` replaces only glass BackdropEffectView materials with an
identity backdrop shader; it retains scroll pockets. `no-pockets` changes only
scroll-edge styles to `.none`; `no-effects` enables both controls. New views
created during navigation receive the same control. These controls deliberately
change appearance to identify costs and are never enabled in ordinary launches.
Their identity shader may compile during warmup, so they are not archive-hit
regression tests. No optical quality reduction is applied to production glass.

## Measured results

All four clean runs report maximum 120 Hz, Low Power Mode off and Serious
thermal state throughout the measured phases. These are single sequential
runs, not randomized or temperature-controlled A/B trials.

| Configuration | First scroll FPS | Navigation FPS | Second scroll FPS | First-scroll GPU CB p50 |
| --- | ---: | ---: | ---: | ---: |
| Normal pacing, full rendering | 42.33 | 44.00 | 38.81 | 32.40ms |
| Uncapped, full rendering | 29.89 | 32.21 | 28.81 | 44.99ms |
| Normal pacing, pockets disabled | 32.15 | 33.35 | 31.12 | 43.14ms |
| Normal pacing, glass material bypass | 112.38 | 110.86 | 111.49 | 7.22ms |

For normal full-rendering first scroll, median frame wall time is 23.47 ms:
layout 0.06 ms, layer drawing 3.88 ms, submission 2.73 ms, drawable acquisition 16.90 ms
(rounded individual medians do not sum exactly). Drawable acquisition is a
wait for an available presentation surface, not CPU layout work. The CPU trace,
long GPU intervals and large material-bypass improvement together identify live
glass rendering as the dominant bottleneck. Turning off pockets alone does not
restore frame rate. The uncapped run shows that removing the limiter is not a
sufficient fix; its lower FPS cannot be attributed solely to pacing because
thermal clocks and capture order were not controlled.

The bypass is a diagnostic bound, not a production optimization or proof that
full-fidelity glass can currently run at 120 FPS. No glass parameters, kernels,
refraction, styling or production rendering policy are reduced by this patch.
The next optimization target is the material/filter-pass graph, including its
pyramid intermediates and native composition. Individual shader attribution
still requires a GPU-pass capture; these measurements do not distinguish blur
from SDF/composition cost conclusively.

An earlier attempt to bypass materials through the component-owned view effect
raised an exception before collecting useful data. It is excluded. The retained
control changes only the private layer material and keeps the navigation bar's
owned UIGlassEffect description intact. Final callback logging also retains its
writer through asynchronous GPU/presentation completion during shutdown.

Compressed raw CSVs, per-run summaries and `verification.json` preserve the
measurements and restore receipts' outcomes. The final app returns to ordinary
launch with no replay, diagnostic writer, uncapping or isolation flags.

## Initial CPU profile

A separate 30 s Time Profiler run captured 6,719 ms of main-thread sampled
weight. Inclusive layer rendering accounts for 2,753 ms, submission for 1,841
ms and layout for 68 ms. These overlap and cannot be added. Glass-container
rendering accounts for 1,059 ms inclusively; filter evaluation includes pyramid
Merge/MatrixTransform/Crop chains. Metal render-pass encoder creation accounts
for 942 ms inclusively. This identifies filter/pass preparation as substantial
CPU work but does not identify an individual GPU shader as the sole bottleneck.

`cpu-summary.json` retains the resolved stacks. The original 20 MB Instruments
trace and XML export are under `/private/tmp/nxkit-ios-fps/` on this workstation.
An initial diagnostic CSV was recovered with LLDB only after motion completed;
that run is labeled `profile`, since Instruments was attached during motion.
It must not substitute for the clean timing runs.

## Validation and limits

The iOS optimized build and six frame-loop/traversal regressions pass, including
new observer checks for submitted/idle frames, one callback per iteration and
restoration of instrumentation scope. The analyzer's synthetic 120 FPS test
verifies asynchronous row ordering, failed presentations and separation from
CPU throughput. No public UIKit API or default appearance is changed.

This is a finite warm-device test, not a controlled temperature, power or
battery-energy experiment. Thermal categories are recorded throughout; equal
categories do not imply equal clocks or temperature. Low Power Mode remains
unchanged. The diagnostic writer itself has overhead (observed file writes in
the CPU trace); no forced GPU wait or GPU validation is added. A cold-device
thermal comparison and exact GPU-pass attribution need separate captures.
