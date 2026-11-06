# Scroll-edge Metal verification

**Historical adaptive baseline.** The current implementation intentionally uses
fixed ScrollPocket masks and stronger blur; see the [current report](../scroll-pocket/README.md).

Working tree after `09e631b`, audited 2026-11-04. Real Graphite/Metal on Apple M1 Max; shader ABI `afcd86dd28eaac42d8649c64ec3e7f0d909381230f476de61e5ca985bfee1465`.

Each cold and strict sweep rendered **40,844 frames**: six 975-frame scroll-edge profiles and two 17,497-frame fullscreen Demo navigation profiles. Every strict profile completed with **zero runtime library compilations, zero pipeline compilations, and zero failures**.

The edge benchmark covers all four edges; automatic/soft/hard/mixed/hidden styles; discovered/obstructed/explicit bindings; side chrome interactions; fractional offsets and overscroll; app masks; sparse transparent content; an opt-in adaptive glass observer over a black sibling inside masked content; and private variable blur with zero/one/ramp masks at radii 0/1/8/24/48.

| Strict edge profile | CPU p50 | CPU p95 | Synchronized total p95 |
|---|---:|---:|---:|
| light 1× | 2.25 ms | 3.14 ms | 5.81 ms |
| dark 1× | 2.24 ms | 3.08 ms | 5.97 ms |
| light 2× | 2.28 ms | 3.34 ms | 6.35 ms |
| dark 2× | 2.47 ms | 3.96 ms | 7.02 ms |
| light 3× | 2.41 ms | 3.49 ms | 7.50 ms |
| dark 3× | 2.42 ms | 3.48 ms | 7.62 ms |

| Strict fullscreen navigation | CPU p95 | Pop CPU p95 | Maximum pop CPU |
|---|---:|---:|---:|
| dark 3× | 2.22 ms | 2.20 ms | 4.66 ms |
| light 2× | 2.06 ms | 1.95 ms | 13.32 ms |

Edge timing table uses the last repeated pass; navigation statistics include every captured frame. CPU submission time and synchronized GPU completion time are separate measures. Async observer completion polling occurs after the edge timing interval. All strict measurements were taken after other builds finished. These are local stall diagnostics, not device FPS or physical-iPhone performance measurements.

The raw catalog contains 88 content-addressed MSL sources and 81 complete render-pipeline descriptors. Earlier raw states are retained additively with explicit ABI/hash provenance. No old GPU binaries were relabelled; macOS and iOS libraries/archives were rebuilt using the installed SDK toolchain.

Normal macOS and unsigned iOS Demo builds passed. Both embedded shader directories match their freshly baked outputs byte for byte. The app configurations, runtime tools and catalog share the same ABI. Offline native archive targets use the installed macOS27/iOS27 compiler generation; portable libraries retain macOS12/iOS15 deployment targets. Actual iOS26-device native archive execution remains unverified. The visual target is iOS26 UIKit. No physical device was installed, resumed or run.

See [report.json](report.json) for source, catalog, archive, executable and bundle hashes; all profile counters and timings; and compressed raw CSV/log hashes. [commands.md](commands.md) records reproduction commands; [merge_catalog.py](merge_catalog.py) validates complete eight-profile live capture and preserves only exact raw source/descriptor states.
