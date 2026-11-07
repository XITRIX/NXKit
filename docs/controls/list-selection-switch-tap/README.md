# Selectable list rows and short switch taps

2026-11-07, working tree after `b895e97`.

The four supplied physical-iPhone recordings show two independent problems. A held Demo row could prevent a later scroll, and a quick switch tap could begin shrinking before its lens completed expansion. The Slider behavior reported as correct is preserved.

## Selectable rows

The Demo uses `UIControl` rows in a `UIScrollView`, rather than native table cells. After the scroll view admitted a held contact, its ordinary UIKit-compatible `touchesShouldCancelInContentView(UIControl)` default protected the control. This was inappropriate for selectable list rows, but remains appropriate for standalone controls and admitted Slider/Switch accessories.

The Demo now shares `GlassDemoListView`, with explicit weak registration of selectable rows. Its cancellation policy lets an available-axis scroll pan cancel row selection after admission. A nonpreventing row drag recognizer retires selection without capturing the scroll pan. Both stay possible during a stationary hold or subthreshold motion; releasing selects once. Unsupported-axis drags clear selection without creating horizontal scrolling. That last cancellation policy is covered as a Demo regression, rather than claimed as a recovered native threshold.

Fullscreen, navigation-morph, large-title, catalog and Slider/Switch settings lists use the helper. Nested controls retain their own admission policy. `canCancelContentTouches=false` and custom cancellation vetoes remain effective. UIAction activation and controller-focus behavior are preserved.

The 34-case routed regression uses the actual six Demo row families: tap, fast vertical swipe, admitted hold followed by vertical drag, horizontal cancellation and stationary held release, plus ordinary-control/default-policy and embedded Slider/Switch cases. Existing Core defaults are unchanged by this follow-up. The earlier [native control/scroll investigation](../scroll-arbitration/README.md) retains public API and real OS-touch evidence for that shared infrastructure.

## Switch lens lifetime

Fresh native UIKit captures establish a shared `spec.lensHangTime` of 0.22 seconds. A short contact commits its value and actions on release, but lowering and its alongside geometry wait until that interval ends. The full expansion and spring overshoot therefore occur before shrinking. This belongs to LensView rather than a switch-specific timer.

NXKit now implements the shared delay, including native same-state callback behavior. Switch size targets change in the alongside callback, so glass and geometry lower together. Forced cancellation, disabling, removal and nonanimated state changes clear pending visuals; cleanup after contact completion does not emit spurious cancellation or undo the committed value. TabBar geometry also follows its actual shared lens transition. Slider retains its existing external presentation clock.

[Native switch report](../../switch/tap-hang/README.md) retains four OS-touch captures, eight direct private-lens timing probes, source hashes, raw traces, REA receipt and limitations. The direct probe is independent of touch routing. Twenty-six shared LensView timing cases and 38 additional switch cases cover 60/120Hz, both states/styles, short/long contacts, callbacks, interruption and lifecycle cleanup.

## Evidence and verification

[recordings.json](recordings.json) records the original user-video paths, hashes and metadata. Original videos remain in Downloads. They are variable-frame-rate recordings: timestamps, rather than nominal frame rates or frame counts, govern comparison. These videos do not establish application FPS.

Final build/test results are recorded in [verification.json](verification.json). Simulator verifies behavior and rendering paths; it does not establish physical-device timing, thermal performance or complete optical parity. The final RelWithDebInfo build with matching offline shaders was signed, installed and launched on iPhone Pro XITRIX (iOS 27.2) on 2026-11-07. Installation does not establish physical animation timing or thermal parity.
