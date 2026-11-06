# ScrollPocket variable-kernel blur — 2026-11-04

Historical full-resolution iteration (`c5d2…`), retained because the paired 3× measurement found an approximately 80% synchronized-render cost increase. See the [optimized variable-kernel follow-up](../variable-kernel/README.md) for the subsequent implementation and final verification.

The former default edge path blurred at one uniform radius and composited that image through a gradient. It was introduced as a performance approximation and did not satisfy the requested progressive kernel. The fixed profile requirement means content-independent spatial control; it does not mean a uniform kernel.

The new edge shader computes `sigma(p) = 4pt × displayScale × gradientAlpha(p)` and samples a normalized 33-tap Gaussian spanning ±3 local sigmas. Two passes form a local two-dimensional kernel. For a one-dimensional edge field, filtering along the gradient first and across it second ensures every input of the second pass uses that output’s same sigma. No sharp/uniform-blur cross-fade is involved. Full display resolution preserves detail as sigma approaches zero; at zero the shader returns the original sample. A clamp crop uses the current viewport’s bounds to prevent transparent-black sampling at its edges.

The separate 0.4→1 content fade remains. Both fields use the existing normalized profile and live safe-area/edge-container geometry, with no fixed geometry extension. The maximum remains the existing 4pt setting. Shader/filter/mask caching and single live-canvas content drawing remain; no content classification or CPU readback is added. Hard/NXKit effects are unchanged.

The general private `UIBlurEffect::effectWithVariableBlurRadius(radius, imageMask)` remains available. Its arbitrary-mask, large-radius path still uses the previously documented variance-pyramid approximation. This change specifically gives ScrollPocket’s one-dimensional radius field a direct variable kernel; it does not claim exact Apple private kernels or broad arbitrary-mask Gaussian equivalence.

## Verification

The mounted impulse test fails with the prior masked-uniform path ([before](before-tests.log)) and passes with the new kernel. At half intensity, kernel sigma halves and its variance quarters; blending a full-width blur with a sharp source instead approximately halves the variance and leaves a central sharp spike.

All **171 behavioral checks pass** ([test output](test-output.log)), including all four gradient directions and 1×/2×/3× scales, live safe-area/container changes, sparse content, mounted masks, nested glass, exact zero-radius source preservation and constant-color viewport boundaries. A three-pixel impulse keeps low-amplitude tails measurable through Skia intermediate surfaces; its known discrete variance of 2/3 is subtracted from the kernel-width check. The finite 33-tap Gaussian is an approximation, not an exact infinite convolution. Optional picture export adds two encoding checks.

The [synthetic kernel comparison](images/comparison.png) isolates blur from content fade. Both columns use the same source and linear radius-control gradient, with the same 4pt maximum. The left is the former masked-uniform construction; the right is the actual new variable-kernel filter. It is not a native UIKit screenshot or a complete Demo optical-parity comparison.

[Build/Metal verification](verification.json) retains source and artifact hashes, cold/strict logs and raw frame CSVs. A fresh six-profile edge sweep covers light/dark at 1×/2×/3×, all four edges and configured styles, fractional scrolling, bindings, sparse content, masks, nested live glass and private-effect controls. Two ordinary push/pop smoke profiles cover navigation integration. Each sweep has 7,310 frames; strict requires the current offline archive and zero runtime shader/pipeline compilation. All GPU binaries are rebuilt for the matching source fingerprint, retaining only exact earlier raw states with provenance.

[Performance notes](performance.json) compare the pre-change 4pt path with the new path on the local Mac. These are renderer diagnostics, not phone power/temperature measurements. No physical device was installed or run. Native optical/kernel calibration remains partial.

Reproduction uses the existing raster harness: build `ScrollEdgeEffectsTests`, `MetalScrollEdgeBenchmark`, `MetalNavigationBenchmark` and `PortableScrollEdgeReference`. Run the focused test; set `NXKIT_PROGRESSIVE_BLUR_PICTURES` to export synthetic comparison PNGs. [run-metal.py](run-metal.py) runs `cold` or `strict` with explicit local paths. Merge the fresh cold harvest using `../scroll-pocket/merge_catalog.py --navigation-smoke`, configure the ordinary projects before baking, then build the macOS and unsigned iOS Demo. The retained [source hashes](sources.json) and snapshots identify the tested revision. Earlier unclamped iteration captures are excluded from final verification.
