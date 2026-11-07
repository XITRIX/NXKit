# UIKit core API progress

This document tracks NXKit against the portable, application-facing core of Apple's UIKit. It is both a compatibility audit and an implementation roadmap.

- Last audited: **2026-11-07**
- NXKit revision: **working tree after `b895e97`**

Selectable-list and short-tap follow-up (2026-11-07): Demo list rows explicitly
opt into scroll cancellation without changing UIScrollView's ordinary UIControl
protection. Row selection and scroll pan remain possible while held; available-axis
drags cancel selection, and stationary release selects once. A shared Demo list
helper protects embedded Slider/Switch accessories and explicit cancellation vetoes.
Thirty-four routed cases cover all six row families. Shared LensView now reads
native spec.lensHangTime (0.22s), deferring visual lowering and alongside geometry
until a short tap's lift completes. UISwitch value/actions still finish on release;
Slider retains its existing external clock. Twenty-six shared lens cases and 38
switch timing/lifecycle cases accompany eight direct native lens probes and four
real-touch switch captures. See `docs/controls/list-selection-switch-tap` and
`docs/switch/tap-hang`. Exact optical parity and physical-device timing remain Partial.

Control/scroll gesture arbitration (2026-11-07): Core now implements weak,
cycle-checked failure requirements, dynamic delegate/subclass ordering, touch/
event/press filters, and buffered UIView touch delivery with UIKit-default
recognizer delay flags. UIScrollView adds delaysContentTouches (150ms gate),
canCancelContentTouches, touchesShouldBegin and touchesShouldCancelInContentView.
Controls use that shared responder path: switch pickup waits while scrolling
can compete; a slider thumb has independent early feedback and captures both
axes, while a track swipe can scroll. Standalone controls respond immediately;
per the requested policy, disabled/non-scrollable ancestors add no delay.
Thirty-four end-to-end control cases plus focused dependency, buffering and scroll
hook regressions accompany 28 native OS-touch scenarios and a shipped UIKitCore
REA audit. Raw physical timestamps and delivered timestamps are distinct; a
loaded Simulator replay compressed a switch tap below its native press minimum
and is reported separately. Generic hardware/controller press buffering,
multitouch control parity and exact intrinsic switch press timing remain
partial. Evidence and reproduction: `docs/controls/scroll-arbitration`.

Idle CPU follow-up (2026-11-07): the iOS backend now waits in the native run
loop instead of SDL's 1ms fallback polling, and retires SDL's empty high-refresh
display link at the idle target while restoring its refresh request for motion.
Native rotation requests survive early event wakes. View display/layout,
safe-area and margin invalidations explicitly schedule the retained tree walk;
unchanged frames skip that CPU traversal as well as framebuffer acquisition.
Environment polling and main-queue/display-link delivery remain active, with
the existing 30Hz idle target. Paced/uncapped startup, display/layout wakeups,
traits, exposes, failed acquisitions, active animations and clean-tree tests
pass. Simulator measurements and remaining visible NX action/clock polling are
recorded in `docs/performance-idle-20261107`; these are host CPU measurements,
not physical iPhone power or thermal results.

Finite lens response correction (2026-11-07): elasticity stays active throughout
the lifted state. The shared affine starts its two-second fade to identity when
lowering begins, even under continuing movement. Ending contact while a control
remains lifted/decelerating does not start the fade; contact-only use retires on
endInteraction. Repeated layout/lowered-state updates cannot renew retirement,
and re-lifting cancels retirement without a pose jump. Position/deceleration,
lifted bounds and glass state remain independent. Both motion sources share
this policy; long/short holds, retirement and interrupted re-lifts are covered
at 60/120Hz. The earlier lift-start timeout was an incorrect interpretation of
the user's request and is superseded. This timeout is a deliberate portable
choice, separate from the measured native transfer. See
`docs/liquid-glass-lens/finite-response`.

Lens activation follow-up (2026-11-07): real `_UILiquidLensView` probes show
global motion-driven flex while lifted/settling, including ancestor movement,
but identity flex for resting lenses during scrolling or screen-style motion.
NXKit now gates view-motion sampling and frame/bounds-triggered wakeups to that
active lifetime, clearing stale history at rest and rebasing on the next lift.
The superview-relative hypothesis was rejected by the native measurements.
Both native styles and 2×/3× resting captures are retained in
`docs/liquid-glass-lens/relative-motion`; 96 portable regressions exercise
styles/themes/scales, ancestor/scroll/presentation movement and reactivation.

Lens performance and system-controls follow-up (2026-11-07): a finite live
TabBar lens replay separates rest, lift, drag, lowering and terminal idle;
CPU Time Profiler attribution and clean presentation/GPU measurements are kept
in `docs/performance-lens-20261107`. Lens filter inputs now use immutable child
devices when supported, avoiding client-surface snapshot copies; pointwise
vibrant foreground composition reads a frozen prefix directly, retaining the
original filter/surface paths as diagnostic controls and backend fallbacks.
Tab dragging updates lens geometry without relaying out unchanged icon rows.
The 128-case complete-frame equivalence fixture covers both styles/themes,
partial/full lift, source vibrancy, stretched/rotated/clipped geometry, translucent
backdrops and 1×/3× output (maximum one channel value difference). Empty-source
composition preserves transparent scene RGBA on Raster and Metal; the filter
fallback now replaces the already-composed scene instead of adding it twice.
Matched Simulator dragging trials reduce CPU preparation approximately 24% and
GPU command-buffer execution 17%. Optimized physical-device thermal headroom
remains unverified. The fullscreen Demo opens a System UI elements catalog with
live Slider and Switch settings and event counters.

Control and slider implementation (2026-11-06): style-independent `UIControl`
now owns event-scoped actions, weak targets, composable state and complete touch
tracking delivery. UIKit adds `UISlider` with the shared LiquidLens renderer,
measured pickup/lift/release behavior and silent programmatic setters. The Glass
appearance Demo uses a live slider and saves on release. Native comparisons
cover 35 geometry cases and six real OS-touch replays; Raster/Metal checks cover
72 theme/scale/lift/disabled cases. Screenshot comparison also corrected layer
shadow radii to scale in point space and the thumbless filled-track cap. The
slider lift now uses the native small-lens spring response/damping/retarget
impulse, a separate resting-fill opacity response, a short-tap minimum lift
interval, and lowers alongside projected value deceleration. Frame curves are
checked against real UIKit presentation-layer traces rather than screenshots
alone. Exact interrupted-contact timing, full tick presentation and platform
accessibility integration remain Partial. See [control semantics](docs/controls/README.md)
and [native slider evidence](docs/slider/README.md).

Slider shadow/retirement follow-up (2026-11-06): actual-slider glassBackground
presentation inputs now establish that material parameters animate while layer
opacity stays1. The slider retires refraction, ring/shadow, face and highlight
settings with the raw spring value; its resting shadow belongs to the fill host
and follows live capsule geometry. Lens output support no longer mistakes native
SDF padding limits for radial alpha masks, preserving the soft lower shadow tail.
Focused native-input and1×/2×/3× surface/shadow regressions accompany matching3×
optical captures. Exact inner-follower timing and private opacityPair grouping
remain Partial; see `docs/slider/lift-motion`.

Shared lens configuration and release follow-up (2026-11-06): slider lowering
now starts alongside projected value retirement rather than after it; 31 native
moving-release samples replay at 60/120Hz with maximum width residual 0.286pt.
A real UITabBar press/release capture establishes common material retirement:
face, refraction and shadow inputs share a .4 critically damped return and retain
lifted reference distances while presentation bounds shrink. This path now applies
to automatic and control-driven LensView clients, fixing the tab knob's persistent
shadow. Public configuration now includes small/large style aliases, frame/custom
resting-background constructors, readonly custom-background state, resting border
and shared UIShadowProperties, source-preservation mode and the native-shaped
four-argument lift entry. Tab bar uses the shared configuration and alongside
layout path; slider no longer edits a private resting shadow layer. Fresh defaults
match the observed native source mode0 / warpsContentBelow=false; tab bar opts
into source hiding and warping. XML supports the new usable attributes. Advanced
lifted-container relocation and alternate punchout remain unexposed, rather than
stubbed. Completion flag names and full private opacityPair/punchout-shadow
semantics remain unverified. Immediate retargeting and the two-argument convenience
are explicit portable differences; see [lens API audit](docs/liquid-glass-lens/api-audit/README.md),
[shared implementation](docs/liquid-glass-lens/shared-configuration.md), and
[moving release](docs/slider/lift-motion/release-timing/README.md).

Slider endpoint follow-up (2026-11-06): the Glass filled track now tapers its
cap width over the outer 2% of normalized travel, removing the 18.5pt endpoint
jump of the standard thumb and scaling correctly for custom thumb widths.
Thumbless tracks remain linear. Native public configured sliders retain a
minimum-origin fill rather than the previously invented bipolar neutral fill.
608 retained native layer measurements cover both themes, thumb styles/sizes,
endpoint images, disabled state, widths/ranges, ticks and neutral configurations;
maximum endpoint/track error is 0.000062pt. Continuity/monotonicity regressions
and 360 static Raster/Metal cases plus 870 motion frames pass at 1×/2×/3×.
The legacy custom-track-image provider and configured native getter/visual
snapping split remain Partial; see [endpoint evidence](docs/slider/endpoints/README.md).

Native lens configuration follow-up (2026-11-06): `LensView` now uses private
UIKit lens profiles recovered from144 fresh iOS27.0/24A434 Simulator captures:
explicit small/large styles, both appearances, six sizes and glass preferences
0/.5/1. The69-input glass background, source/backdrop displacement, seven-ray
foreground, linear edge opacity and final highlight have independent portable
translations. Lens optics retain their native independence from the glass tint
trait. Live portal identity captures distinguish selected-source displacement
from backdrop rendering. Zero-radius sampling now uses the actual warped sharp
source, avoiding a redundant LOD0 atlas; native near/deep backdrop placement
checks pass. Size-dependent distances follow presentation geometry as well as
model resizing. Resting fill and control geometry remain caller-owned; spring
specs are retained as evidence, while exact motion, physical-device/HDR and full
optical parity remain Partial. Native configuration tests cover72 independent
samples; further sampling-boundary verification is recorded with the comparison
artifacts in `docs/liquid-glass-lens/native-settings`.

Physical-device headroom follow-up (2026-11-06, validation in progress): real
iPhone presentation measurements confirm the remaining full-screen Demo GPU
bottleneck. Bounded content fades avoid full-viewport isolation only when clip
and sampling boundaries are equivalent; transformed/embedded viewports retain
the general path. Glass reductions are frozen once per frame on aligned pixel
grids, with sampling halos and frame-local ownership; fractional/affine cases
retain lazy filtering. Shared material lanes reuse their frozen source. Finite-support reductions
retain complete sampling footprints. Native materials omit only an unsampled
base atlas tile; generic shaders retain it. Frame pacing preserves its schedule
across wake jitter without replaying missed frames. The
finite phone replay supports sustained 30-second windows, thermal state,
shader-compilation counters, controlled orientation and separately enabled GPU
stage counters. Independent native blur controls retain their prior errors;
Settled selection lenses bound their destination-dependent composition and plain-source rasterization while retaining the original sampling grid; transformed/custom sources keep the general path. Image equivalence and callback controls cover both Raster and Metal. A conservative Metal known-clear reload path retains only unchanged full clears within one command buffer; format, snapshot, intervening-write and alias guards accompany pixel comparisons. Uniform pending clears avoid a scroll-mask snapshot only with validated format and quantization; nonuniform prefixes retain snapshots. Progressive edge kernels request sampling halos along their actual axis, preserving convolution and output semantics. Raster/Metal boundary comparisons pass (96 cases each, maximum 2/255 on Metal). Low-frequency memory sampling finds stable process/GPU usage, with unused cached resources retained after navigation. Idle-only cache cleanup preserves the recent working set. A strict phone idle/resume replay reclaims stale resources with zero shader compilation or idle GPU frames; a cleanup-disabled control reproduces the first-gesture drawable wait. The replay now rejects unrendered motion phases and explicitly restores its scrolling root after delayed navigation steps. Physical sustained results still fall short of stable 120 Hz under charging/Serious thermal conditions. See the
[device investigation](docs/performance-ios-20261106/iphone-headroom/README.md).

Pressed glass exterior correction (2026-11-06): press lighting now follows the glass surface coverage instead of whitening the retained exterior backdrop. A fixture reproduces the tab-bar halo (an exterior channel rises from 187 to 221 before the fix). All 432 cases pass on Raster and Metal with unchanged exterior pixels and retained interior response, covering quarter/full press, clear/regular materials, tint preferences, explicit color, themes, button/platter sizes and 1x/2x/3x. Full optical, grouped-glass, tab and navigation regressions pass. The rebuilt offline package passes 13,028 cold Simulator frames and a strict A17 Pro replay with zero runtime shader/pipeline compilations and zero failures. The corrected build is installed; see the [halo investigation](docs/glass-press-halo-20261106/README.md).

Full-screen scrolling follow-up (2026-11-06): grouped glass material programs
read each member's uniforms/samplers directly with constant indices, retaining
shared SDF geometry, ownership and touch lighting. Steady scrolling of the actual
full-screen Demo now has a dedicated simulator benchmark, optional live backdrop
appearance delivery and separate Metal GPU execution timestamps. Alternating
live-appearance runs reduce CPU recording/submission by 12.4% and synchronized
completed work by 9.7%; GPU time is effectively unchanged. Full resolution,
optical recipes and rendering cadence are retained. Twelve simulator regression
suites and the uncapped control pass, including 240 new mixed-material lane
checks across 2/3/4/6 items and reordering. This does not establish physical
120 FPS or thermal improvement. See the [measurements and verification](docs/performance-ios-20261106/fullscreen-scroll/README.md).

iOS Demo performance audit (2026-11-05): opt-in real-device replay now records
drawable presentation timestamps, asynchronous Metal command-buffer execution
times and CPU frame stages. On a warm iPhone 15 Pro Max / iOS 27.2, the earlier
baseline delivers 42.3 then 38.8 scrolling FPS; uncapping does not recover 60 FPS.
The diagnostic glass-material bypass reaches 112.4 then 111.5 FPS with scroll
pockets retained; disabling only pockets remains near 31–32 FPS. All clean runs
report Serious thermal state and Low Power Mode off. These sequential controls
identify glass rendering as the dominant cost, not a controlled thermal A/B or
a completed performance fix. Production optics and idle skips remain unchanged.
Six frame-loop/traversal regressions and the metrics analyzer regression pass.
See [device measurements and reproduction](docs/performance-ios-20261105/README.md).

Scrolling follow-up (2026-11-06): paced rendering preserves a monotonic deadline
across early SDL input wakes, retaining fractional waits and explicit uncapped
benchmark behavior. The slow-release scroll follower keeps its 120Hz request.
Glass pyramid stencils specialize their tap count and reduce at destination
resolution; fully covered body pixels skip overwritten exterior shadow work.
Matched simulator runs reduce completed navigation/scrolling work by 24.2%/25.2%.
Independent native blur errors improve; intermediate rounding can change pixels.
Physical iPhone smoothness and thermal improvement remain unverified. See the
[scrolling follow-up](docs/performance-ios-20261105/scroll-followup/README.md).

Simulator optimization follow-up (2026-11-05): sharp backdrop inputs are bounded
independently of blur-atlas addressing; native exterior glass explicitly clips
replacement to its declared output, rather than treating saveLayer bounds as a
clip. Capture reconstruction uses fixed tap-count programs and skips only
exactly zero interpolation weights; native pyramid addressing uses an equivalent
geometric sum and avoids zero-weight level reads. Kernels, material recipes,
optical parameters and idle rendering policy are preserved. Invalid display
scale traits retain the existing one-point material fallback. Final simulator
Metal/raster capture controls are byte-identical, standalone-output checks cover
1x/2x/3x, 64 native blur controls retain their error scores, and 15 rendering,
navigation, interactive-pop, scroll-pocket and module regressions pass. The
scroll-pocket suite includes 213 checks; material tests include 480 color
controls. Simulator GPU work is not evidence of physical iPhone FPS or thermal
improvement. Final warm offscreen medians improve 25.708→17.578ms for navigation
and 65.601→41.176ms for nested glass scrolling. A further 4,052 bundled simulator
frames need no new source/pipeline compilation during interaction. The signed
iPhone artifact embeds 217 freshly rebuilt libraries and 165 offline pipelines;
it is ready for installation, with phone execution intentionally deferred.
See [optimization measurements and limits](docs/performance-ios-20261105/optimization/README.md).

Liquid Glass spatial correction (2026-11-05): the refraction, radius-to-LOD,
13-tap blur pyramid, face chroma compensation, fill, bleed, ring, shadow,
holding-tone and SDF-lighting equations are independently translated from
shipped QuartzCore AIR. Inner and outer refraction sample separate spherical
rays and evaluate blur at their shifted distances. Accent matrices use the
native DesignLibrary HSL/achromatic CPU equations, verified against 224 native
matrices (including 32 holdouts) within 0.000001 coefficient error. The 560
captured material profiles remain separate from these equations. Generic Core
filter-pyramid support keeps native material recipes in UIKit. Materials with
no exterior pass retain a single host antialiased clip.
Native same-screen trait overrides also establish display-scale-specific
microcontour width and bias at1x/2x/2.5x/3x, independent of phone/tablet idiom.
The adapter resolves these before application optical overrides; 336 independent
2x material controls preserve every other native scalar input.
Earlier spatial-correction verification: 18 integration regressions pass, alongside 64 native blur
controls and 480 material-color controls (maximum channel error 1/255).
All 37,566 strict Metal replay frames load that revision's 151-pipeline archive without
runtime library/pipeline compilation. Its 200-library offline bundles build for
macOS and iOS; the Switch NRO builds. That earlier iPhone build is installed and
passes bounded strict launcher/scrolling checks before returning to normal mode.

Compatibility remains **Partial**: captured-profile interpolation is portable
policy; arbitrary OS/accessibility recipes, HDR/out-of-gamut rendering and
physical GPU precision remain outside this audit. The [spatial audit](docs/liquid-glass-native-settings/spatial-corrections.md)
records native evidence, verification and limits. Earlier translation/build
figures are historical and do not establish current spatial parity.
- UIKit-cross-platform reference: **[`ed2c290`](https://github.com/flowkey/UIKit-cross-platform/commit/ed2c290025f2dfd714ed945153069ca54e55a23e)**

## How to maintain this document

Any patch that changes a UIKit-facing public API or its behavior must update this document in the same patch. Follow the maintenance contract in `AGENTS.md`.

Statuses describe behavior, not declarations:

- **Usable** — the listed core behavior is available and has no known application-blocking defect; this does not imply every Apple overload or convenience API exists.
- **Partial** — a meaningful subset works, but important behavior, fidelity, or verification is missing.
- **Stub/unsafe** — the public surface exists but is empty, misleading, or has a known correctness/lifetime blocker.
- **Missing** — no equivalent implementation exists.
- **Alternative** — NXKit intentionally solves the need with a different, non-source-compatible API.
- **Out of scope** — intentionally excluded from the current portable-core target.

Priorities:

- **P0** — required for a dependable small application.
- **P1** — required by most production applications.
- **P2** — common advanced application functionality.
- **P3** — valuable breadth after the core is dependable.

Apple's [views and controls](https://developer.apple.com/documentation/uikit/views-and-controls), [view controllers](https://developer.apple.com/documentation/uikit/view-controllers), [UIScrollView](https://developer.apple.com/documentation/uikit/uiscrollview), [UITextView](https://developer.apple.com/documentation/uikit/uitextview), and [CALayer](https://developer.apple.com/documentation/quartzcore/calayer) documentation define the target. UIKit-cross-platform is a secondary behavioral and test reference, not the specification.

## Module ownership

UIKitCore now owns style-independent behavior and platform backends. UIKit and
NXKit are independent component kits that each link only Core; applications can
mix their components on any platform. Apple-style buttons, glass, lens materials,
and their XML registration live in UIKit. Tab item metadata and presentation modes
live in Core; UIKit owns the item-driven Apple bar and bottom tab controller. Generic backdrop shader descriptions,
compilation, and `CABackdropLayer` rendering remain in Core. Tests follow module
ownership, with mixed-kit behavior verified separately. See
[`docs/module-boundaries.md`](docs/module-boundaries.md) for migration details.
Verification: the earlier full raster/Core-only suites passed 32/13 checks;
a fresh Core/navigation/tab run against matching rebuilt Apple Skia passes 3/3.
The iOS Debug Demo and macOS Metal Demo compile/link successfully; the Switch Demo
packages an NRO.
Switch startup crash investigation and the remaining visual/exit verification are recorded in `docs/switch-launch-crash`.
Skia is upgraded to official `main` at `c733ad3e37` plus one consolidated Horizon commit (`eb958eafdc`). All seven native Vulkan/text archives and the Switch Demo NRO build successfully; renderer/text/pipeline-cache header probes pass. Matching iOS/macOS Metal and text archives are rebuilt with Chromium allocator integration disabled to match NXKit's headers, resolving the previous Apple build blockers. Prior source/dependency edits and platform build outputs are recoverable; see `docs/skia-upgrade` and its Apple build manifest.
The Apple tab controller now implements bottom-bar presentation; sidebar remains
deferred. These tab APIs remain Partial pending the broader behavior listed below.

## P0 — dependable application foundation

| UIKit API or capability | Status | NXKit evidence and remaining work |
| --- | --- | --- |
| `CGFloat`, `CGPoint`, `CGSize`, `CGRect` | Partial | `NXFloat`, `NXPoint`, `NXSize`, and `NXRect` provide common geometry and transforms. `NXRect::operator!=` now complements equality, with changed/unchanged rectangle coverage. Positive `insetBy` currently expands rather than contracts a rectangle; this existing convention remains incompatible with UIKit insets. Names and portions of Core Graphics behavior differ; add edge-case tests for null/empty/infinite geometry, standardization, intersection, containment, and pixel alignment. |
| `UIEdgeInsets` | Partial | Basic value type and arithmetic exist. Verify equality, inset operations, direction-aware edges, and UIKit-compatible defaults. |
| `CGAffineTransform`, `CATransform3D` | Partial | `NXAffineTransform` and `NXTransform3D` support common transforms and animation. Affine inversion now reverses translation, accepts reflections and rejects singular/nonfinite matrices; round-trip tests cover translated, rotated and sheared contact coordinates. Expand conversion, concatenation, decomposition, and numerical tests. |
| Safe object ownership | Stub/unsafe | UIKit-style objects depend on the custom `new_shared` ownership path. Stack allocation or `std::make_shared` can create competing owners. Replace `SharedBase.hpp` with standard ownership before declaring the object model dependable. |
| Main-thread UI confinement | Partial | A main `DispatchQueue` exists, but UI objects have no enforcement or assertions comparable to `@MainActor`. Add a UI-thread contract and debug checks. |
| `UIApplicationMain`, `UIApplication`, `UIApplicationDelegate` | Partial | Portable `overrideGlassTintAmount()` / `setOverrideGlassTintAmount(optional)` and effective `glassTintAmount()` support per-app settings without writing system preferences; persistence belongs to the application. Launch, quit, foreground/background callbacks, SDL event handling, and one global application exist. iOS startup no longer forces a 1.3× logical coordinate multiplier: window/layout/touch coordinates and safe areas retain native UIKit points, with the drawable-to-point ratio applied once during rendering (3× on iPhone 15 Pro Max). A real-startup raster regression covers 2×/3× portrait/landscape bounds, scale traits, safe areas, rendered 44-point controls and normalized SDL touch delivery; physical-device verification remains pending. The build exports `SK_RELEASE` to match the checked-in release Skia archives, preventing incompatible inline `SkRefCnt` disposal semantics at shutdown. Apple and Switch teardown snap and synchronously finish pending recorder work, then let the recorder abandon tracked devices and destroy its promoted-image provider before the Graphite context. Switch now uses native Graphite Vulkan with a statically linked Horizon NVK driver, explicit acquire/present synchronization, VMA unified-memory allocation and orderly Vulkan teardown before same-process homebrew-launcher handoff. Startup prewarms common/captured pipelines; a device shader-compilation target bundles a matching persistent cache and requires a fresh-install validation pass before publishing. Hardware cold and bundled-only runs each complete all 28 bake stages, replay 51 serialized keys on the cached run, present frames, save the 2.45 MB native cache over existing SD files and return to Sphaira. A checked-in Skia patch initializes builtin runtime-effect metadata during fresh-process key replay; SD replacement preserves a recovery file and rolls back on failure. The verified package includes the native cache, key seed and attribution (see `docs/switch-vulkan`). Switch builds now preserve devkitPro’s software TLS compiler flags across reconfiguration, correcting a hardware-confirmed first-frame crash in the render guard. A subsequent hardware run exposed invalid glass-rim WGSL; renaming its builtin-colliding parameter avoids that shader-generation failure. The C/C++ TLS configure/reconfigure harness passes. Quit/release handling detaches press sources before callbacks, preventing a B-button Exit action from invalidating its caller’s map iterator; a focused regression reproduces the former heap use-after-free under ASan and passes after the fix. The verified Vulkan NRO is installed on SD; normal Demo startup loads 51 saved keys and presents at least 600 frames. The user confirmed visible UI and responsive controls on the physical Switch. See `docs/switch-launch-crash`. Two consecutive Vulkan bake runs verify the corrected Switch shutdown and launcher handoff. Complete lifecycle ordering, activation semantics, error handling, and reproducible tests. |
| `UIWindow` and root controller | Partial | Default tint remains system blue through UIView inheritance rather than an explicit window override, allowing component kits to distinguish default versus app-configured tint. Root-controller installation, key visibility, event dispatch, safe area, traits, and focus exist. Desktop live exposes now lay out and present synchronously during the SDL/AppKit modal resize pump, with no recursive event/dispatch processing or pacing sleep. Event-driven pacing wakes without consuming input. iOS samples a private UIKit-coordinated bounds probe once per frame, keeping logical geometry, interpolated safe areas, native density and drawable pixels consistent through rotation. Ordinary changes apply immediately; animation duration/easing come from the native coordinator. Four simulator rotations with actual Metal drawables verify intermediate dimensions, constant scale and reference-curve agreement; native Cocoa and 2×/3× startup/resize tests cover immediate updates and reentrancy. See `docs/resize-rotation`. Physical-device rotation/energy and full macOS Graphite resize verification remain pending; clarify key-window semantics, multiple-window support without adopting a public `UIScene` API, and teardown. |
| `UIResponder` and responder chain | Partial | Touch and press forwarding exists, hardware presses start at the focused responder or the active presentation root when focus is empty, and responders can register semantic, input-grouped executable actions with enabled state, input matchers, dynamic `canPerform` availability, same-responder priority, and explicit repeat policy through their `UIAction`. Repeated input skips automatic/non-repeatable actions and may fall through to a lower-priority or later-responder repeatable action; regular unavailable matches likewise fall through, while an available disabled regular match still consumes the press so parent fallbacks cannot run accidentally. Ancestor responders can temporarily gate action dispatch for their subtree without changing the responders' own enabled state; `NXNavigationController` uses this during transitions, and `NXResponderAction` supplies typed console-button registration and collection without placing console button types in `UIKitCore`. `UIViewController::show` also propagates primary-context presentation requests through parent containers before falling back to modal presentation. String selector discovery/dispatch now bridges control targets to named responder commands with cycle protection during nil-target lookup. First-responder management, detail-context presentation and broader chain semantics remain incomplete. |
| `UIAction` and menu-leaf repeat policy | Partial | Closure-backed actions provide stable identifiers, title mutation, weak sender-aware dispatch copies and guarded execution. Copies retain identifiers; same-ID control registrations replace the handler and union event masks. `UIMenuElementRepeatBehavior` mirrors UIKit's `automatic`, `repeatable`, and `nonRepeatable` values; the current menu-less subset exposes `repeatBehavior` directly on `UIAction`, and automatic is conservatively non-repeatable for hardware input. Add images, attributes/state, the `UIMenuElement` / `UIMenuLeaf` hierarchy and menu integration. |
| `UIEvent`, `UITouch`, presses and keys | Partial | Touch and gamepad/press events exist. UITouch now exposes tapCount; explicitly supplied counts survive window routing, and SDL receives a same-view 300ms/22pt isolated-tap fallback for touchDownRepeat (portable policy, not measured Apple thresholds). Held keyboard and gamepad presses use Borealis's 250 ms initial threshold and 100 ms repeat interval; repeat deliveries carry `UIPress::isRepeat()`, directional focus always accepts them, and actions must explicitly opt in. SDL's platform keyboard repeats are ignored so supported backends share one cadence. Physical key and gamepad-button downs are idempotent, thumbstick direction changes and direct returns to neutral balance their press lifecycle, and held presses are cancelled on controller removal, backgrounding, or window focus loss so a missed release cannot leave a repeating command behind. `UIPressPhase::cancelled` now reaches the responder chain without performing release actions. Keyboard and gamepad dispatch no longer depends on renderer geometry that is only needed for pointer translation. SDL finger-cancel events now carry a distinct cancelled phase through recognizers and the hit view instead of being treated as successful touch endings. The SDL3 Switch backend preserves finger identity across polls and emits balanced down/motion/up events; the full Switch Demo `.nro` build verifies backend integration, but repeat cadence and touch behavior still need manual hardware confirmation. Recognizer terminal cleanup now tolerates a touch already removed by callback-driven reset/disable, covered by live tab-list replacement during routed touch-up. Complete timestamps, multi-touch identity, modifier/key semantics, mouse input, broader cancellation cases, and deterministic tests. |
| `UIView` geometry and hierarchy | Partial | `removeFromSuperview()` retains its receiver through hierarchy/mask detachment and final invalidation, including a first child owned only by its parent; focused removal/discovery regressions pass.  Frame/bounds/center, subviews, superview, hit testing, conversion, alpha, hidden state, backgrounds, alpha masks, tint and content modes exist. Newly attached views now inherit their superview's trait collection before receiving trait-change callbacks, including subtrees created dynamically after the window is visible; mask views participate in display/layout and trait/tint propagation, descendant traversal correctly advances through each ancestor, and removal now detaches the Yoga node immediately so reparenting before the next layout is safe. Fix index validation, remaining hierarchy-cycle checks, invalidation, conversion through transforms, and insertion parity. |
| `UIView` display and invalidation | Partial | Normal application loops skip drawable acquisition, GPU rendering and presentation when the retained layer tree is unchanged, following the renewed user request. Event/dispatch delivery, layout, animation advancement and backdrop-read completion continue; visual changes and platform exposes resume rendering immediately. `NXKIT_CONTINUOUS_RENDERING=1` or `NXKIT_UNCAPPED=1` preserves continuous benchmark compositing. `setNeedsDisplay()` still controls view/layer content regeneration, and missing content invalidation is not repaired by continuous compositing. Layout settles before view contents regenerate; newly generated contents/density reach an active presentation in the same frame while explicitly animated density retains its interpolated value. Clean view visits no longer copy tint/trait drawing environments; layout refreshes configured corners once per visited view. Dirty descendants, draw-time sibling invalidation, inherited tints and caller environment restoration are covered. The startup regression verifies zero acquisitions/presents over 120 clean iterations, active animation wakeup/settling, visual mutations, unavailable-drawable retry, exposes and current display-link pacing at 2×/3× in both orientations; continuous/uncapped variants verify every-frame presentation. Audit remaining setters and display callbacks before treating this as usable. |
| `UIView` layout lifecycle | Partial | `setNeedsLayout`, `layoutIfNeeded`, `invalidateIntrinsicContentSize`, margins, safe areas, sizing, and controller layout callbacks exist. `sizeToFit` now dispatches virtually so fixed-size controls preserve their UIKit sizing through UIView pointers. Intrinsic invalidation dirties Yoga's cached measurement for measured leaf views. Layout now resolves the complete subtree before display, guards reentry, snapshots hierarchy membership and settles callback/style/safe-area/margin invalidations with bounded retries. Yoga applies changed bounds through UIView, preserves nonparticipating manual children and uses per-node current-scale pixel rounding. Initial window layout precedes root did-appear/focus; an animation resample at the same timestamp keeps current layout in active presentation layers. Actual framebuffer tests verify identical first/second startup pixels, 24 consecutive resize samples, fractional UI scale changes, 1x/2x/3x rounding, visible fades, hidden/revealed views, inherited environments and callback mutation. Full constraint-layout compatibility and broader intrinsic/lifecycle edge cases remain incomplete. See `docs/layout-pass`. |
| Auto Layout (`NSLayoutConstraint`, anchors, guides) | Alternative | Yoga flex layout is the current portable layout system. Document the intentional incompatibility. Add `UILayoutGuide`-like safe/readable guides only if they improve portable layout; do not claim Auto Layout compatibility. |
| `CALayer` geometry, hierarchy, and visual properties | Partial | Bounds, position, anchor point, affine/3D transform, z-order, opacity, background, rounded corners, borders, shadows, contents, hierarchy, alpha masks, and presentation copies exist. Rendering rejects offscreen subtrees only when `masksToBounds` bounds all descendant output, and skips stable sorting for already ordered children while retaining a snapshot for drawing callbacks. Raster/recording tests cover overflowing descendants, transformed clips, scroll origins, reparenting, equal/mutated z-order and callback removal. Bitmap contents use linear resampling under fractional placement and transforms; whole-pixel 1:1 coverage remains exact. Retained masked/unmasked regressions verify smooth subpixel motion at 1×/2×/3×. `CALayerCornerCurve::{circular,continuous}` with `cornerCurve()`/`setCornerCurve()` selects a shared native-calibrated shape for backgrounds, clipped content, inside borders, shadows, blur and glass. Continuous is the explicitly requested NXKit default; circular is available. The discrete curve is copied, invalidates display and reaches an already active presentation; animated bounds/radius recompute geometry. XML validates both names. A bounded per-thread geometry cache retains the rounded-superellipse/circular-arc reconstruction adapted from Flutter under BSD-3-Clause. The same precomputed octants drive path clipping, optical normals/distances, merged items, rim/shadow contours and lifted-lens sampling. Fresh iPadOS 27.0 solid masks equal the retained 26.4.1 masks byte for byte; maximum-radius capsule mask MAE falls 0.742→0.210 and 1.150→0.222/255. All 40 fixed regular-glass conditions improve across both themes, white/gray/blue/striped backdrops and five sizes. Independent native transition coordinates and tall/wide/tiny-radius path/distance tests supplement the retained coverage regressions. See `docs/liquid-glass-corners/continuous` for exact overlays, provenance and remaining private-curve/optics limitations. Native solid-layer captures at five radii/sizes and raster regressions cover AA at 1x/2x/3x, fractional placement, rotation, masks, inside borders, invalidation, XML and presentation copies. UIKit's exact private spline and end-to-end GPU parity remain unverified; `maskedCorners` and the `cornerCurveExpansionFactor` query remain missing. See `docs/liquid-glass-corners`. Fix incomplete insertion, invalidation, contents rectangles/centers, coordinate conversion, and remaining safe-hierarchy mutation gaps. |
| Core Animation actions and transactions | Partial | `CAAction`, `CABasicAnimation`, spring animation, timing functions, model/presentation layers, implicit actions, and basic transactions exist. Add robust begin/commit nesting, completion semantics, repeat/autoreverse, fill behavior, additive/keyframe/group animation where useful, interruption, and tests. |
| `UIView` animation APIs | Partial | Duration, delay, options, spring parameters, completion, and without-animation blocks exist. `beginFromCurrentState` samples the currently visible presentation-layer value when replacing an in-flight property animation, replacement installs the new animation before invoking the old completion, and a multi-property animation now reports `finished == false` if any property was interrupted. Touch hit testing and responder actions share the same per-view `allowUserInteraction` gate, so hardware actions do not bypass a running animation's interaction policy. Verify nested blocks, broader interruption/removal completion delivery, inherited options, and overlapping-animation interaction semantics. |
| `CADisplayLink` | Partial | A callback is registered with the main dispatch loop and can be invalidated; background dispatch queues do not execute UI display links. Tick snapshots now retain cancellable invocation state rather than raw display-link pointers, and the current callback survives self-destruction. Removing another scheduled link skips it safely; newly created links wait for the next tick. The callable is retained without copying its mutable state between ticks. Main-thread, self-removal, stateful callback and callback-driven replacement regressions pass under AddressSanitizer/UndefinedBehaviorSanitizer. The lifetime defect was reproduced during the Switch Vulkan shader bake. Add timestamp/duration, preferred rate/range, pause, broader thread behavior and frame-drop tests. |
| Controller container discovery | Partial | `UIViewController::navigationController()` and `tabBarController()` find the nearest actual ancestor implementing the Core navigation/tab capability interface, without loading views or retaining ancestors. NX containers establish containment when configured, including before view loading; removal/replacement clears the relationship. Core-only custom-container and mixed-kit integration regressions cover lookup, nested wrappers, reparenting, ancestor destruction, push/pop, and tab selection. Interfaces preserve Apple operation names; interface return types, optional empty selection, and bool selection setters are deliberate C++ differences. Core also exposes the optional readonly `interactivePopGestureRecognizer()` capability. UIKit installs an enabled leading-edge gesture; NXKit defaults to no installed gesture (null), without depending on UIKit. |
| `UIViewController` view loading and lifecycle | Partial | Lazy view loading, appearance callbacks, title, safe-area callbacks, traits, responder participation, adaptive primary-context `show` forwarding, modal relationship accessors, and being-presented/dismissed state exist. View insertion no longer synthesizes duplicate appearance callbacks; modal completions follow the final appearance callback; a built-in presentation reversed before `viewDidAppear` now proceeds through `viewWillDisappear` / `viewDidDisappear` without synthesizing `viewDidAppear`, while a full-screen presenter's pending disappearance is balanced by `viewWillAppear` / `viewDidAppear`; and NX containers explicitly forward transitions only to visible children. Add stricter callback-mutation, broader canceled-transition cases, repeated-presentation, memory/unload, detail-context, and interactive appearance-coordination tests. |
| View-controller containment | Partial | `addChild` rejects null children, existing parents, self-containment, and ancestor cycles; removal completes the move callback without incorrectly synthesizing disappearance. `NXNavigationController` retains its stack while mounting only top content. `NXTabBarController` retains its configured children in regular/sidebar-list mode; opening a compact tab transfers that actual controller to the enclosing navigation stack (or standalone modal presentation), then returns it on Back or expansion without rebuilding its view. Regression coverage verifies attachment, removal, nested show routing, stack changes, selection, and adaptive transfers during animation. Add broader ordering, reparenting, callback-mutation, and animated-transition tests. |
| Modal `present` / `dismiss`, `UIPresentationController`, and custom transitions | Partial | Modal relationships resolve through containment to the actual presenter; every presentation-root controller supplies a low-priority default Menu/B Dismiss action, including when its focus hierarchy is empty, while higher-priority local and container actions can intercept first. Full-screen, over-full-screen, and custom presentation styles retain a presentation controller through dismissal; full-screen transitions remove and restore the presenting view; custom presentation controllers control the final frame, chrome lifecycle, and relayout, including for Yoga-backed presented roots whose container-owned origin must remain stable; and focus follows the top presentation as soon as it is installed, before its animation completes. `UIPresentationController` participates in the focus-environment chain, and the top presentation is a hard focus boundary: directional and explicit focus updates cannot reach its presenter, while a presentation with no eligible item leaves focus empty. `UIModalTransitionStyle` provides cover-vertical, horizontal-flip, cross-dissolve, and a portable 2D partial-curl approximation that folds the presented view without reordering the live presenter. Partial curl requires full-screen presentation style but uses the presentation container's bounds instead of rejecting transient or container-managed presenter geometry. Built-in modal animators request the 120 fps frame limit used by navigation transitions, opt into interaction so early dismissal input stays in the incoming hierarchy, and use `beginFromCurrentState`. Dismissal during an in-flight built-in presentation cancels that presentation and immediately animates back from the current presentation-layer transform/opacity while retaining relationships, focus ownership, and balanced appearance callbacks until dismissal completes. Other overlapping operations remain serialized per presentation stack; repeated presentation input while a controller is already presented and queued requests that no longer apply are ignored without corrupting relationships or throwing. Custom transition actions are consumed without executing when their animated view omits `allowUserInteraction`; custom transitions remain serialized until an interruptible property-animator contract exists. `UIViewControllerTransitioningDelegate`, `UIViewControllerAnimatedTransitioning`, and a non-interactive `UIViewControllerContextTransitioning` subset support separate presentation/dismissal animators with correct completion timing. Dismissing a lower controller removes its whole presented stack while animating the top controller. The navigation regression harness covers built-in full-screen/overlay/partial-curl hierarchy behavior plus custom framing across Yoga layout, animator/context use, relationships, lifecycle hooks, completion, focus trapping, 120 fps scheduling, transition-time input ownership and allowed/blocked action routing, immediate presentation reversal from an intermediate visual state, overlapping-operation serialization, repeated-action safety, and modal-aware visibility; the scrollable Demo playground exercises all built-in styles, over-full-screen behavior, and a safe-area-centered glass alert whose height is measured from its Yoga content and whose card transform uses ease-in-out timing without spring overshoot. Add interactive/custom transition interruption, transition coordinators, adaptive presentation/delegate behavior, current-context and sheet/popover styles, dismissal-prevention policy, broader cancellation/order tests, and a true renderer-backed page curl before treating the full UIKit surface as usable. |
| `UIColor` and dynamic colors | Partial | `resolvedColorWithTraitCollection()` returns an explicitly resolved value; Apple materials use it without friendship into Core internals.  RGBA colors, many semantic colors, alpha changes, tint and dark/light dynamic providers exist. Eleven chromatic system colors now use independently resolved UIKit 27.0 light/dark values (including systemBlue 0/136/255 and 0/145/255), rather than the older palette; systemGray is unchanged. The public probe and measurements are in `docs/liquid-glass-colors/system-colors.json`; Display P3, accessibility contrast and older-version palette selection remain unimplemented. View drawing now establishes a non-null view trait context for dynamic-color resolution and restores the surrounding context after each subtree. Add color-space-independent semantics, more system colors, and broader tests. Pattern colors are not core priority. |
| iOS 26 `UICornerRadius`, `UICornerConfiguration`, `UIView::cornerConfiguration` / `effectiveRadius` | Partial | Core provides fixed/concentric/minimum radii, independent corners, capsules, uniform sides/edges, combined `UIRectCorner` maximum queries, XML, logical-point invalidation, reparenting and implicit per-corner presentation animation. Native iOS 27 public-API measurements verify asymmetric insets, uniform maximum selection, fixed-radius clamping and transparent versus explicitly configured wrappers; legacy layer radii remain independent. iOS 26+ samples each native SDL-window/root corner using public concentric configuration and effective-radius queries; the explicitly requested pre-26 fallback uses guarded `UIScreen::_displayCornerRadius`, verified by executing the production bridge on iOS 18.5. Both native fixtures return 62pt and the modern bridge matches an independent public probe. macOS/Switch default to square window corners. UIKit navigation uses temporary wrapper card clipping for animated push/pop and interactive commit/cancel without replacing application-owned view styling; cleanup restores transparent corner inheritance. Focused Core raster, existing corner, navigation and interactive navigation tests cover 1×/2×/3×, clipping, model translation, XML, animation and cleanup. Arbitrary scale/rotation/3D container parity, merged-material independent corners, uneven optical normals, physical-device visuals and mid-transition screen-shape changes remain unverified/incomplete. Core-only module/configuration tests pass; two actual Metal profiles render 34,994 full-window frames with no new pipeline compilations in the last two passes. The regenerated catalog has 68 libraries and 55 descriptors; the standard iOS Demo builds and embeds the matching offline Metal libraries successfully. See `docs/concentric-corners`. |
| Trait environment and `UITraitCollection` | Partial | Portable `glassTintAmount()` and `traitCollectionWithGlassTintAmount()` add a validated 0…1 glass preference, with 0.5 selecting the regular midpoint. Application overrides are optional; system defaults on Apple 27+ and the portable midpoint propagate through existing and future views/controllers, masks and appearance-derived trait copies before frame layout. Clearing an override resumes current system following. The native bridge is private and guarded; the material now follows independently captured iOS 27.1 preference/style/color controls. Regular glass retains its neutral body and explicit accents at 0%; clear style preserves its body response while frost changes. The 480 flat-color controls across five preferences, two styles, neutral/blue tints, both themes, two sizes and six backgrounds pass with mean channel error 0.17/255 and worst residual 7/255 after the full native-profile translation. Textured framebuffer comparisons cover all 24 requested endpoint/midpoint combinations; optical rims and fine blur remain approximate. See `docs/glass-tint-preference`. `UIUserInterfaceSizeClass::{unspecified,compact,regular}` and read-only `horizontalSizeClass()`/`verticalSizeClass()` are implemented; standalone values start unspecified and copies preserve both axes. iOS reads native SDL-window/root-controller UIKit traits, including orientation/multitasking classification, independently of UI/pixel scale. macOS/Switch use documented logical width 600 / height 480 cutoffs, with invalid/empty axes unspecified. Initial traits arrive before root loading and ordinary descendant `viewDidLoad`; the application loop compares and publishes class/style/scale/glass-preference changes before layout even at unchanged bounds, retaining stable previous snapshots and suppressing unchanged notifications. Traits reach window chrome, controller-owned roots, children, ordinary views/masks, presentations and newly attached/reparented content, including Glass appearance copies. Layout is invalidated and propagation snapshots tolerate hierarchy changes during callbacks. Focused headless tests exercise the actual application frame, resize/rotation, native overrides, inheritance, previous values, layout and lifecycle; existing navigation and glass/corner/gallery tests pass. iOS Demo/trait targets build/link, but native rotation/multitasking execution and macOS/Switch GPU builds remain unverified. Draw/layout traversal scopes the current collection; add thread-local current traits, `performAsCurrent`, explicit subtree trait overrides/composition, layout direction, preferred content size, accessibility contrast, interface idiom where portable, and modern trait registration/dependency tracking/transition ordering. See `docs/SIZE_CLASSES.md`. |
| `UIImage` / `CGImage` | Partial | Images load from path, resources, and data; scale and template rendering exist. `UIImage::systemImageNamed(name, pointSize, weight, scale)` loads native monochrome SF Symbols on iOS/macOS using CSS-style weights, native point metrics and explicit raster density, returns null for unavailable symbols/platforms, and rejects invalid geometry. The full-screen Demo verifies navigation/tab symbols and scale-dependent pixel dimensions; broader symbol configurations, variable/multicolor rendering and catalog availability parity remain incomplete. Raster images are promoted through a bounded Graphite image-provider cache on every supported backend. Add orientation-independent image metadata, resizing/cropping, cap insets or a portable equivalent, decoding errors, and cache/lifetime tests. |
| `UIImageView` | Partial | Basic image display, tint/template mode, content mode, and fitting exist. Add highlighted/animated images only after core invalidation and state behavior are dependable. |
| `UIFont` and font descriptors | Missing | Labels accept only numeric size and weight. Apple backends now resolve the actual UIFont/NSFont system face at the requested point size/weight, and the SkParagraph font collection registers that resolved face instead of ignoring it during family rematching. Normal AAT tracking is restored from the font’s own size table; custom-data managers retain their fallback. `SkiaCtx::getDefaultTypeface` is now virtual with an optional point-size argument, default 17. Add a portable `UIFont` facade for family, size, weight, traits, metrics, fallback, and custom font registration. |
| `UILabel` | Partial | SkParagraph/ICU shaping, text, color, size, weight, alignment, measurement, and rendering exist; default-family resolution also supports platform custom-data font managers. Labels now center the primary typographic line box and round block/origin/baselines at display scale, rather than centering font-wide extrema or rounding baselines to whole points. Taller emoji fallback retains the primary line box. UILabel disables SkParagraph's whole-point width flooring, preserving fractional layout widths so Retina-fitted captions do not wrap; 24 scale/weight/word combinations verify fit stability and genuine wrapping, and measurement works before window traits arrive. Empty labels measure zero; intrinsic sizes round to display pixels without extra one-point width padding, and scale changes invalidate measurement. Apple plain text uses unsmoothed grayscale coverage with no contrast/gamma boost; UILabel, CATextLayer and UITextView share that policy. Numeric size/weight inputs reject nonfinite values, and XML now accepts `fontWeight` through the same setter. Native UILabel captures at 1x/2x/3x in both themes include identical-file Roboto controls, system regular/semibold/bold, multiline, Unicode fallback and empty content; actual retained tests cover sizing, real weights, shared raster coverage and validated AAT tracking. See `docs/text-rendering` for measurements and remaining platform-font/multiline/fallback differences. iOS Demo builds/links; no physical-device or Switch pixel parity is claimed. Text and font mutations invalidate Yoga's cached intrinsic measurement, allowing live labels such as the navigation clock to grow without wrapping, and the old text-setter debug logging is removed. Expose and implement line count, wrapping, truncation, attributed text, baseline behavior, font objects, fuller intrinsic-size parity, and accessibility text. |
| `UIControl` target/action and state | Partial | Core now implements UIKit option-set masks/state, weak target/action and identified UIAction registrations, partial removal/introspection, responder-chain dispatch, tracking hooks, touch/repeat/drag/enter/exit/up/cancel events, reentrant mutation and ancestor-interaction cancellation. Actual touch tracking now follows shared Window responder admission rather than an eager private gesture, preserving scroll delays and cancellation; portable provider hooks keep early visual feedback independent of tracking. Explicit programmatic dispatch works while disabled; disabled hit testing rejects input and manually assigned flags survive disabling, matching native probes. Focus/held-button behavior remains portable. String selectors, value-copy actions and LTR leading/trailing resolution are documented C++ differences. OS menus/tooltips/symbol effects require missing infrastructure. See `docs/controls/README.md` and native comparison evidence in `docs/slider`. |
| `UIButton` | Partial | Lives in the optional UIKit module, including style XML parsing and the former default UIControl focus/highlight visuals. Mixed-kit tests exercise it inside NX navigation/tab content.  A title label, image view, four visual styles, Yoga layout, focus and primary action exist; inherited control tracking is cancelled when an overlapping scroll pan succeeds. Add per-state title/image/color, content alignment/insets, configuration/state updates, per-state disabled appearance, button-specific event semantics, and intrinsic sizing. |
| `UIGestureRecognizer` base | Partial | Tracked input, continuous/discrete prevention, simultaneous recognition, cancellation, terminal reset and shouldBegin are implemented. Weak explicit failure requirements reject null/self/cycles; dynamic delegate/subclass failure ordering and touch/event/press admission filters arbitrate pending recognition through an entire event batch before reset. UIKit-default delaysTouchesBegan=false / delaysTouchesEnded=true buffer UIView touch delivery with original contact identity, phase, location, timestamp and matching event membership; resolution flushes outside events as well, and recognized cancellation precedes actions. Removal/reparenting, disabled recognizers, reentrant callbacks and exception-safe cancellation have focused tests. Focused press recognizers now receive the actual Window press hierarchy, but the delay flags do not yet buffer hardware/controller press responder delivery. See `docs/controls/scroll-arbitration`. |
| `UITapGestureRecognizer` | Partial | Single-touch tap/press recognition exists. Add tap count, touch count, movement/duration thresholds, cancellation behavior, and tests. |
| `UIPanGestureRecognizer` | Partial | Positive validated minimum/maximum touch counts, centroid tracking across overlapping fingers, membership rebasing, continuous recognition below the minimum after beginning, final-translation retention, vector conversion through affine transforms, and deterministic two-sample velocity estimation are implemented. `UIGestureRecognizer::numberOfTouches()` reports tracked contacts. Public UIKit traces cover one/two-finger motion, handoff, minimum/maximum counts, translation reset and release velocity; focused routed-touch tests additionally cover a 40-contact relay and scaled/rotated vector conversion. SDL touch delivery now preserves queued motion packets and their sample intervals instead of dropping events drained within 5 ms; a routed SDL-burst regression covers 4 ms packets. Visual following stays outside recognition, so measured pan translation and velocity remain unchanged. Exact deferred action timing/coalescing, dynamic count changes, perspective transforms and physical-device cancellation remain unverified. See `docs/scroll-physics` and `docs/drag-smoothing`. |
| `UIScrollView` basic scrolling | Partial | UIKit-shaped delaysContentTouches/canCancelContentTouches defaults, touchesShouldBegin and touchesShouldCancelInContentView hooks are implemented through a finite weak 150ms delayed-began recognizer and shared Window buffering. Admitted content honors cancellation policy; controls remain immediate without competing scroll capacity. The portable `dragSmoothingEnabled` policy defaults on for Switch and off elsewhere, with a public setter/getter and XML attribute. A shared `DragSmoother` follows measured targets per frame at subpixel precision; slow release settles pending distance, while fast release starts existing inertia from the displayed offset with unchanged gesture velocity. Routed-touch tests cover release, reentrancy, programmatic rebasing, disabling/removal, viewport changes and indicator cleanup. The Switch NRO passes both 28-stage shader-bake runs and normal startup/presentation with the baked cache. The user confirmed smoother physical-device movement with acceptable lag. The subsequent B-button exit crash was traced to press-release callbacks clearing a map before its iterator was erased. Completed inputs now detach before callbacks and events stay alive through delivery; button/axis/keyboard release, same-source replacement and reentrant cancellation regressions pass, including ASan/UBSan. On 2026-11-03, the installed Switch build completes an eight-screen routed-touch fixture and exits through an actual SDL B-button release without the direct-quit fallback; the user confirms Sphaira is open. Final crash-report readback and refreshed bundled shaders await FTP restart; see `docs/drag-smoothing`. `panGestureRecognizer`, `isDragging`, and `isTracking` are exposed. Multi-contact pans, catching inertia without restarting it, inverse rubber-band rebasing on bounce re-grab, velocity gating, independent analytical axis motion and geometry retargeting/clamping now have native UIKit replay and deterministic regression coverage. Viewport changes repair stretched offsets; fractional inset edges settle exactly; programmatic updates cancel stale physics even at an unchanged offset. Fast inward releases from overscroll retain uncompressed momentum and transfer from elastic motion to free deceleration at the solved boundary crossing; twelve additional native top/bottom reversal cases and routed-touch/frame-subdivision tests cover shallow/deep pulls, slow returns and already-crossed releases (`docs/scroll-physics/reversal`). Fast outward releases now preserve excess momentum beyond an edge instead of dropping it entirely; existing stretch supplies the spring return component. Free-flight impacts use the separately measured 11/s critical spring, while passive pulls retain their existing return and inward reversals retain their calibrated handoff. Thirteen new iOS26 captures cover both edges, inside/near/already-crossed/deep releases and 40–400ms holds; the four free-flight/near-edge peak stretches match within 0.34pt. Full waveform errors remain 3.5–11pt in several cases because activation/coalescing still differs; this is not complete native parity. Four-edge routed-touch regressions cover the lost-momentum bug, gentle deep pulls and disabled bouncing. Clamping/rebasing a drag after geometry changes now clears stale scroll-release velocity. All 26 preceding native replay checks still pass; see `docs/scroll-physics/impact`. Spring re-grab fidelity remains approximate (about four points RMS in the retained cases), and activation/coalescing and long native relay input limitations remain documented in `docs/scroll-physics`. Offset, explicit validated `contentSize`, `adjustedContentInset`, insets, axis-specific bouncing, safe-area adjustment, pan, deceleration, spring-back, `isDecelerating`, and `isScrollEnabled` exist. Scroll views are focusable when scrolling is enabled and expose the portable `UIScrollViewFocusTrackingMode` extension: natural mode exhausts fully visible descendants before scrolling at a constant rate and transfers focus as incoming items become fully visible; centered mode animates each focused descendant toward the inset-adjusted viewport center without exceeding scroll limits; focused mode scrolls only the minimum distance required for full visibility. All focus modes keep their visibility viewport outside physical safe-area occlusion even when `contentInsetAdjustmentBehavior` is `never`. Programmatic focus requests targeting a scroll view or a controller whose preferred root is a scroll view resolve through to its visible preferred descendant when one exists; childless subclasses still resolve to the scroll view itself. Natural scrolling uses a separate internal exact-item request when it must temporarily hold focus on the scroll view. It reads the focus system's authoritative physical-direction state instead of depending on a particular descendant responder to forward begin/end events, so controls that consume a press and controls removed before release cannot reject or strand scrolling. When no descendant exists in the held direction, focus remains on the scroll view instead of jumping backward to the first visible item. Both axes are supported, and focusable childless subclasses such as long `UITextView` content consume repeated directional scrolling until an edge. A scrollable axis retains release momentum even when bouncing is disabled; the bounce flags control rubber-banding and the boundary spring only. Disabling scrolling gates the pan recognizer and active deceleration while preserving programmatic offsets. Existing callers that do not set `contentSize` retain the historical first-subview inference as a compatibility fallback; migrate them to explicit sizes, then remove that divergence. |
| Content scroll-view selection | Partial | `UIViewController::setContentScrollView(scroll, edge = all)`, virtual `contentScrollViewForEdge`, and `NSDirectionalRectEdge` support independent weak bindings and nil reset. Containing bars resolve the first-subview chain, including a scroll root; they do not search later siblings or load an unloaded view. A direct nil override disables observation. Core regression tests cover first-branch changes, explicit placement, expiry, overrides and validation. The base public getter returns only explicit bindings, as observed on iOS 26. A forwarding C++ override preserves automatic fallback; calling the base then unconditionally returning nil cannot be distinguished from that forwarding case. Portable leading/trailing getter support extends the iOS 26 runtime, whose getter accepts only top/bottom. |
| `UIScrollView` edge effects | Partial | Stable per-edge `topEdgeEffect`, `leftEdgeEffect`, `bottomEdgeEffect`, `rightEdgeEffect` metadata exposes automatic/soft/hard style and visibility, plus the portable `none` style. UIKit resolves automatic bottom edges to none (iOS 27 policy); other attached automatic edges remain soft. Explicit soft/hard re-enable the bottom; none removes blur, tint, separators and fading on every edge independently of `isHidden`. Core retains automatic metadata and NXKit retains shared hard defaults, while honoring explicit none. The full-screen Demo cycles Automatic/Soft/Hard/None. Private per-screen `UIScrollPocket` hosts follow ordinary/interactive UIKit navigation and tabs. Soft pockets now use a custom spatially varying Gaussian kernel: local sigma is the 4pt maximum times gradient alpha, with variable directional kernels. At 1x/2x and for narrow 3x kernels, sampling stays at display resolution; broader 3x kernels use a prefiltered lower-resolution first pass and fused finishing pass, compensating input/resampling variance. Gradient-axis-first ordering preserves a common local sigma across the second pass; viewport sampling clamps at live boundaries and zero radius leaves the sharp source unchanged. The separate fixed 0.4-to-1 content fade remains independent of content coverage/luminance. Its cached gradient now approximates a quintic eased recovery through the live boundary; monotone cubic joins smooth the independent radius profile without weakening title-band blur. The kernel now uses its own progression: it starts growing at the live inner boundary and is already substantial behind the navigation title, retaining the same 4pt outer maximum. This corrects readable content overlapping the title when the fade recovery was also used for radius. This replaces the masked uniform-blur approximation, which failed the new mounted impulse-width regression. There is no CPU content replay, luminance analysis or adaptive black backing; masks and filters are cached. Bounds come from the selected scroll view’s live safe area, extended only by actual associated container bounds and clipped to the viewport; there are no fixed pocket extensions or assumed bar heights. Activation scales with that extent. The 213 edge checks cover none/hidden raster equivalence, automatic bottom suppression, restoration of explicit bottom styles, live geometry, rotation, resizing, compact/regular bars, zero occlusion, all four container edges, removal, private effects, sparse transparency, nested live glass, kernel-width profiles at 1x/2x/3x, zero radius, viewport boundary colors and mounted title-band legibility in both themes at 1x/2x/3x. Hard retains its bounded blur/separator; NXKit retains shared hard navigation/toolbar material. macOS and signed iOS Demo builds and the Switch NRO build pass; 7,310 strict Metal frames use the freshly baked archive without runtime Metal library/pipeline compilation. See `docs/scroll-edge-effects/none` for the current bottom-default/none policy, Demo option and verification, `docs/scroll-edge-effects/eased-gradient` for current easing/comparison/build verification, `docs/scroll-edge-effects/blur-onset` for the earlier title-band correction and `docs/scroll-edge-effects/variable-kernel` for the kernel implementation and earlier performance measurements. The earlier 12pt/9pt masked-uniform blur performance reports remain historical. Arbitrary chrome contours, RTL mapping, accessibility adaptations, standalone custom-container installation, exact iOS26 optical kernels and physical/Switch GPU visual/thermal parity remain incomplete. |
| `UIInteraction` and scroll-edge element containers | Partial | Core supplies `UIView::interactions`, `setInteractions`, `addInteraction`, `removeInteraction` and the weak-view protocol. Mutations serialize reentrant callbacks; tests cover ownership, transfer, ordering, duplicates and callback changes. UIKit supplies `UIScrollEdgeElementContainerInteraction` with weak `scrollView` and `edge`, initially none, to associate custom edge chrome inside UIKit navigation/tab screens. The associated container's actual bounds expand the scroll view's current safe-area pocket; descendant label/image sizes cannot shrink that geometry. Side safe insets alone do not invent horizontal effects. Multiple independent chrome groups on one edge and native descendant-shaped contours remain outstanding. |
| Private variable backdrop blur extension | Partial | Explicit `private/UIBlurEffect.h` exposes the iOS 26-observed `UIBlurEffect::effectWithVariableBlurRadius(radius, imageMask)` factory through `BackdropEffectView`. Image alpha controls spatial blur radius; zero preserves content, full alpha blurs, and a ramp progressively varies the blur. Nil masks and invalid radii reject before installation. A Gaussian variance pyramid avoids large-radius tap aliasing, with a provisional radius-to-sigma fit to the native radius-24 probe. This is a private API translation, not Apple's public UIBlurEffect style surface. Exact native kernels, mask adaptation and accessibility behavior remain unverified. |
| `UIScrollView` indicators | Partial | UIKit-shaped `UIScrollViewIndicatorStyle`, `showsVerticalScrollIndicator`, `showsHorizontalScrollIndicator`, axis-specific indicator insets, the deprecated combined-inset convenience, `flashScrollIndicators()`, and a `std::function`-based `withScrollIndicatorsShownForContentOffsetChanges()` are implemented with UIKit defaults; the block convenience reveals only axes whose offset changed. Private overlay layers stay out of `subviews()` and therefore out of content-size inference, track both axes and explicit content insets, reserve their shared corner, compress during overscroll, support default/dark/light styling, remain fixed to the moving viewport, appear for touch/controller scrolling, and fade after tracking, deceleration, bounce, or a flash. Indicator tracks use the greater of each physical safe-area edge and its explicit axis-specific inset, keeping both thumbs wholly inside visible bounds without discarding larger caller-supplied insets. Focused regression coverage verifies defaults, geometry, normalized movement, per-axis visibility/insets, asymmetric safe-area containment, styling, flash state, block-driven axis selection, overscroll compression, non-scrollable axes, and public validation. Add layout-direction-aware placement, exact platform-metric/visual verification, automatic controller-driven indicator-inset adjustment, and animation-completion-aware block fading before treating the broader current UIKit surface as complete. The C++ enum uses `defaultStyle` because `default` is a language keyword. |
| `UIScrollView` delegate | Partial | `scrollViewWillBeginDragging` and `scrollViewDidEndDragging(willDecelerate)` are delivered once per drag, alongside `scrollViewDidScroll`. Reentrant callbacks cannot complete or overwrite a replacement physics animation. Routed-touch tests cover multi-contact lifetime, stationary inertia catch and callback replacement. Add `willEndDragging` target-offset adjustment, begin/end deceleration and scrolling-animation completion callbacks; exact native deferred delivery timing remains incomplete. |
| Editable text: `UITextInput`, `UITextField`, `UITextView` editing | Missing | A display-only `UITextView` now exists, but forms, search, login, and settings still require a portable text-input client, SDL IME bridge, selection/caret model, clipboard integration, composition/marked text, secure entry, and keyboard navigation. |
| Core accessibility element semantics | Missing | Add accessible flag, label, value, hint, traits/role, enabled/selected state, frame, hidden state, grouping, ordering, actions, focus, and a platform bridge. Accessibility is part of the core definition of done, not optional polish. See Apple's [Accessibility for UIKit](https://developer.apple.com/documentation/uikit/accessibility-for-uikit). |
| Automated core tests | Partial | The opt-in NXKit tests cover Borealis-timed keyboard repeat generation, suppression of native SDL repeats, duplicate physical-down suppression, repeat flags, repeated directional focus, thumbstick reversal/neutral release, controller-disconnect cancellation, action opt-in/fallthrough, all 26 normalized SDL gamepad buttons plus both analog triggers routing to distinct responder actions, repeatable control highlighting, direct scroll-view and scroll-root controller focus resolution to a visible child, natural scroll-view temporary self-focus and constant-speed handoff through a press-consuming child, release after responder removal, centered edge clamping, minimal focused-mode visibility scrolling, and repeated focusable long-text directional scrolling; tab selection/delegation/configuration, compact-list navigation and regular-width restoration during animated push/Back, preserved selection/controller/view state and nested pushes, standalone full-screen/overlay modal fallback, remaining-width layout for intrinsically wide tab content, responder-routed A/B focus transfer with child B interception, unavailable-action fallthrough versus disabled interception, nested-navigation-to-tab fallback, window-owned top-level Exit, and touch-selected tab focus restoration when controller input resumes; touch-to-controller focus loss/regain animation callbacks, focused-control touch execution without visible focus reactivation, trait propagation into a focus-triggered action-legend rebuild, Yoga-enabled view reparenting, live label intrinsic-width invalidation, scroll-pan cancellation of a highlighted control and its pending action, non-bouncing release deceleration on a scrollable axis, scroll-disabled gesture gating with preserved programmatic offsets, scroll-indicator defaults/geometry/movement/insets/styles/visibility/flash/overscroll compression and validation, `UITextView` defaults, multiline storage, and horizontal-scroll/wrapping mode, safe self-removal during touch action dispatch, plus navigation stack mutation, containment, visible push/pop lifecycle, 120 fps transition scheduling, serialized overlapping programmatic push/pop requests, destination focus transfer with action gating during presentation, delegates, validation, nested `show` routing, overlay safe areas, movable status/action widgets, responder-derived and presentation-layer-isolated action legends, controller-icon resolution for every action and explicit generic/Xbox/PlayStation/Nintendo family, a Switch-only direct-provider harness that rejects silent fallback for every mapped NintendoExt glyph, invalid icon-size rejection, touch execution and non-focusability of non-A legend commands, latest-handler replacement, nearest-action precedence, SDL execution of custom and container Back commands, default modal Dismiss with focused and empty-focus presentations, immediate animated-presentation focus ownership, transition-time B routing with and without `allowUserInteraction`, presented-navigation Back-before-Dismiss priority, non-animated custom modal presentation/dismissal relationships, frames, lifecycle hooks, contexts, completions, navigation visibility, nested presentation-stack dismissal resolution, top-presentation focus trapping for directional and explicit requests, immediate built-in presentation-to-dismissal reversal with presentation-layer continuity and balanced lifecycle, dismissal-to-presentation serialization, and built-in full-screen versus over-full-screen hierarchy/appearance behavior. Expand into a fast core suite covering ownership, geometry, view/layer hierarchy, conversion, layout, lifecycle, controls, gestures, scrolling, traits, and animation; port applicable behavioral tests from UIKit-cross-platform. |

## P1 — common production application structure

| UIKit API or capability | Status | NXKit evidence and remaining work |
| --- | --- | --- |
| `UINavigationController` | Partial | UIKit now has its own concrete container over Core’s navigation capability: mutable stack, push/pop/pop-to/root, top/visible controllers, weak will/did-show delegate, child `show` routing, automatic Back and semantic menu/Back responder command, lazy visible views, eager containment, serialized/reentrant operations, selected-child appearance, focus/action gating, and live animated bar visibility with independent safe-area reservations. Content animates in container-owned hosts, preserving application transforms. A shared near-critical clock drives the native half-second content/title slide, 30% rear parallax, title wiping and rear-content dimming; the full Demo cycle is compared against fresh native containers. A public UIKit simulator oracle verifies lifecycle/delegate ordering, visible/top stack state, root-pop semantics, inline bar frame and safe areas; headless tests cover mutation validation, queued pop return values, metadata, focus/actions, 1×/2×/3× traits, compact height, independent ownership and nested Demo tabs. The full-screen iOS Demo now uses four real UIKit navigation stacks inside UITabBarController; its existing list is each stack’s root content. The enabled readonly `interactivePopGestureRecognizer()` drives a reversible leading-edge pop: content/title parallax and clipping follow translation, source glass previews half the destination extent with outgoing ink/tint retained, commitment hands off the expanded carrier to the spring/SDF/blur morph, and cancellation restores the original items and stack. Committed interactive pops now retain both existing title carriers and their reveal mask on the content timeline through release, preserving the release velocity/settling curve instead of restarting a standard title slide. Distinct custom title views travel inside those carriers and restore their ordinary hierarchy without changing application transforms. A 120-case regression now samples 17,400 frames over paused/flick/reversal outcomes, light/dark, 1×/2×/3× and ordinary/custom titles: 7,200 release frames plus 10,200 frames across natural completion and the material retirement tail. The outgoing title retires with interactive content completion, preventing it from reappearing at its model position when the gesture clock is discarded before the separate glass cleanup. The post-completion check fails 4,104 times before the cleanup fix and passes afterward; both navigation suites pass. See `docs/navigation/pop-title-settlement` and `docs/navigation/pop-title-completion`. Root/veto/direction checks, velocity-aware release, touch ownership, disabling/reset, second contact, resize, custom-view ownership, queued operations and cancellation callback timing have focused regressions. Thirty-two light/dark public UIKit recordings cover the eight arrangements; two further timed recordings verify callback/delegate ordering with the restored stack. Full-content/RTL pop, interruption of an already running automatic transition, transition-controller customization, search, toolbar and broad appearance APIs remain missing. See `docs/navigation/interactive`. NXNavigationController remains an independent console component. See `docs/navigation/README.md`. |
| `UINavigationBar`, `UINavigationItem`, bar-button items | Partial | Core owns style-neutral UINavigationItem/UIBarButtonItem data, including sharesBackground and identifier; UIKit mounts each control inside its actual shared/separate glass group (76 native configurations and 20 transition endpoints; see grouping audit); UIViewController.navigationItem() is lazy/read-only and follows the controller title until explicitly overridden. UIKit’s inline bar supports validated standalone item stacks and push/pop delegates, title/custom title view, left/right arrays and single items, live titles/images/enabled state/width/tint/custom views, plain/prominent buttons, common system items, hidden shared backgrounds, supplemental/hidden automatic Back, template/native symbols with portable text fallbacks, and animated retained leading/trailing shared glass capsules. Right arrays are rightmost-first. Native UIKit comparison establishes a 54-point portrait reservation with 44-point controls, standalone last-item pop and veto return semantics; glyph/group geometry is compared separately from private optical fidelity. Container-owned bars guard item/delegate mutations. Custom Back metadata, full spacer/overflow/RTL policy, menus/search, UIBarAppearance and scroll-edge material behavior remain incomplete. The complete eight-arrangement native playground now covers all 288 push/pop/light/dark layout pairs and root boundaries, plus dense rendered-frame captures of the 32 normal cycle transitions. Retained Back, supplemental item delay/absorption, root-specific phasing, title/content travel and wiping, dense trailing title alignment, glass carrier selection after removal, two-item unwrapping and retained/reparented ink replacement are compared separately. Focused routed-input, SDF/gap, blur, timing, reentrant lifecycle and cleanup regressions accompany native/portable contact sheets and movies. See `docs/navigation/cycle`; the smaller `docs/navigation/motion` audit is historical calibration evidence. Residual shape/phase/tint/blur differences remain; exact native pixel parity is not claimed. |
| `UITabBarController`, `UITabBar` / `UITabBarView`, tab items | Partial | Core supplies `UIBarItem`/`UITabBarItem` metadata (title, image, tag, enabled, selected image, badge value/color), lazy `UIViewController::tabBarItem()`, and `UITabBarControllerMode::{automatic,tabBar,tabSidebar}`. UIKit owns the extracted item-driven `UITabBarView`, with `UITabBar` alias, standalone nullable selection, weak delegate, live items/images/badges/tints, touch/pan/focus input and retained glass/lens physics. Its concrete `UITabBarController` manages eager containment, lazy selected content, mutable/reordered controllers, preserved identity/state, silent programmatic selection, user veto/notification, balanced selected-child appearance, focus transfer and container-reserved safe areas independent of application additional insets. Callback mutations queue or revalidate; invalid lists/hierarchy/items reject before mutation. `automatic` uses `tabBar` in UIKit and existing `tabSidebar` in NXKit on every OS. Each rejects its unimplemented explicit mode. NX sidebar now consumes shared item titles/enabled state live, retaining grouped and compact list/navigation behavior (see `docs/ADAPTIVE_TABS.md`). `UIKitTabTests`, module-composition/Core metadata tests and retained tab/adaptive/lens regressions verify this subset; the Apple tabs Demo nests UIKit tabs inside NX containment. UIKit sidebar, More/customization, modern UITab groups, appearance APIs, accessibility, reduced-motion and native GPU/physical-device parity remain outstanding. |
| `UIScreen`-like display information | Missing | Expose logical bounds, pixel scale, refresh rate/range, and display identity through a portable facade rather than requiring application code to access `SkiaCtx`. |
| `IndexPath` | Partial | A two-component section/item value now has const access and equality and is exercised by tab-controller tests. Add ordering/hash, arbitrary components if needed, negative-component policy, and focused value tests before using it as the stable public list index type. |
| `UITableView`, cells, data source and delegate | Missing | High-value foundation for settings, lists, menus, and feeds. Start with vertical reusable cells, headers/footers, selection, scrolling, insert/delete/reload, stable reuse, focus, accessibility, and keyboard/gamepad navigation. |
| `UICollectionView`, cells and basic layouts | Missing | Implement after table/reuse primitives. Begin with a flow/grid layout, reusable cells/supplementaries, selection, batch updates, and accessibility. |
| Reuse and diffable data model | Missing | Define stable identifiers and snapshot/diff behavior shared by table and collection views. A simpler C++ API is acceptable if lifecycle and update semantics are documented. |
| `UIStackView` | Alternative | Yoga covers most stack-layout needs, but there is no UIKit-compatible arranged-subview API. Add only if UIKit familiarity materially improves common layout code. |
| `UITextView` display/editing subset | Partial | `UITextView` subclasses `UIScrollView` and provides UTF-8 multiline text, viewport-constrained wrapping by default, opt-in unwrapped horizontal scrolling through `horizontalScrollEnabled`, explicit measured content sizing, dynamic color, numeric size/weight, paragraph alignment, UIKit's default `(8, 0, 8, 0)` text-container inset, measurement, safe-area-adjusted scrolling, XML construction, and a long-text Demo. SkParagraph shapes and lays out the document once per width/style change; only visible 1024-point retained drawing tiles plus one-tile overdraw are mounted, offscreen tiles are released, and each tile paints its clipped paragraph slice directly into the platform canvas without an intermediate bitmap upload. Font resolution, normal tracking and Apple grayscale coverage now share the corrected label/text-layer path; retained same-font coverage matches the label path. See `docs/text-rendering`. Add attributed runs, incremental/viewport text layout, selection, caret/editing/IME, links/data detection, scroll-to-range behavior, accessibility, and visual/performance regression coverage. |
| `UISwitch` | Partial | Optional UIKit control built on Core UIControl and shared LensView: on/setOn(animated), nullable on/thumb tints, retained modern-provider on/off images, preferred/concrete styles, value events, tap/drag/cancel, disabled interruption, console/portable accessibility activation and XML registration. Shared responder admission now delays pickup only for competing scroll gestures, yields early swipes to the scroll view, preserves ownership after admission, rejects already-ended delayed pans, and retains native held-vertical commit behavior without setting UIControl tracking/highlight flags. Native captures establish 63×28 actual / 61×28 fitting size, 37×24 resting / 58×38⅓ lifted knob, no resting shadow, programmatic movement without lift/actions, tap valueChanged-before-touchUpInside and drag valueChanged-only delivery. Short taps now complete the shared lens minimum animated lift before geometry lowers; value/action delivery remains immediate. Thirty-eight additional 60/120Hz timing, cancellation and independent-clock cases accompany four real-touch native switch and eight direct lens captures (see `docs/switch/tap-hang`). Native lift/position/lowering regression data, reentrant actions and cancellation/reparenting tests are retained under `docs/switch`. Exact track-well optics, arbitrary drag velocity/reversal, RTL and OS accessibility remain partial. Catalyst-only title/checkbox UI is excluded; requested checkbox resolves sliding as observed on iOS. Fullscreen Demo now exposes System UI elements → Slider/Switch with live settings/events; Glass appearance retains its slider and switch preview. See `docs/controls/UISwitch.md`. |
| `UISlider` | Partial | UIKit component now provides silent finite range/value setters, animated values, continuous/final events, per-state thumb/track images, tints, endpoint images, public geometry overrides, default/thumbless styles, copied track configurations, XML registration, keyboard/controller and portable accessibility adjustment. Thumb contact feedback begins independently of delayed UIControl tracking; thumb movement captures either axis while track swipes yield to scroll, preserving the original down point through replay. LiquidLens knob uses native profiles, measured 37×24pt resting / 1.55× contact geometry, pickup deadband, release projection and delayed terminal event ordering. Native geometry/touch replay and Raster/Metal comparisons are retained in `docs/slider`; Glass appearance Demo uses it instead of percentage buttons. Press/release geometry and resting-fill curves now follow recovered native small-lens springs with independent frame-curve regression data; short taps retain minimum lift, moving release overlaps lowering with deceleration, and stationary tracking ends independently from the visual tail. Slider optics respect native `warpsContentBelow=false` without suppressing its independent Gaussian transition; direct CAFilter inputs drive its optical spring and completion window; the visual host no longer cancels flex each frame. Filled-track endpoints use the native 2% cap taper, with 608 native measurements and explicit thumbless behavior. Public neutral configurations preserve minimum-origin fill. Exact interrupted-contact timing, full tick visuals, native configured getter/visual snapping, legacy track-image provider geometry, platform accessibility/haptics and physical-device parity remain unverified. |
| `UISegmentedControl` | Missing | Implement items, selection, momentary mode if needed, control events, intrinsic sizing, focus, and accessibility. |
| `UIActivityIndicatorView` and `UIProgressView` | Missing | Provide determinate/indeterminate progress primitives with reduced-motion-aware animation and accessibility values. |
| `UIAlertController` / actions | Missing | Implement portable alert and action-sheet semantics, focus trapping, keyboard/gamepad selection, cancellation, safe-area sizing, and accessibility. Avoid platform-specific iOS presentation chrome. |
| `UISearchBar` or search-field composition | Missing | Implement after `UITextField`; a composed control is sufficient if behavior, clear/cancel actions, focus, and accessibility are familiar. |
| `UIVisualEffectView` / blur and backdrop effects | Partial | Full captured iOS 27 glass-stack translation uses 560 sized profiles and 224 verified native accent matrices; custom sRGB colors use the native HSL and achromatic CPU equations. Standalone and merged paths share captured coefficients and independently translated AIR pass equations. Groups larger than three shade weighted optical batches against one original backdrop, preserving the common union while fitting the 16-sampler limit. Current native comparisons and platform/archive checks are recorded in `docs/liquid-glass-native-settings/spatial-corrections.md`; older texture/native-iPad figures in `full-translation.md` are historical. Apple material/interaction implementations now live in UIKit. Core exposes neutral `BackdropEffect`/`CABackdropLayer` for custom shaders, including validated `outputBlurRadius()` applied after the runtime shader independently of input blur and retains generic UIBlurView/CABlurLayer blur/tint; explicit Apple shader resolution is `UIGlassEffect::backdropEffect()` (rendering extension), replacing `BackdropEffect::glass()`. The Apple UIVisualEffectView hierarchy itself remains absent.  `UIBlurView`/`CABlurLayer` provide Skia blur/tint. Regular/clear copied `UIGlassEffect` also supports the NXKit `contentVibrancyEnabled` extension, false by default: only children are isolated and color-blended against the already painted material. XML, traits, copied effects and presentation layers preserve the option. Twelve public native system-tint probes per theme over six backgrounds calibrate bounded color-dependent additive/subtractive matrices, with green excluded from fitting. This approximates native foreground vibrancy; it does not claim UIKit’s private equation or automatic semantic-color inference. Literal RGB content remains source-over by default. See `docs/liquid-glass-colors`. Regular/clear copied `UIGlassEffect` supports opt-in `isInteractive` (false by default), passive descendant touch observation, native-trace-calibrated size-dependent spring inflation, centered independent-axis stretch and touch light without changing app layout/hit geometry. On the measured iPadOS 27.0 runtime the soft/stiff longest-side boundary is 119/120 points: small growth is 16 points, large growth resolves from 16 to 4 using the shortest side; press/release damping and edge-crossing pull resistance are calibrated separately. Regular neutral/tinted press lighting now has measured light/dark response. A 53-case actual UIKit touch/presentation fixture replays 8,857 retained frames with worst per-case aligned RMS geometry error 0.08276 points; acquisition delays/omissions and sparse samples are explicit. This is a curve/geometry result, not end-to-end latency or physical iOS 27.2 validation. See `docs/liquid-glass-interaction`. Removal/replacement/reparenting and hidden/disabled ancestors reset feedback; XML validates flags. MIT-licensed `liquid_glass_widgets` lens geometry is adapted to live backdrops with native-calibrated inward refraction, full-bevel frost, directional regular rim lighting, inverse-clipped default shadows and single device-space AA sharing continuous/circular corner geometry. Size/optics resolve from presentation bounds; filters run at pixel resolution. Explicit clear tint now renders; separate clear/regular vibrancy and dark/neutral tone are calibrated with the shared gallery's original material fixtures. The expanded glass gallery adds fixed clear/tinted controls and separate/container-backed mock navigation button rows, with actual control actions, responsive point-sized touch targets, press/cancel/resize regressions and matched UIKit light/dark captures; the container row merges through press inflation without dragging. Earlier gallery raw-channel metrics are superseded: the native hierarchy captures embed Display P3 and must be converted to sRGB before comparison. The new 60-scene/300-element iPadOS 27.0 optical matrix covers both themes, five sizes, three tint choices and ten backgrounds with strictly aligned overlays, exact-match suppression and separately labeled 2/255 residuals. Regular glass now uses a C1 quintic shoulder, directional rim lobes, extended-RGB vibrancy, theme-selective detail and recalibrated body/tint response. The original production matrix improved body/rim/refraction-band MAE from 6.703/9.085/6.981 to 1.305/3.065/1.850; the current refinement improves those values to 1.163/2.944/1.671 on the 0–255 channel scale. Four independent 60/240-point color-wave controls improve from 6.623/14.882/10.105 to 5.588/11.857/7.801, rejecting an extra dark chroma filter that only helped the original wave. Six pastel controls retain 1.402/255 body MAE. The actual resolved frost bounds the added chroma/detail kernels; 30 zero-frost locality controls stay exact and 30 positive-frost controls distinguish surrounding colors. All 762 measured matrix checks pass; individual 80-point light-line tradeoffs remain explicit. Default regular shadows now use a shifted-distance Gaussian survival profile in both themes, while clear glass has no automatic exterior shadow. Native 84-scene/408-region exterior MAE improves from 1.14077 to 0.01445, with no worsened region; application shadow overrides remain authoritative. See `docs/liquid-glass-refraction` and `docs/liquid-glass-shadows`. Hosts adopt display scale on construction/reparenting/screen changes, update active presentations, and retain same-screen explicit scale overrides. Mixed-color artwork and exact private optics remain approximate; see `docs/liquid-glass-optics`. Version validation keeps 27.0 light/dark appearance stable through resize round trips; the earlier 26.4.1 theme flip is not the default target. `adaptsAppearanceToBackdrop` remains copied and false by default: an 8×8 asynchronous sample selects small regular material and ordinary descendant content traits using approximate thresholds/hysteresis; clear/large use inherited appearance. Main-queue transfers reject stale replacements/reparenting and request missed-frame samples; default-off incurs no readback. Scroll-edge integration also verifies sampling the current offscreen composition rather than an older root framebuffer. Retained appearance/material/motion/corner/gallery regressions pass on actual UIKitCore raster; iOS Demo and regression targets build/link. The gallery 26.4.1 neutral/opt-in setup still differs materially from native; controlled contrast evidence remains in `docs/liquid-glass-ios27`. Exact private tint/edge optics, mid-size and iOS 26 retained states, materialization/effect transitions, canonical `UIVisualEffectView`, native touch timing and end-to-end Metal/Vulkan rendering/readback/performance remain partial. The normal macOS archive is absent; Switch Vulkan smoke verification is recorded in `docs/switch-vulkan`. See `docs/liquid-glass-gallery`, `docs/LIQUID_GLASS.md`. Long neutral regular platters use a chroma-preserving pressed response instead of the short-button white overlay; ten native phone tab-bar holds verify both themes with independent coral/green validation. Other wide control configurations remain unverified. See `docs/liquid-glass-lens/tab-press`. |
| `UIGlassContainerEffect` / grouped glass | Partial | Copied `UIGlassContainerEffect` with validated zero-default spacing is hosted by the portable `GlassContainerView/contentView()` extension; XML and UIKit umbrella/CMake integration exist. The live layer traversal combines child presentation SDFs and facing normals into one antialiased surface, with resolved per-item material/frost/tint, touch transforms, content drawn above, independent hit testing, unified default elevation, stable z-order and traversal-scoped child-material suppression. Removing/hiding/reparenting, ordinary affine wrappers, bounds origins, nested-container isolation and model/presentation copies are supported. The 17-screen Demo gallery includes neutral/tinted morphing and a blue/coral two-view order test. Explicit regular tint takes priority over neutral material; equal-priority materials follow painter order through a smooth eight-point ownership transition. Press light is clipped to each active source silhouette and projected through group coordinates onto the owning material, including a differently tinted neighbor; the extruded bridge stays unlit. Each exact material is now shaded in its own transformed coordinates before colors blend; categorical tint/style modes and lens/touch coordinates are no longer averaged across joins. A numerical smooth-field gradient corrects physical rim/optical distance and AA at thin necks without moving the measured zero contour. One-item containers retain the ordinary renderer. Mixed regular/untinted/blue pressed joins in both orders, off-center lighting, transformed geometry, thin-join AA and single-item 1x/2x/3x equivalence have regressions. The earlier join report is superseded by 96 color-managed native checkpoints covering light/dark, both orders, neutral/blue and blue/coral, and persistent joins before touch and after release. The isolated ownership/clip correction improves seam MAE from 11.861 to 7.009/255. The current combined material/shadow rerender reduces it to 4.897/255; one held blue/coral checkpoint still regresses against that baseline, recorded explicitly. Shared lighting itself causes no checkpoint regressions in its isolated matrix. Mixed-size neutral regular members now share defaults resolved from the largest neutral member short side while retaining their own optical geometry and every explicit override; clear and explicit-tint materials retain independent size resolution. Eight native white/gray group controls improve body MAE from 11.605 to 0.466; light-white near-ceiling residuals slightly worsen. Hide/resize/tint/clear/override regressions pass, and all 96 two-small-item rasters remain byte-identical after this additional CPU correction. The group also shares the maximum regular shadow elevation regardless of aggregate bounds or separation; see `docs/liquid-glass-shadows/shared-material` and `docs/liquid-glass-interaction/retained-joins/current-combined`. Integrated results and remaining residuals are in `docs/liquid-glass-interaction/retained-joins`. Retained raster tests cover 1x/2x/3x, animated geometry, touch inflation, content/input, two/three items, mixed tint, opacity, non-stacking overlaps, invalid/empty geometry, XML, clipping and membership changes. Independent native UIKit fixtures compare 108 retained movement/resize/light/dark/return cases per version: iPadOS 26.4.1 matches all 108 connectivity cases with minimum silhouette IoU 98.38%; 27.0 matches 107/108 with minimum IoU 96.91% and one explicit one-point AA transition difference. The default test tolerates that boundary; strict mode reports it. iOS Demo/tests build/link; actual core raster tests pass. Alpha-masked items and clipped/masked or 3D/perspective wrapper branches retain independent rendering. Dedicated 14-state two-tint comparisons on each of iPadOS 26.4.1/27.0 agree on tint ownership in both orders/themes, with mean body error 4.900/5.306; actual view reordering also updates hit ownership. Internal join shading, exact merge kernel, materialization/effect-transition animation, transformed-item optics and end-to-end Metal/Dawn performance/parity remain. See `docs/liquid-glass-container` and `docs/liquid-glass-gallery`. |
| `LensView` (portable extension, private UIKit lens research) | Partial | UIKit owns this Apple-styled component; Core remains independently usable. Explicit small/large (regular/selection aliases) styles, custom resting-background constructors, shared resting shadow/border configuration, source-preservation mode, native-shaped alongside/completion entry, caller-owned resting fill and content, weak external selected-source ownership, optional source vibrancy/magnification, animated lift, traits/scale/XML, presentation geometry and opt-in contact/view-motion interaction are implemented. The native-settings follow-up replaces the former empirical optical field with captured iOS27.0/24A434 small/large profiles:144 fresh settings cases cover both appearances, six sizes, rest/lift and tint preferences0/.5/1. All69 explicit glass-background inputs, missing-key provenance, independent backdrop/wrapper/source displacement, seven-sample foreground, capture margin, linear edge fade and final highlight are retained. Live portal identities and19 verified single-pass ablations establish routing; native foreground optics are independent of the system glass tint preference. Native UV normalization precedes its axis swap; an explicitly inferred capture-context policy is checked against independently varied margins and2×/3×/wide-lens references. It does not claim recovered native texture allocator dimensions. Size-dependent values follow model resize and live presentation bounds. Bounded compatible intermediates preserve native sampling boundaries and keep source RGBA/coverage; unsupported recording/oversized surfaces retain the analytic fallback. Zero-blur backdrop samples no longer bypass displacement through an unwarped LOD0 atlas, and omit that unnecessary allocation. Native profile, source lifetime/opacity, magnification, resizing, center identity, per-channel dispersion and ordinary backdrop placement regressions pass;48 lifted Metal/raster cases cover both styles/themes,1×/2×/3×, literal/vibrant ink and partial/full lift. The prior480-case bounded resting-source checks remain. Fresh lenses default to native mode0/preserveSource and warpsContentBelow=false; UITabBarView explicitly opts into mode1/hideSource and warping. Material retirement now applies equally to automatic and control-driven clients, using native lifted reference distances. Animated automatic lowering now preserves the captured 0.22s minimum lift with deferred alongside callbacks; 26 shared timing/interruption cases cover both styles at 60/120Hz. Externally clocked Slider motion keeps its established independent phase. TabBar layout follows the actual shared lens transition, with paired geometry/lens-clock regressions. The native `warpsContentBelow` policy is available as a portable extension; a dedicated displacement-then-Gaussian pass precedes caller-owned resting fill and ClearGlass optics. Direct slider CAFilter measurements establish independent optical timing, `max(0,6*(1-q))` blur, a cold first-lift `50*(1-q)` warp and zero settled/release warp. Generic small-lens endpoints (50/6 → 9/0) are retained; later-contact and switch interpolation remain unverified. Actual tab selection uses its captured resting Gaussian radius 2 / scale .25 and native color matrix, replacing pixel-fitted coefficients. Native spring specs drive slider bounds, with independent opacity timing. Exact interrupted-contact timing, automatic style thresholds, full rim/optical pixel parity, physical-device/HDR and every backend remain unverified. Current evidence and remaining differences: `docs/liquid-glass-lens/native-settings`; prior reports under `docs/liquid-glass-lens/{refraction,chromatic,motion,tab-content,tab-press,screen}` remain historical calibration evidence, not claims that the superseded empirical shader is still active. |
| `CAGradientLayer` | Missing | Add linear gradients first using Skia, including colors, locations, start/end points, scale, animation, and mask compatibility. |
| `CAShapeLayer`, `CGPath`, `UIBezierPath` core | Missing | Needed for icons, progress rings, clipping, custom controls, and vector drawing. Start with move/line/curve/close, fill/stroke, line styles, bounds, hit testing, and animation-safe ownership. |
| Layer masking | Partial | `CALayer::mask` uses destination-in alpha compositing, honors mask geometry and presentation state, preserves the backdrop required by blur layers, and invalidates with its masked hierarchy. Its content-isolation and destination-in offscreen passes are now bounded to the masked layer's local rectangle instead of implicitly allocating against the full canvas; a recording-canvas regression test verifies both bounds and backdrop initialization. Add focused pixel tests for partial alpha, transforms, animated masks, nested masks, nonzero bounds origins, shadows or descendants extending outside masked bounds, and transparent backbuffers; evaluate luminance masks only as an explicit extension because UIKit's contract is alpha-based. |
| Focus engine | Partial | Directional focus, focus environments, coordination, gamepad input, and explicit environment-targeted focus requests exist. Focus-update validation now visits the previous and proposed items followed by each unique ancestor focus environment, allowing containers such as `UIScrollView` to accept or defer descendant transitions without bypassing leaf vetoes. Explicit view requests use the view's focus search behavior: ordinary focusable views resolve to themselves, while focusable containers such as `UIScrollView` may resolve to a preferred descendant; a private exact-item path supports temporary container focus without changing public environment semantics. The top modal presentation defines the active focus branch as soon as installation begins, so traversal, programmatic requests, control actions, and presses cannot remain in or move into a retained presenting hierarchy; an empty presentation clears focus until it is dismissed. Touch input hides the focus appearance without preventing the same tap from reaching the focused control; focus requests made by touch-driven actions retarget the retained item without reactivating controller-style focus. Switching back to controller input models reactivation as `none -> retained item`, so the control repeats its gain-focus animation instead of treating the update as a failed same-item move. Replace hierarchy-order-only movement with geometric candidate scoring, disabled filtering, focus guides or overrides, and accessibility integration. |
| Keyboard navigation and shortcuts | Partial | Low-level key/press types and standard select/menu mapping exist. NX responder actions map Return/Escape/X/Y/equal/hyphen and the corresponding gamepad inputs to A/B/X/Y/plus/minus commands; Start/plus is no longer intercepted as an application quit shortcut. Focused-responder routing, unavailable fallthrough, disabled interception, navigation Back, tab focus fallback, modal Dismiss, and application-window B/menu Exit have regression coverage. Tab navigation, richer shortcuts, localization-aware confirm/cancel policy, and broader standard-control behavior remain incomplete. |
| Layout direction and localization support | Missing | Add left-to-right/right-to-left traits, leading/trailing layout concepts, natural alignment correctness, string/resource localization hooks, and mirrored control behavior. |
| Dynamic type / scalable metrics | Missing | Add preferred content-size categories or a platform-neutral scale trait, font metrics, relayout, and accessibility-size support. |
| Clipboard / `UIPasteboard`-like service | Missing | Provide portable text and image copy/paste through SDL/platform backends, required by editable text and common application workflows. |
| Application resources / bundles | Partial | `NXData`, image resource loading, and libromfs exist. Define a stable bundle/resource abstraction, errors, locale variants, scale variants, and testable lookup rules. |
| `UINib` / declarative view loading | Alternative | Core registers only its views; optional UIKit views require explicit idempotent `registerUIKitViews()`. Shared XML converters no longer import UIButton or its style enum.  A custom TinyXML2 view registry and XML inflater exist. It is not compatible with Apple nibs, storyboards, outlets, actions, or archives. Keep the dialect portable and document its schema; do not imply Interface Builder compatibility. |
| Notifications / observation utility | Missing | A small safe notification/observer primitive is useful for UIKit-like lifecycle and app state. Prefer typed C++ callbacks/tokens and deterministic removal over copying Objective-C selector APIs. |

## P2 — common advanced functionality

| UIKit API or capability | Status | NXKit evidence and remaining work |
| --- | --- | --- |
| Pinch, long-press, swipe and rotation gestures | Missing | Implement shared recognizer timing/state infrastructure first, then individual recognizers with touch-count and cancellation tests. |
| Scroll-view zooming | Missing | Add zoom scale/range, content centering, delegate hooks, gesture arbitration, and accessibility after basic scrolling is dependable. |
| `UIPickerView` and a portable date-picker model | Missing | Prioritize general picker mechanics; keep platform-specific calendar appearance separate from the data/control contract. |
| `UIPageControl` | Missing | Add page count/current page, value events, focus/touch input, intrinsic sizing, and accessibility. |
| `UIToolbar` and general bar items | Missing | Reuse navigation item/action and appearance primitives; avoid iOS-only placement assumptions. |
| `UISplitViewController` | Missing | Implement adaptive two/three-column containment only after navigation and trait systems are stable. |
| `UIPageViewController` | Missing | Implement after container lifecycle and scroll/gesture arbitration are tested. |
| Menus and contextual actions | Missing | Provide portable action models, nested menus, enabled/selected state, mouse/touch/gamepad presentation, keyboard shortcuts, and accessibility. |
| Pointer, hover and mouse interaction | Missing | SDL mouse input exists at a low level. Add hover events, cursor/interaction state, control highlighting, and consistent hit testing. |
| Drag and drop inside the application | Missing | Start with local typed payloads and reordering. Native inter-application drag and drop can remain platform optional. |
| Multiple windows | Missing | Support multiple `UIWindow` roots and event routing without requiring Apple's `UIScene` API. |
| Custom modal transitions and presentations | Missing | Add transition contexts, cancellation, presentation containers, dimming, and interactive progress after standard modal behavior is stable. |
| Interruptible view animation / property animator | Missing | Current animations cover common implicit/basic/spring cases. Add pause, continue, reverse, fraction-complete, interruption, and completion-position semantics. |
| Advanced attributed text and text attachments | Missing | Build on `UIFont`, path/drawing, and editable text. Include paragraph styles, runs, links, attachments, selection, and accessibility. |
| Image animation and resizable images | Missing | Add only after image lifetime, caching, and invalidation are well tested. |
| State restoration | Missing | Use portable identifiers and serializable controller/view state; exclude Apple's process- and scene-specific mechanics. |

## P3 — later portable breadth

| UIKit API or capability | Status | Notes |
| --- | --- | --- |
| Advanced layer types (`CAReplicatorLayer`, tiled rendering, emitter-like effects) | Missing | Add based on demonstrated application demand and Skia feasibility. |
| Appearance proxies and broad theming | Missing | Prefer explicit theme/trait propagation first; add appearance-style defaults only with deterministic scope and invalidation. |
| Content configuration APIs | Missing | Useful once reusable list cells and control state configuration exist. |
| Rich text layout manager compatibility | Missing | Consider a smaller portable API unless source familiarity justifies UIKit/TextKit-shaped types. |
| Native inter-application drag/drop and platform services | Missing | Platform capability, not a requirement for the portable core. |

## Resize and rotation audit — 2026-11-02

The renderer exposes a guarded render-only path for native live-resize callbacks. macOS redraws within SDL's main-thread expose watch even while AppKit holds the outer event pump, and native layer geometry changes disable implicit actions. Apple SDL frame pacing waits for events rather than blocking through a complete idle interval. Resize rendering never polls events, runs dispatch/display-link callbacks or sleeps recursively.

iOS keeps SDL's root controller and native Metal-view frame/rotation ownership. A transparent child controller receives `viewWillTransitionToSize:withTransitionCoordinator:` and animates an internal bounds probe with the coordinator; the renderer samples its presentation bounds once per frame for logical size, safe-area interpolation and native-resolution drawable allocation. Display scale no longer comes from dividing endpoint SDL pixels by intermediate point bounds. Active transitions request the display refresh rate, and generation checks protect interrupted/completed transitions. Ordinary nonanimated changes are not eased. Simulator Metal/reference-curve, Cocoa expose/wakeup, startup scale/reentrancy and glass regression evidence is retained in `docs/resize-rotation`; physical GPU/rotation feel remains unverified.

## Nested glass stress audit — 2026-11-02

The Demo adds a live 96-card glass screen with one vertical feed and eight horizontal rails, available from Glass or direct launch with `NXKIT_GLASS_STRESS=1`. The clean-tree framebuffer skip is disabled globally; normal frame pacing remains, and the stress controller requests the display refresh rate while visible (updated 2026-11-04). A 64-frame SDL replay verifies vertical dragging through a rail, drags in every horizontal rail, card/toolbar actions, simultaneous scroll offsets and presentation of every frame. Scroll pans reject travel along a non-scrollable axis before preventing an orthogonal ancestor; general pan behavior is unchanged. Catching an already-decelerating nested rail before axis selection remains an arbitration limitation.

The measured hot paths are full-tree preparation of clipped content and glass shading. Clipped subtree rejection avoids invisible glass/filter preparation, already ordered layers avoid stable-sort scratch work, and shared distance/normal shader evaluation plus inactive-press/sample fast paths reduce optical arithmetic without reducing backdrop resolution. The view layout walk still visits the full tree. Focused regression and pixel comparison evidence, CPU timing methodology and platform build limitations are recorded in `docs/liquid-glass-stress`. CPU raster and null-canvas results are not physical-device GPU/thermal measurements. Portable glass and scroll coverage stays Partial.


### Native Metal follow-up — 2026-11-04

`NXKIT_GLASS_STRESS_REPLAY=1` launches the same 96-card Demo and repeatedly scrolls
its feed and all rails. `NXKIT_UNCAPPED=1` removes the shared software frame wait;
normal runs keep their pacing and clean frames still composite. Native drawable
acquisition can still pace a displayed app. A new offscreen Metal benchmark has
neither presentation throttling nor a frame sleep, and separates layout/display,
recording, submission and synchronous completion wait.

Three alternating-order repetitions at each of dark 3× and light 2× cover 5,760
frames with zero runtime library/pipeline compiles. Reducing redundant corner
traversal, clean drawing-environment copies, and repeated weak-parent copies
reduces warm scrolling preparation by 76.7% and combined CPU time by 22.4%/22.0%.
Synchronized total medians improve 7.0%/5.7%; scrolling completion waits are largely
unchanged, and glass GPU work remains substantial. Traversal counts match and
four initial/scrolled raster images are byte-identical. This is Mac evidence,
not a measured reduction in iPhone temperature or energy. The initial installed
phone CPU profile is retained separately; device before/after work was abandoned
at the user's request after connectivity was lost. Reproduction, validation,
and remaining hot paths are in `docs/performance-stress-20261104`.

### Sustained glass follow-up — 2026-11-04

A connected iPhone capture reports Serious thermal state and substantial Metal
debug-validation stacks. A separate validation-disabled stress replay identifies
glass drawing and repeated material resolution as hot paths; these different
workloads are not a device A/B comparison. Resolved material descriptions are
cached across steady frames, including grouped-size defaults. Immutable
descriptions are shared by model/presentation layers and grouped drawing;
internal uniform reads borrow values instead of allocating temporary vectors.
Sampling bounds, optical uniforms and blur quality are preserved. Material
snapshot isolation and Demo/stress raster regressions cover the retained path.
The Xcode Demo generates an optimized, symbolized **ThorVGApp Performance**
scheme with Metal validation/capture disabled and normal pacing. Rebuilding
preserves user launch arguments and benchmark environment overrides. Following
the renewed user request, normal use skips unchanged frames before acquiring a
drawable; benchmark executables explicitly retain continuous rendering and
uncapped runs remain continuous. Physical sustained energy/thermal improvement
is not established. Strict device replay exposed four device-only MSL variants;
complete diagnostic source blocks are recovered with provenance and freshly
baked into the final 125-library/102-pipeline bundle. The Mac strict 7,310-frame
coverage matrix has zero runtime library/pipeline compiles. Alternating paired
Mac runs show 1.8–5.1% less CPU time in stress scrolling, no consistent GPU
completion gain, and byte-identical output in all twelve raster comparisons.
A native Mac idle comparison removes all sampled rendering/submission after
startup and lowers main-thread sampled CPU weight 65.2% in one identical-window
pair (unoptimized Demo, active stress display link). Signed iPhone archive-only
stress replay reports no shader misses; sustained post-change thermal evidence
remains unavailable after Instruments attachment/save failures.
Measurements, fresh offline shader provenance and remaining work are recorded
in `docs/performance-glass-20261104`.

## Glass tint preference — 2026-11-04

`UITraitCollection::glassTintAmount()` exposes the requested portable glass
preference, from clear (0) to tinted (1). The regular fallback is 0.5. `UIApplication` provides a validated optional override;
clearing it follows the latest platform value. Window snapshots deliver changes
before layout through controllers, views, masks, and inherited glass appearance.
Existing glass hosts rebuild their cached material when only this trait changes.

Apple 27+ reads UIKit's dynamically resolved nullable NSNumber getter or macOS's
native cached `NSGlassTintAmount` preference. Both bridges are read-only, guarded,
and fall back safely when unavailable/invalid. These are private system interfaces,
not public UIKit trait APIs. Other platforms use the regular midpoint unless
an app opts into another value. Independent iOS 27.1 framebuffer captures now
calibrate the preference/style/color response: regular glass retains a neutral
body and explicit accents at 0%, clear style retains its color transfer while
frost changes, and 100% still transmits backdrop color. The 480 native color
controls pass (mean channel error 1.80/255, maximum 14/255), alongside 80 textured
controls (mean amplitude error 1.53/255, maximum 9.01/255). The former interpolation
fails these controls. Icon opacity, geometry and explicit frost overrides retain
their meaning. Rims and detailed blur remain approximate; native setting-change
notification delivery and the full preference response of every tint hue/alpha
remain unverified.
Caller-authored backdrop shaders and LensView's caller-owned resting fill retain
their existing meanings.

The Glass gallery includes app settings with system following and five tint
positions, persisted by Demo through its own SDL preferences path. Range/nonfinite
validation, snapshots, app precedence/reset, same-bounds system updates, late child
inheritance, reparenting, masks, cached material invalidation, and legacy glass
appearance copies have focused regressions. Material tests cover both themes,
regular/clear and colored materials over changing backgrounds. Native bridge
observations, API examples, shader coverage, builds and remaining limits are in
`docs/glass-tint-preference`. The calibrated source ABI `4b541699…` passes a fresh
43,416-frame Mac archive-only replay across scale/theme edge profiles, all eight
navigation arrangements, interactive commit/cancel paths and fixed/continuous
glass preferences, with zero runtime Metal library/pipeline compilations. All
137 libraries and 110 pipeline descriptions are rebuilt into matching macOS and
signed iOS bundles. Both app builds and the Switch NRO pass. An archive-only
physical iPhone stress smoke run prepares 110 native pipelines and reports no
shader misses or runtime pipeline compilations; the normal app is restored.
The independent visual matrix is iOS 27.1 Simulator; a full physical-iPhone native
visual comparison, external system-change delivery and refreshed Switch GPU
verification remain outstanding.

## Glass performance audit — 2026-10-30

The full-screen glass review fixes clean-frame pacing (120 Hz interaction requests return to the existing 30 Hz idle target), avoids repeated invalidation of an unchanged elastic lens transform, and skips periodic NX navigation widget refresh while detached/hidden. Resting vibrant lens content blends its picture directly, avoiding the lifted source-surface/backdrop-filter path; lifted refraction retains device-aligned capture. Containers conservatively partition distant output regions while retaining a common union/material and sampling/elevation reach. Scale/pacing, hidden/revealed widgets, settled-lens, filter-region and glass regression tests accompany both-theme raster pixel comparisons. A physical iPhone 15 Pro Max/iOS 27.2 idle CPU profile identified tree traversal/hidden action polling, with no sampled rendering during that interval. Busy-host CPU raster times improved; these are not GPU timings or evidence of a measured temperature reduction. Physical post-change power/GPU/thermal and Switch/Dawn verification remain outstanding. See `docs/liquid-glass-performance`.

## Tab lens transfer audit (2026-11-02)

The Demo tab composition now retains its enlarged clear lens through short-tap travel, then lowers near arrival instead of lowering immediately on touch-up. Six real UIKit phone cases establish first/fourth, reverse, adjacent, reselect, dark and held-press behavior. A measured position curve and minimum-lift/arrival policy produce 0.12–0.36pt center RMS error after temporal alignment, with lowering within one 120Hz frame; these are trajectory comparisons, not input-latency measurements. Routed-touch regressions cover catching a travelling lens without double elastic drift, rapid retarget, cancellation, exact resting geometry and pulse disposal. Five lens/screen suites and the iOS Debug Demo pass. Optical/elastic parity remains incomplete (including excessive horizontal stretch on long jumps); physical-device, reduced-motion and GPU parity are unverified. That measured composition now supplies the shared gesture/material implementation of public `UITabBarView` in UIKit; Demo retains its comparison artwork/geometry. Generic item layout and complete native parity remain Partial. See `docs/liquid-glass-lens/tab-transfer` for raw native/portable evidence, screenshots, reproduction and limitations.

## Navigation bar/container audit — 2026-11-03

Core navigation metadata and UIKit navigation presentation now have separate ownership. The Full-screen iOS layout Demo uses UITabBarController → four UINavigationControllers → retained list root controllers, with navigation actions supplied by UIBarButtonItems. Its list controls, collection behavior, bouncing and per-tab state remain in Demo. The former screen-owned bar/lens chrome is removed from that composition; independent optical fixtures remain in Demo.

The production tab layout now restores native floating geometry: a 21pt bottom gap with a 34pt device inset, an 83pt total child bottom safe area, and 4pt selection-lens edge insets. `UITabBarView::sizeThatFits` reports natural widths for two/three items (188/274pt); four/five use available phone width. Selection width follows each actual cell, including content-weighted narrow layouts, with cached title metrics and shared hit/ink/lens centers. A new public UIKit oracle and explicit-width probe verify all selections for 2/3/4/5 tabs in both themes at 320–430pt: 140 geometry comparisons pass within 0.23pt. Navigation-item materials now receive passive routed touches from their sibling controls; plain/shared and prominent press/drag/release/cancel behavior and disabled/action gating are tested. Five focused suites pass; both Apple Demo builds compile/link. See `docs/navigation/layout-fixes` for captures, source hashes and reproduction. Optical parity, Dynamic Type and full overflow/tablet policy remain Partial.

A native-only public UIKit probe captures stack and lifecycle/delegate state, read-only UIView/CALayer presentation geometry and screenshots on iOS 27.0. Portable regressions compare the same operations and separately check retained glass carriers, presentation continuity, reentrant operations, content transforms, hide/show, disabled actions, traits and nested kit composition. Null traits during reparenting are now safe in UILabel; incoming transition hosts attach before loading their content. Exact motion/optics and physical-device GPU behavior remain incomplete. Sources, captures, tolerances and reproduction are in `docs/navigation` and `Demo/NavigationComparison`.

All 10 native stack/frame/safe-area cases and the 46-event delegate/lifecycle sequence match. Endpoint-normalized horizontal capsule width RMS error is 1.13pt for push and 0.46pt for pop-to-root after onset alignment; center RMS error stays below 0.09pt. Native evidence is light-mode only; portable regression/renders cover both themes. The earlier full raster/Core-only runs passed 32/13 tests. Fresh Core/navigation/tab checks pass 3/3 with rebuilt matching Skia, and the native comparison remains green with refreshed portable evidence. The iOS Xcode Debug Demo (unsigned) and macOS Metal Demo now compile/link; the Switch Demo packages successfully. Physical-device/GPU execution is not established by these builds.

### Navigation grouping follow-up

The full-screen iOS Demo now exposes a Navigation morphing row in every tab's
collection. It pushes eight actual UINavigationController destinations: shared
icons, a prominent action, a zero-width fixed-space split, three shared images,
text actions, leading items supplementing Back, an explicitly unshared item and
a hidden-background item. Each destination offers Push next, direct arrangement
links and Pop to collection, while bar actions update an on-screen tap counter.
Stable item identifiers exercise matching across image/text/style changes.
The Demo regression follows all eight pushes and reverse Back operations,
arbitrary jumps, pop-to-root, tab stack retention and collection state retention.
This adds manual animation coverage without expanding the library API.

Navigation grouping follow-up (2026-11-03, working tree after `c87cd14`):
UIKit now mounts each navigation control inside its actual glass capsule,
replacing the sibling-background press workaround. Adjacent images/custom
views share; text, prominent, Back/Done, fixed/flexible separators and explicit
`sharesBackground=false` split groups. Hidden backgrounds retain independent
content without glass. Core adds usable `UIBarButtonItem::sharesBackground`
and optional string `identifier`; identifiers preserve controls across metadata
replacement/reordering. Native phone separators use a measured 12-point group
gap irrespective of tested fixed-space widths. Group and ink geometry share a
spring clock, split births/pulses carry their content, and interrupted joins
retarget visible geometry. CALayer completion now uses elapsed duration/delay
instead of the eased value, allowing pulse return/cleanup. Native evidence:
76 grouping states, 20 push/pop endpoints and four public XCUITest contacts;
all group decisions/endpoints pass, with at most 1-point scored frame error.
Custom UIButton intrinsic sizing is reported separately. Raster contact affine,
in-flight containment, cancellation, disabled state, identifier reorder and
interruption regressions supplement Core timing cleanup checks. The API remains
Partial: exact transient optics/physics, item groups, RTL, Dynamic Type, overflow
and general tablet layout remain incomplete. See `docs/navigation/grouping`.

### Navigation motion polish

The 2026-11-03 frame-by-frame follow-up (working tree after `4cd6300`) replaces
independent scale/fade capsules with overlapping SDF separation and absorption.
A retained source capsule now stays continuous until its neck pinches apart;
redundant joining material moves into the receiver and is removed before its
undershoot. Capsule bounds, item layout, birth growth and a separate surface
pulse use fitted springs. A private UIKit foreground layer supplies a Gaussian
blur reveal/removal without filtering the glass or sibling content. Native
matched identifiers update symbols immediately; unmatched/reparented items blur.
Retirement has a separate clock, interrupted tails are removed, and routed
button input remains available after the content transition settles.

A warmed public UIKit iOS 27.0 fixture captures 20 light/dark push/pop clips;
the cold-start 329 ms capture stall is preserved separately. The 18 comparable
baseline/current clips supply 720 geometry samples each with no phase fitting:
rectangle RMS error falls from 10.30 to 1.33 pt. Over 360 dark raster samples,
mean silhouette IoU rises from 81.4% to 96.5%; connectivity agreement on the six
join/split cases rises from 68.8% to 98.3%. The report includes per-case errors,
matched/unmatched blur frames and a compositor join video. Exact optics,
asymmetric birth stretch, arbitrary leading arrangements and
broad platform/layout parity remain incomplete. Navigation and seven related
regression suites pass; macOS and unsigned iOS Demo builds compile/link.
See [`docs/navigation/motion`](docs/navigation/motion/README.md).

Navigation spacing correction (2026-11-03, working tree after `4cd6300`): the
native and portable resting layout gap is already 12 points. Reduce only the
navigation glass container's SDF blend reach from 20 to 12 points to stop the
facing-edge deformation and delayed separation. Raster regressions require a
neck at 1/4 second, a clear gap at 1/3 second, and independent round resting
contours within one pixel at 3×. See the motion report's spacing follow-up.

## Intentional divergences to preserve or decide explicitly

These are not silently “complete.” They must remain documented until a deliberate compatibility decision changes them.

| Area | Current divergence |
| --- | --- |
| Private liquid-lens configuration | `LensView` / `UIShadowProperties` are documented portable UIKit extensions. Generic resting fill stays caller-owned/clear. Animated lowering preserves the captured 0.22s minimum lift and deferred alongside callbacks, including same-state pending-unlift behavior. The nonanimated two-argument convenience provides a portable force/snap path; normal grouped callbacks use the observed two-boolean pair, whose native flag names remain unknown. Advanced portal-container relocation and alternate punchout are not exposed as stubs. Private punchoutShadow/opacityPair compositor behavior remains approximate. |
| Finite lens elasticity | Extra deformation remains active while lifted and fades to identity within two seconds after lowering starts, regardless of continuing movement. Ending contact while still lifted does not begin retirement; contact-only use retires on endInteraction. Re-lifting cancels retirement continuously. This is separate from lifted geometry and optical state, and is not claimed as UIKit's private activation policy. |
| ScrollPocket disabled style | `UIScrollEdgeEffectStyle::none` is a portable extension; the iOS 27.1 UIKit header still declares only automatic/soft/hard (native callers can hide an edge). Kit resolution keeps Core neutral: UIKit automatic bottom is none, other attached edges soft; NXKit automatic remains hard. |
| ScrollPocket appearance | At the user’s request, UIKit soft pockets use a fixed 0.4-to-1 content fade and a fixed spatial profile selecting a local Gaussian kernel radius up to 4pt. The gradient changes kernel width, not the opacity of a uniformly blurred copy. The kernel progression is separate from content attenuation and grows earlier inside the pocket so text behind bar titles is blurred; the maximum stays 4pt. Cached quintic attenuation and monotone cubic radius profiles smooth the transition inside the same live bounds. Content coverage/luminance does not vary this profile. This simplifies iOS26 adaptive legibility; bounds follow the scroll view’s live safe area and associated edge-container geometry. |
| Container capabilities and modules | `navigationController()`/`tabBarController()` return Core interfaces rather than Apple concrete classes so custom and Nintendo containers participate. Discovery follows actual parent containment (including compact NX reparenting), not logical membership in a tab model. `selectedIndex()` uses `std::optional<size_t>` and setters return success. The portable `UITabBarView` name has a `UITabBar` alias. Automatic tab presentation is kit-specific (UIKit bottom bar, NX sidebar/compact flow), rather than Apple platform policy. Concrete Apple APIs belong to UIKit; existing non-Apple effect hosts remain explicitly documented rendering extensions. |
| Language/API shape | C++ setters/getters, callbacks, smart pointers, and value types replace Objective-C/Swift properties, selectors, ARC, and protocols. Until the menu model lands, UIKit's `UIMenuLeaf::repeatBehavior` policy is exposed directly on `UIAction`. |
| Control selectors and slider configuration | Control selectors are strings with weak responder targets; action handlers receive a value copy carrying a weak sender. The legacy optional primaryAction remains an adapter. Leading/trailing alignment is LTR until a layout-direction trait exists. Slider defaultStyle avoids C++’s reserved default keyword; track configurations are copied values. Keyboard/accessibility adjustment uses a portable 1% step. Private flex/timing, full tick visuals and OS accessibility/haptics remain incomplete; see `docs/controls/README.md`. |
| Geometry names | NXKit currently uses `NXPoint`, `NXSize`, `NXRect`, `NXAffineTransform`, and `NXTransform3D` rather than Core Graphics names. |
| Layout | Yoga flex layout replaces Auto Layout constraints and anchors. |
| Interface files | NXKit's TinyXML2 format is a custom declarative layout dialect, not an Apple nib/storyboard parser. |
| Drag presentation | `DragSmoother` and `UIScrollView::dragSmoothingEnabled` are portable extensions. Switch defaults to visual following with a small adaptive delay; other platforms opt in. Pan recognition and velocity stay independent of presentation. |
| Rendering | Normal use renders on retained-tree changes or platform exposes; view-only display/layout invalidations schedule a tree walk even before a layer changes. Clean frames skip both traversal and GPU acquisition. Continuous benchmark rendering is opt-in through `NXKIT_CONTINUOUS_RENDERING=1`, and implied by `NXKIT_UNCAPPED=1`; environment polling and animation delivery remain paced while idle. iOS uses native run-loop waiting and stops SDL's empty high-refresh display link at the idle target. Views are custom-rendered through Skia rather than native platform widgets. Native platform look, accessibility, text input, and system integration must therefore be implemented explicitly. |
| Concentric corner bridge | C++ uses immutable values, explicit validation, and Objective-C selector stems where Swift argument-label overloads collide. Detached concentric views immediately use their minimum rather than retaining UIKit's stale attached cache. As explicitly requested, iOS below 26 queries the private screen-radius selector; iOS 26+ uses only public APIs. Other platforms supply square native window geometry by default. |
| Layer corner curves | Continuous is the requested default instead of UIKit's circular default. The portable rounded-superellipse and circular-arc reconstruction approximates the private native curve; see the native-mask follow-up in `docs/liquid-glass-corners/continuous`. `cornerCurve` switches discretely; animated `cornerRadius` and bounds use that curve. |
| Size classes | iOS uses native UIKit traits from the SDL window. Platforms without UIKit classify logical window width at 600 points and height at 480 points; these are explicit NXKit cutoffs, not Apple guarantees. Classes are inherited by subviews rather than inferred from individual view bounds. Modern trait dependency tracking and subtree overrides remain unimplemented. |
| Glass preference | `glassTintAmount` is deliberately a portable trait and application override, rather than UIKit’s private global setting. Apple reads are guarded and read-only; missing values retain 0.5/current appearance. Applications own storage of their overrides. |
| Backdrop effects | `UIGlassContainerEffect` uses `GlassContainerView/contentView()` rather than Apple's `UIVisualEffectView` host; spacing starts at zero. Unsupported masked/3D wrapper branches render independently. `UIGlassEffect` mirrors regular/clear styles and opt-in `isInteractive`, hosted by NXKit's `BackdropEffectView` rather than canonical `UIVisualEffectView`. Analytic native-calibrated lens/rim equations replace the lightweight fallback. Regular glass adds a separate directional half-point exterior contour and a tight pale-material reflection core; the earlier inner-rim score omitted that contour. Standalone and merged-container paths share the measured light/dark law, including stronger dark density as size increases, darken only backdrop coverage and preserve internal joins. Native2x exterior probes,1x/2x/3x point-width, overrides, hide/reparent and presentation-position tests are recorded in `docs/liquid-glass-rim`; the independent elevation shadow remains separate. Full tint, corner and GPU pixel parity remain approximate. NXKit retains regular-size adaptation; regular uses directional neutral light while clear retains a configurable exterior-color reflection contribution. Its default-false `adaptsAppearanceToBackdrop` extension restores iOS 26-style contrast selection for small regular glass and ordinary content, using approximate private thresholds; the host keeps inherited traits. Feedback changes rendering geometry only; public layout and hit-test geometry stay app-owned. The portable host exposes `interactionTransform()` for inverse contact mapping into its shared presented deformation; tab grab-offset tests cover inflation. Custom effects use SkSL with one backdrop child, optional unfiltered `sharpContent`, automatic geometry/touch/filter-scale uniforms, and an explicit sampling radius. |
| Scroll-view focus tracking | `UIScrollViewFocusTrackingMode` is an NXKit extension inspired by Borealis controller navigation. UIKit does not expose the same natural/centered/focused three-mode API. |
| Partial-curl transition | Skia currently has no page-curl primitive in NXKit. `UIModalTransitionStyle::partialCurl` preserves the full-screen restriction with a safe 2D affine fold of the presented view; it does not reorder or curl the live presenting hierarchy and is not visually identical to UIKit's page curl. |
| Extension components | `NXNavigationController`, `NXNavigationItem`, `NXTabBarController`, `NXResponderAction`, `NXControllerIconResolver`, and `NXControl` are opinionated console components and do not count as UIKit API coverage. `NXNavigationController` borrows UIKit's stack operations, top/visible concepts, serialized transition requests, delegate timing, appearance/containment model, per-screen item configuration, and `show` handling while retaining Switch-style blurred header/footer chrome, movable status/action widgets, and a responder-derived console legend. `NXResponderAction` models commands for all normalized SDL gamepad buttons and both triggers, including enabled state, dynamic availability, semantic identifier, priority, and `UIAction`; nearest-responder precedence differs from UIKit command discovery. `NXControllerIconResolver` follows the last active controller family and deliberately uses native Apple SF Symbols, Nintendo's Switch font glyphs, or portable vector fallback artwork instead of prescribing one cross-platform asset set. The Demo Controller tab provides a manual regression screen for every normalized button icon, both stick axes, analog trigger pressure, icon aging, and the hold-to-exit interaction. `NXTabBarController` keeps grouped sidebar, focus-following selection, tab-item A-to-content behavior, conditionally available B-to-selected-tab fallback, selected-item focus restoration across touch/controller input changes, and a regular-width content column constrained beside its configurable sidebar, plus a compact full-width list that opens actual tab controllers through `show()` and restores selection/state on expansion, while borrowing UIKit's mutable controller list, selected controller/index, delegate veto/notification, and containment ordering. |

## Currently out of scope

Do not add these to the active completeness tables unless the project scope changes:

- `UIScene`, scene sessions, iOS process restoration, and iOS-specific application-shell lifecycle
- status-bar, home-indicator, device-orientation, and iPhone/iPad idiom policy APIs
- Apple Pencil, 3D Touch, and device-specific sensor interactions
- UIKit wrappers for camera, photo library, contacts, mail, messages, telephony, StoreKit, or authentication services
- printing, AirPrint, document browser/picker, and iOS share-sheet controllers
- CarPlay, watchOS, visionOS-only, or extension-host APIs
- WebKit, MapKit, AVKit, SpriteKit, and other separate Apple frameworks
- haptic APIs until a portable feedback abstraction becomes a project goal

Safe areas, keyboard/gamepad focus, mouse input, multiple windows, accessibility, localization, and clipboard remain in scope because they are broadly useful across supported and future platforms.

## Near-term milestone order

1. Replace unsafe shared ownership and add the initial automated test target.
2. Make view/layer invalidation, masking, hierarchy operations, and lifecycle ordering dependable.
3. Finish remaining `UIControl` platform integration, `UIButton`, `UILabel`/`UIFont`, editable text/IME, and `UIScrollView` core behavior.
4. Add accessibility semantics and a first platform bridge.
5. Implement standard navigation/tab containers and reusable table/list infrastructure.
6. Add gradients, paths/shapes, common controls, alerts, and localization/dynamic type.
7. Add Android, Windows, or Linux backends only with shared conformance tests so platform expansion does not fork behavior.

## Scroll behavior audit — 2026-10-31

Public UIKit `UIScrollView`/`UIPanGestureRecognizer` were exercised with actual OS touches on an iPhone 17 Pro simulator running iOS 27.0. Thirty-nine retained scenarios cover contact changes, slow/reversed releases, inertia catches, bounce re-grabs, viewport changes and rotation. Native input timestamps and delivery timestamps are separate; all raw captures, source hashes, before/after replay traces and limitations are retained in `docs/scroll-physics`. The native fixture uses private XCTest event synthesis only; glass/lens production rendering has no private UIKit calls (the separately documented pre-iOS-26 window-corner bridge uses the requested private screen-radius selector). A long FIFO multi-touch relay exposes a simulator pointer-retirement artifact and is explicitly excluded from fidelity claims; the portable routed-touch harness verifies a stable 40-contact relay instead.

Analytical motion solves edge collision and free-flight stop times independently per axis. Timers retain their current invocation during cancellation, and generation checks protect replacement animations from stale callbacks. Completing retained layer animations snapshots registry membership so completion callbacks may remove animated layers safely. Six focused regression targets and the iOS Demo build verify this patch; physical iOS/Switch feel and cancellation need device validation.

### Complete navigation playground motion follow-up

The native UIKit fixture now mirrors all eight Demo arrangements, direct links,
Push next, Pop to collection, four tab stacks and bar tap feedback. Its root
body is intentionally a small table fixture; the portable comparison uses the
actual Demo controllers. A fresh-container matrix covers all 288 layout pairs
in both directions and themes, including root boundaries and equal-layout
controllers. A dense 32-clip pass captures every available native rendered
frame of the normal push/Back cycle and every 120-Hz portable frame. Actual
request-relative timestamps are retained without per-case onset alignment.

This audit fixes supplemental Back/star phasing, root-specific leading timing,
shared title/content travel and wiping, native half-second completion, rear
parallax/dimming, three-item title alignment, carrier identity after removing
the visual-leading item, two-item unwrapping and carrier-specific ink replacement. It also
corrects the earlier raster assumption that every split should retain a neck
at 250 ms: native two-item unwrapping has a center gap there, while removing
an item from a three-item group still forms a joined SDF surface. Input, model
transforms, lifecycle serialization and nonanimated interruption remain covered
by focused regressions. Current results, limitations, every-pair evidence,
build/source hashes and reproduction are in `docs/navigation/cycle`.

The completed 288-clip comparison reduces changing-leading Back rectangle RMS
from 2.81 to 0.66pt and opacity-weighted star rectangle RMS from 26.15 to 3.05pt.
Content horizontal RMS across the matrix improves from 45.10 to 5.55pt; right
rectangle RMS where material counts agree improves from 4.13 to 2.41pt.
Mean per-clip right count agreement is 98.0%, with hidden-background transitions
excluded from that material score but retained in the visual evidence. The
dense cycle records 1,090 native and 2,912 portable rendered frames;
Back/star RMS is 0.47/2.22pt. These are timestamped geometry comparisons,
not claims of pixel, input-latency or physical-device parity. All eight focused
suites and macOS/unsigned iOS Demo builds pass.

### Navigation ink color follow-up — 2026-11-03

Ordinary navigation image/text items, supplemental leading items and automatic
Back now default to dynamic `UIColor::label`, matching the native cycle fixture
even when the bar or its ancestors have an explicit accent tint. Explicit
`UIBarButtonItem::tintColor` still overrides ordinary ink; clearing it restores
label ink. Prominent items retain white ink and use the explicit item tint or
inherited bar accent for their glass fill. Live tint/style changes and light/dark
resolution pass focused navigation regressions at 1×/2×/3×. Fresh production
Demo renders verify text, Back/supplemental images and prominent controls against
the retained native captures in `docs/navigation/cycle/label-ink`. Historical
motion captures remain unchanged. Navigation tests and both Apple Demo builds
pass; broader appearance customization and exact optical parity remain Partial.

### Leading navigation material overlap follow-up — 2026-11-03

Leading glass now separates and absorbs through geometry while its material
stays opaque. Previously fading the foremost glass during overlap exposed the
backdrop inside Back, producing a conspicuous dark patch in dark mode. Incoming
ink reveals independently after separation begins; Back's blur settles by the
content completion. Collapsed private carriers use an invertible subpixel scale
to avoid the transform decomposition's zero-scale fallback. Dark raster
regressions compare overlap against Back alone with both glyphs hidden (at most
4/255 interior darkening), and cover delayed ink, geometric retirement, root
timing and the sharp Back endpoint. Native/before/after light and dark captures
of push/pop with automatic and custom-root Back are in
`docs/navigation/leading-material`. This fixes navigation's opacity path; it
does not establish general translucent-material overlap or full motion parity.
Navigation and Demo screen suites pass, and macOS/unsigned iOS Demo builds
compile and link successfully.

### Metal pipeline preparation and offline archives — 2026-11-03

The paused iPhone stack identified synchronous Metal pipeline compilation during
`Recorder::snap`, beyond the SkSL cache. Stable runtime-filter decal sampling
reduces animated graph permutations. A maintained Skia Metal hook lets Core load
build-time libraries/native archives and prepare generic pipeline descriptors
before interaction without depending on either component kit. Missing/invalidated
known binaries recover from minimum-OS IR at launch; custom/new states retain a
measured worker-assisted fallback and an atomic, asynchronous persistent cache.
ABI/GPU/OS keys prevent incompatible cache reuse. Strict validation forbids source
compilation and binary misses; rejected pipelines now fail without a Skia null
dereference. The initial Demo Apple catalog baked 68 shader sources and 55 pipeline descriptions.
The current source catalog also covers interactive navigation; its manifest and
`docs/navigation/interactive` record the current counts and coverage.
Fresh-process strict Metal runs cover 5,833 frames per profile, dark 3× and light
2×, with zero runtime library/pipeline compilations. Final packaged-archive runs
have pop CPU p95 of 2.18/2.52 ms and maxima of 6.42/16.18 ms. These are Mac CPU
submission measurements, not iPhone/GPU/FPS parity. Removing the native archive
recovers all 55 states before the first frame; the next strict process reuses the
persisted archive with zero compilations. Strict mode rejects an absent archive
without silently compiling. All 32 headless and six catalog/embedding tests pass;
macOS and unsigned iOS builds include verified shader bundle resources, and an
unchanged iOS rebuild skips the bake.
The standard catalog, current OS compiler-generation requirement, cache recovery,
custom-state limits and reproduction are documented in
`docs/navigation/pipeline-cache`. Physical-device execution of the rebuilt iOS
app, older GPU/OS catalogs and arbitrary custom shader coverage remain Partial.


### Interactive navigation pop — 2026-11-03

Core's navigation interface now exposes Apple's nullable readonly gesture
capability. UIKit installs one enabled leading-edge pan on its container view;
NXKit inherits the no-installed-gesture default. No UIKit dependency was added
to Core or NXKit. A private reversible layer clock keeps per-transaction
animations alive while tracking, without changing ordinary time-based animation
completion or application content transforms. Animations started while loading
application content remain outside the navigation clock. Cancelled touch ownership and
reentrant begin vetoes remain in Core; layout, preview geometry, material,
release springs and the stack transaction remain in UIKit.

The native oracle contains 32 actual edge swipes (eight adjacent arrangements,
light/dark, completed/cancelled) and two timestamped lifecycle/delegate traces; two repeated leading-item cancellations check one anomalous native completion.
It establishes retained source ink/tint during tracking, partial glass extent,
content/title wiping and parallax, continuation from the release pose, and
cancellation reconciliation after visual return with the original stack already
restored. UIKit's source did-appear then will/did-show cancellation order is
reproduced. Application gesture observation cannot replace the framework action.
Focused regressions also cover anonymous items, ordinary root boundaries,
custom-view ownership/transforms, second contacts and delayed taps, enabling,
self-resetting delegates, queued changes and forced cancellation on resize/bar
visibility changes. The 2,663-frame matrix has 0.525pt tracking carrier-coordinate RMS, 0.239pt held
RMS and a retained 6.60pt peak outlier; one native short swipe completed unexpectedly
and is annotated, while two fresh repetitions cancelled. Paired clips, full traces, reproduction and measured residuals
are in `docs/navigation/interactive`. Private optics, exact continuous spring/
input-latency parity, RTL/full-content gestures and automatic-transition
interruption remain Partial. The Apple shader catalog is regenerated from the
actual full-screen automatic/interactive fixture rather than relabelled; ordinary
source edits now trigger its ABI dependency check.

All 33 headless C++ suites and six shader catalog/embedding tests pass; the
separate UIKit-only interactive executable also passes without NXKit linkage.
macOS and unsigned iOS Debug Demo builds succeed. Fresh-process packaged-archive
checks cover 34,994 full-screen automatic/interactive Metal frames in dark 3× and
light 2×, with zero runtime library or pipeline compilations. Both bundles contain
68 libraries and 54 native pipeline descriptions and match their baked outputs.
Source/artifact hashes, raw counters and test/build summaries are retained in
`docs/navigation/interactive/verification.json`. Physical-device execution remains
unverified; the existing paused iPhone debugger session was left untouched.

### Stable automatic Back identity — 2026-11-03

The bar's persistent automatic Back item now has the explicit internal identifier
`UIKit.navigationBar.back`. Automatic/custom roles are isolated in ordinary and
interactive identifier matching, so an application item cannot take over Back's
control by reusing that string. A focused regression samples 144 push/pop
transitions across portrait/compact layouts, 1×/2×/3×, both themes and unchanged
supplemental leading configurations: Back retains its item/control/ink/glass,
with exact stationary geometry, identity transforms, unchanged opacity and no
ink blur. Twenty-four collision/action cases and interactive tracking/reversal/
commit/cancel checks also pass. Actual leading-item layout changes keep their
split/join animation. See `docs/navigation/stable-back` for current verification.
UIKit libraries and navigation tests compile/pass on macOS, and the iOS UIKit
library compiles. Full Demo rebuilds encountered concurrent
`UICornerConfiguration.h:9,13` explicit-constructor/copy-list initialization
errors; those unrelated edits were left untouched. The completed navigation
snapshot and offline catalogs are verified independently of that ongoing work.
Fresh offline archives for that navigation snapshot cover 34,994 strict
full-screen automatic/interactive Metal frames with zero runtime library or
pipeline compilations; current source/artifact hashes and the unrelated full
Demo build errors are retained in `docs/navigation/stable-back/verification.json`.

## Navigation-bar large titles (2026-11-04)

Demo inset correction (2026-11-05): the fullscreen grouped list and large-title
playground explicitly use `scrollableAxes` rather than `always`. They retain
vertical navigation/tab safe-area adjustment while their full-width content no
longer acquires a horizontal scroll range from landscape side insets. Existing
Demo rotation regressions cover both lists. Core currently defaults to
`scrollableAxes`; UIKit's distinct `automatic` inset policy remains unimplemented.

`UINavigationBar.prefersLargeTitles` (default false) and
`UINavigationItem.largeTitleDisplayMode` (Automatic/Always/Never; default Automatic)
are implemented. Automatic inherits the preceding item's **request**, even when
that screen has scrolled its large title closed. The newer Inline display mode,
Dynamic Type, RTL and search integration remain unimplemented; these APIs stay
Partial pending that broader coverage.

UIKit owns an internal accessory-row layout with measured extents, collapse
allocation and settling boundaries. A navigation controller coordinates those
rows with its screen's automatically discovered or explicitly selected content
scroll view. Visible safe-area reservation changes independently of the full
scrollable accessory range. Container observation is weak, separate from the
application's delegate and disabled for inline-only screens. Selection and
hierarchy changes rebind observation. Per-screen extents/offsets survive push,
pop, tab switches and cancelled edge pops; title rows share the existing
reversible title/content animation clock. Scroll pockets follow the resulting
live safe areas. No search-controller or palette API is exposed as a stub.

Pop sequencing regression: title-carrier bounds now include their own visible
accessory rows independently of the bar's changing safe-area reservation. A
button pop to an expanded title reveals that row during the slide; a subsequent
edge pop retains the outgoing row even when its carrier inherited clipping from
the preceding gesture. Routed-input tests cover 32 combinations of first-pop
method, second-pop commit/cancel, expanded/partially collapsed state, light/dark
appearance and 2×/3× scale, including presentation samples and held-gesture layout.

Core now delivers `scrollViewWillEndDragging(scrollView, velocity,
NXPoint* targetContentOffset)` with UIKit's points-per-millisecond velocity,
mutable destination, finite-value validation and reentrant-scroll protection.
Applications can override the container's proposed destination. Targeted
settling preserves unaffected axes and cannot turn an interior destination into
a second free flight. Existing free-flight/rubber-band behavior is unchanged
when no destination adjustment is made.

A public UIKit fixture on iPhone 17 Pro / iOS 27.0 measures a 52-point expanded
row above the existing 54-point portrait bar, point-for-point drag geometry and
asymmetric release relaxation (12.1/s expansion; 16.8/s collapse). Retained touch
traces and measured response errors are in `docs/navigation/large-titles`.
The Full-screen iOS layout Demo adds **Navigation → Large titles** with
Automatic, Always and Never push destinations. Existing collection/morphing
screens keep their inline appearance. Native programmatic offset/property
updates can defer expansion until another presentation; NXKit updates its
retained layout immediately. Full native animation/pixel parity and physical
large-title testing are not claimed.

Verification: 11 focused regression executables pass, including 213 scroll-edge
checks and all 27 Graphite Metal vertex-format round trips. Large-text rendering
exercised `UShort4Normalized`; the offline descriptor exporter/loader now handles
that format. macOS, iOS RelWithDebInfo and Switch NRO builds succeed. Fresh Apple
bundles contain 162 libraries/129 pipelines under the final shader ABI. A strict
Mac Graphite replay of 21,018 large-title/navigation/edge/ink-blur frames performs zero
runtime library or pipeline compilations. Configuration supplied while a screen
loads is applied before transition snapshots, and a mounting guard preserves
ScrollPocket overlay ordering. See the retained verification and fidelity limits
in `docs/navigation/large-titles`.
