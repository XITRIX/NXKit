# Idle CPU review — 2026-11-07

Working tree after `b895e97`; shader ABI remains `2ee3d064`. This review changes
scheduling and invalidation, without changing lens/material equations or output
resolution. The latest signed build was installed and launched on iPhone Pro
XITRIX. CPU results below are from Simulator, as requested.

## Findings and changes

1. SDL's UIKit driver has no native `WaitEventTimeout` implementation. The
   fallback in `SDL_events.c` pumps events and sleeps for **1ms** until the frame
   deadline. That repeatedly enters both native UIKit run-loop modes and polls
   game controllers, even when NXKit submits no frame. The iOS backend now waits
   in `CFRunLoopRunInMode`, waking on native sources or the existing deadline;
   queued SDL events are preserved for UIApplication. Input wakes keep the
   pending frame deadline, and uncapped mode still bypasses sleeping.
2. SDL also creates an empty high-refresh `CADisplayLink` on ProMotion displays.
   Its refresh request now stops at the idle target and resumes for motion,
   using SDL's public animation-callback API. Native rotation keeps its refresh
   request between geometry samples. The callback does not run a second render
   loop.
3. The renderer previously visited every visible view for layout and display
   before checking whether a frame was dirty. View invalidations now schedule
   this work explicitly, independently of layer mutations. Clean frames skip
   the tree walk. Clearing the flag before callbacks preserves invalidations
   raised during traversal. Unchanged window geometry is no longer reapplied
   every frame. Environment polling, animation advancement, main-queue work,
   drawable retries and exposes remain active.

The settled fullscreen runtime has **zero CADisplayLinks, zero animated layers,
zero active touch events, and clean layer/view flags** (`settled-state.txt`). Its
remaining work includes the 30Hz environment/geometry check and native run-loop
delivery. The main NX menu additionally resolves visible responder actions and
controller glyph metadata each tick, and updates its seconds clock once per
second (`after-menu.sample.txt`). Therefore the menu legitimately presents clock
updates; it is not a completely static UI. Those live action providers have not
been cached in a way that could make availability or callbacks stale.

## Measurements

Optimized arm64 RelWithDebInfo, iOS27.0/24A434 Simulator, Metal validation,
continuous rendering, uncapped mode and frame diagnostics disabled. CPU is
process CPU-time delta divided by elapsed time; **100% means one host core**.
It is not Xcode's physical-device CPU gauge or a measurement of power/heat.

| Case | CPU before | CPU after |
| --- | ---: | ---: |
| iPad Simulator, settled fullscreen | 13.84% (20s) | 2.42% (two20s windows, final quiet run) |
| iPhone Simulator, normal NX menu, two 20s windows | 13.39% | 4.11% |

The native-wait-only intermediate measured 3.45%; adding the clean-tree skip
measured2.65%. Final fullscreen measurements are retained in
`after-fullscreen-cpu.json`. An
exploratory three-window capture was excluded because a debugger attached during
its last window. The retained runtime inspection was performed separately from
the reported clean CPU windows. These are directional before/after trials,
not an ABBA thermal experiment; initial trials overlapped build activity and
other Simulator replay work. They establish a substantial reduction, not literal
zero idle CPU or a physical-device temperature improvement.

The same live scrolling/navigation/idle-resume replay runs before and after.
This Simulator reports a **60Hz native screen**: scrolling submission rates are
60.10/60.08 before versus 60.04/60.02 after, and resume is 60.09 versus 60.10.
Navigation contains different layouts and transient 120Hz requests, so its
aggregate rate is not a steady-state FPS target. Runtime shader-library and
pipeline-creation counts do not increase during either replay. The resume
capture is a short wakeup check, not a sustained scrolling or 120Hz device test.
The idle phase can contain a final transition frame; settled startup regressions
separately assert zero framebuffer acquisitions/presents over 120 clean ticks.

## Verification

- Paced and uncapped frame-clock tests pass.
- Application startup tests pass at 2×/3× in both orientations, including view-only
  display/layout invalidations, main-queue wakeups, rendering after idle,
  unavailable drawables, exposes, animations and continuous/uncapped overrides.
- Idle display-tree visits are zero over 120 clean ticks in normal mode.
- Layout passes, interaction, layers and live trait/preference changes pass.
- Simulator, macOS and signed iPhone builds succeed.

The raster fixture now supplies its missing no-GPU idle-cache hook. Startup and
trait fixtures respect real frame waits; a tight loop is no longer mistaken for
one rendered frame per function call after the existing pacing gate was added.

To repeat a clean measurement (the fullscreen replay finishes before sampling):

```sh
python3 Demo/scripts/measure_simulator_idle.py \
  --simulator F0FF4E71-DF79-41DD-8862-DC161CB409E0 \
  --app Demo/build/ios-sim-ninja/ThorVGApp.app \
  --output /private/tmp/nxkit-idle-repeat --samples 3
```

Use `--screen menu` for the normal NX menu. Do not attach a debugger or enable
frame metrics during CPU sampling. Raw profiles, CPU windows, replay summaries
and final regression logs are kept beside this report.
