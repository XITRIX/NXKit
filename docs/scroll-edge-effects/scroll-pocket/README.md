# Fixed ScrollPocket verification (initial 12pt setting)

The current edge path now uses a direct [variable kernel](../variable-kernel/README.md); this report retains the earlier masked-uniform approximation and its historical measurements.

The current default was subsequently relaxed to **9pt**. See the [tuning report](tuned/README.md) for current sources, builds and shader verification. This report retains the original 12pt optimization evidence and measurements.

The private implementation now uses one `UIScrollPocket` per edge. Public configuration remains on [Apple-shaped UIScrollEdgeEffect styles](https://developer.apple.com/documentation/uikit/uiscrolledgeeffect/style-swift.class). The retained iOS26 traces identify `_UIScrollPocket`, PocketMask and PocketBlur; this is a portable C++ translation, not a dependency on private Apple runtime calls.

At the user’s request, soft pockets use a fixed **0.4→1 content mask** and **12pt Gaussian blur** blended through a fixed edge gradient. Blur alone runs at one pixel per logical point; foreground and analytic masks retain full display resolution. There is no extra SkPicture recording/replay of scrolling content, CPU image/coverage/luminance analysis, adaptive black backing, or full-viewport mask-picture rasterization. Masks and filters are cached. Scroll penetration still animates activation; overscroll, visibility and per-screen ownership retain their meanings.

[Retained adaptive baseline sources](baseline-source/manifest.json). [Before/after light/dark dense/sparse images](images/before-after.png), [exact image hashes and unchanged geometry](images/visuals.json), [49-case native geometry comparison](comparison/README.md). All 76 active carrier bands have unchanged coordinates versus the prior portable renderer. Native iOS26 optical adaptation is intentionally not matched by this fixed policy.

Working tree after `09e631b`, audited 2026-11-04. Real Graphite/Metal on Apple M1 Max; shader ABI `b6d5a4a49e00a71c46af76b4c38084ec93c044a9a3ab0b757b8da9585c54c4a5`.

Each cold and strict sweep rendered **40,844 frames**: six 975-frame scroll-edge profiles and two 17,497-frame fullscreen Demo navigation profiles. Every strict profile completed with **zero runtime library compilations, zero pipeline compilations, and zero failures**.

The edge benchmark covers all four edges; automatic/soft/hard/mixed/hidden styles; discovered/obstructed/explicit bindings; side chrome interactions; fractional offsets and overscroll; app masks; sparse transparent content; an opt-in adaptive glass observer over a black sibling inside masked content; and private variable blur with zero/one/ramp masks at radii 0/1/8/24/48.

| Strict edge profile | CPU p50 | CPU p95 | Synchronized total p95 |
|---|---:|---:|---:|
| light 1× | 1.39 ms | 1.75 ms | 4.69 ms |
| dark 1× | 1.36 ms | 1.84 ms | 4.97 ms |
| light 2× | 1.45 ms | 2.01 ms | 5.66 ms |
| dark 2× | 1.51 ms | 2.07 ms | 5.98 ms |
| light 3× | 1.57 ms | 2.13 ms | 8.77 ms |
| dark 3× | 1.61 ms | 1.98 ms | 7.53 ms |

| Strict fullscreen navigation | CPU p95 | Pop CPU p95 | Maximum pop CPU |
|---|---:|---:|---:|
| dark 3× | 1.81 ms | 1.79 ms | 7.73 ms |
| light 2× | 1.70 ms | 1.73 ms | 10.96 ms |

Edge timing table uses the last repeated pass; navigation statistics include every captured frame. CPU submission time and synchronized GPU completion time are separate measures. Async observer completion polling occurs after the edge timing interval. All strict measurements were taken after other builds finished. These are local stall diagnostics, not device FPS or physical-iPhone performance measurements.

The raw catalog contains 98 content-addressed MSL sources and 88 complete render-pipeline descriptors. Earlier raw states are retained additively with explicit ABI/hash provenance. No old GPU binaries were relabelled; macOS and iOS libraries/archives were rebuilt using the installed SDK toolchain.

Normal macOS and unsigned iOS Demo builds passed. Both embedded shader directories match their freshly baked outputs byte for byte. The app configurations, runtime tools and catalog share the same ABI. Offline native archive targets use the installed macOS27/iOS27 compiler generation; portable libraries retain macOS12/iOS15 deployment targets. Actual iOS26-device native archive execution remains unverified. The visual target is iOS26 UIKit. No physical device was installed, resumed or run.

See [report.json](report.json) for source, catalog, archive, executable and bundle hashes; all profile counters and timings; and compressed raw CSV/log hashes. [commands.md](commands.md) records reproduction commands; [merge_catalog.py](merge_catalog.py) validates complete eight-profile live capture and preserves only exact raw source/descriptor states.

## Paired performance

| Fixture | CPU median before → after | Synchronized total median before → after |
|---|---:|---:|
| dark 3× | 3.10 → 1.78 ms (42.6% lower) | 6.58 → 4.61 ms (30.0% lower) |
| light 2× | 2.98 → 1.59 ms (46.6% lower) | 5.55 → 4.21 ms (24.2% lower) |

Three alternating-order pairs per profile; repeated passes 1/2 only, 84 vertical-scroll frames per version/profile. This 402×874pt fixture has **all four edge-chrome pockets**, and selects discovered automatic/soft rows; it differs from a typical top/bottom-only phone screen. Both binaries forbid runtime compilation and use their own verified offline bundles. These are whole-render CPU/submission/completion measurements on this Mac, not isolated shader timing, phone power or temperature. See [performance.json](performance.json) for source CSV hashes, selection rules and limits.


## Functional verification

All **38 regression suites**, **four Core-only suites**, and **50 scroll-edge checks** pass. Analytic-mask tests verify group alpha at 1×/2×/3× and bypass of picture fallback. The updated Switch ELF source target builds; no NRO regeneration or Switch GPU shader bake/device execution is claimed. [Retained test logs and artifact identities](verification.json).

