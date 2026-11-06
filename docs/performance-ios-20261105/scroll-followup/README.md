# Scrolling follow-up — 2026-11-05–06

Working tree after `50a7240`. This follows the user's report that scrolling in
the installed optimized build still felt laggy. Implementation and validation
remain on iOS Simulator; the user's physical iPhone is not used for measurements.

## Changes

- Preserve a monotonic render deadline across early SDL event wakes. Input and
  dispatch callbacks still drain promptly, but waking a wait no longer permits
  another render before the requested frame interval. Fractional milliseconds
  are retained; idle rendering skips and the explicit uncapped benchmark mode
  remain available. A slow frame renders the current state once, without
  replaying a backlog of missed deadlines. Native live-resize rendering remains
  immediate.
- Keep the slow-release scroll follower's 120 Hz request after finger lift,
  instead of dropping its request to 60 Hz while its last measured position
  settles. Gesture smoothing constants and scroll physics are unchanged.
- Specialize pyramid shaders for their actual stencil size, removing a
  32-iteration uniform branch from the normal thirteen-tap glass reduction.
- Evaluate each blur reduction at its destination resolution, with tap offsets
  mapped to the preceding level. This avoids a full-size blurred intermediate
  before downsampling. Stencil weights, radii, capture scale, atlas layout,
  material recipes and optics are retained. Removing intermediate quantization
  can change pixels slightly; this is checked against independent native blur
  references and before/after scene images below.
- Skip the exterior shadow sample and its grouped shifted-distance calculation
  where full body coverage replaces it. Antialiased boundaries and exterior
  shading retain their complete path.

## Timings

Apple M1 Max, iOS 27.0 Simulator / iPhone 17 Pro Max, SDK 27.1,
arm64 RelWithDebInfo, Metal validation disabled. Same 402×874-point, 3×
offscreen workloads as the preceding report. Runs are sequential before/after,
then after/before, without concurrent builds or tests. Only the final warm pass
is included. Medians combine equal-sized runs; pipeline counts do not increase
in those passes. **These times include synchronized GPU completion; they are
not delivered FPS or measurements of physical iPhone performance.**

| Workload | Installed-renderer baseline | Follow-up | Reduction |
| --- | ---: | ---: | ---: |
| Eight navigation arrangements | 17.297 ms | 13.120 ms | 24.2% |
| Nested glass scrolling | 46.038 ms | 34.425 ms | 25.2% |

Each renderer contributes 1,008 navigation frames and 48 scrolling frames.
[Measurements and executable hashes](measurements.json), plus compressed CSVs
under `raw/`, preserve the runs. Host conditions differ from the preceding
report; its numbers should not be substituted for this matched baseline.

## Validation

Results are retained in `regressions/`. The new material shortcut compares the
old exterior/body equation over translucent colored content in both themes,
clear/regular styles, neutral/blue color, all three glass preferences and
1×/2×/3× scales: 72 cases on each of raster and Simulator Metal. Output is
byte-identical. The existing 20 fractional capture controls per backend also
remain byte-identical, and standalone output bounds pass at all three scales.

All 64 independent native blur controls pass unchanged thresholds. Step MAE
improves from 0.406494 to 0.363281/255; stripe MAE improves from 0.704753 to
0.562500/255. The destination-resolution reduction changes intermediate
rounding; exact full-scene pixel identity is not claimed. Twenty-four navigation
controls have maximum whole-image resting MAE 0.0175/255 and motion MAE
0.0433/255. Most resting cases have maximum channel error 2/255; one has a
localized larger residual. Motion controls also include relative-timer phase
variation. [Image differences](image-comparison.json) and the
[header comparison](header-comparison.png) retain this distinction.

Frame scheduling checks cover 1 kHz wake bursts, exact fractional 120 Hz
intervals, idle-to-touch rate changes, late frames, and explicit uncapped mode.
Scroll physics additionally exercises the real application loop after slow
release. The standalone iOS input harness now calls `SDL_SetMainReady` and
checks event initialization: it previously ignored a failed SDL startup, making
its queued-input check unusable under `simctl spawn`. Existing assertions and
thresholds are retained.

Twelve targeted suites and the additional uncapped pacing case pass. Navigation
and tab layout fixtures now request explicit synchronous render frames rather
than assuming five immediate run-loop calls advance five display deadlines.
Their layout, trait, lifecycle and animation assertions are unchanged. The
separate frame-pacing suite exercises the deadline gate itself.

## Reproduction

Use the Simulator configuration and build commands in the preceding
[optimization report](../optimization/README.md). Build with
`NXKIT_BUILD_METAL_SHADERS=OFF` while harvesting changed shader sources, then
rebuild the matching offline bundle before packaging. Run the navigation and
stress commands twice in opposite order, retaining pass 2 only. Run
`NXKitFramePacingTests` normally and with `SIMCTL_CHILD_NXKIT_UNCAPPED=1` plus
`--uncapped`; run `NXKitScrollPhysicsTests` and the retained material/navigation
suites using `simctl spawn`.

## Packaged build

Fresh capture covers 14,644 simulator frames in dark 3× and light 2× profiles:
all navigation arrangements, push/pop, interactive commit/cancel, large-title
scrolling/transitions and the stress scene's glass-setting sweep. Warm passes
compile no new pipelines. The raw-source catalog retains earlier unchanged
states with provenance; all binaries are freshly rebuilt for the current ABI.
The phone bundle contains 228 libraries and 171 pipeline descriptions; its GPU
archive is 124,633,600 bytes. The optimized app is built and signed with the
existing development identity, and all 238 sealed resources verify.
[Artifact hashes](iphone-build.json) identify the ready-to-install app at
`Demo/build/ios/RelWithDebInfo-iphoneos/ThorVGApp.app`. It has not been installed
on the iPhone during this follow-up.

The simulator bundle uses its freshly captured 99 libraries / 75 pipelines.
A cold-cache packaged check exercises another 1,513 navigation and interactive
gesture frames with zero source-library or interaction-time pipeline compiles.
Its 75 startup pipeline preparations are expected on Simulator; these are
prepared-object hits, not native GPU binary-archive hits. See
`bundled-shader-check.json`.
The actual packaged full-screen replay completes both scroll passes and
navigation, reaches idle with zero submitted idle frames, and restores normal
Demo launch. `packaged-replay.json` records this validation; its timing is
excluded because the phone build ran concurrently. The export harness now
waits, within a bound, for the completion phase rather than assuming app startup
has finished when `simctl launch` returns.

This establishes less rendering work and fixes event-wake scheduling. It does
not establish sustained 120 FPS or lower temperatures on the physical iPhone.
