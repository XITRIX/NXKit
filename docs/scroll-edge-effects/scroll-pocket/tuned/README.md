# Relaxed ScrollPocket blur

The current edge path now uses a direct [variable kernel](../../variable-kernel/README.md); this report retains the earlier masked-uniform approximation and its historical measurements.

This records the 9pt tuning revision. A later [navigation title settlement fix](../../../navigation/pop-title-settlement/README.md) changes the library/build fingerprint while retaining this edge-blur setting and rendering path.

The soft-edge Gaussian radius is **9pt**, reduced from 12pt. The fixed 0.4→1 content-opacity mask, edge extent, activation, one-pixel-per-point blur evaluation and cached rendering path are unchanged. Hard pockets are unchanged.

The [light/dark dense/sparse comparison](images/before-after.png) shows both portable settings. All 76 active-band coordinates match exactly across 49 states. This tunes the requested fixed appearance; it does not claim native adaptive optical parity.

The same 50 focused edge-effect checks pass. Current-ABI Metal verification covers light/dark at 1×/2×/3× (5,850 frames) and single-arrangement ordinary navigation push/pop at dark 3× and light 2× (1,460 frames). A separate strict sweep requires the freshly compiled offline archive and zero runtime shader/pipeline compilation. The [verification record](verification.json) retains outcomes, source identities, logs and app bundle hashes. macOS and unsigned iOS Demo builds use newly baked libraries and pipeline archives.

The [initial 12pt report](../README.md) retains the broader 38-suite regression run, four Core-only suites, full navigation catalog/gesture coverage, Switch ELF build and paired performance measurements. Those measurements belong to that setting; this radius-only follow-up does not remeasure phone thermals or reproduce every earlier check. No physical device was installed or run. The ordinary iOS command encountered a locked Xcode build database twice; an isolated `OBJROOT`/`SYMROOT` command completed verification without removing the existing database.

Reproduction: build `ScrollEdgeEffectsTests`, `PortableScrollEdgeReference`, `MetalScrollEdgeBenchmark` and `MetalNavigationBenchmark` in the existing raster harness. The retained [runner](run-metal.py) takes `cold` or `strict`; cold must precede merging/baking. Use `../merge_catalog.py --navigation-smoke` with the fresh raw harvest and the retained prior catalog, then bake both SDK outputs as described in [commands](../commands.md). Strict uses the matching macOS output. See the runner's explicit local paths for this run.
