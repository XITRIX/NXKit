# ScrollPocket variable-kernel blur — 2026-11-04

Subsequent [blur-onset tuning](../blur-onset/README.md) separates radius progression from the fade curve. The implementation and measurements below describe the preceding profile; the custom kernel and 4pt maximum remain.

The former default edge path blurred at one uniform radius and composited that image through a gradient. It was introduced as a performance approximation and did not satisfy the requested progressive kernel. The fixed profile requirement means content-independent spatial control; it does not mean a uniform kernel.

The target Gaussian width is `sigma(p) = 4pt × displayScale × gradientAlpha(p)`. The custom shader changes sample offsets with this value. Filtering along the gradient first and across it second gives a common local sigma throughout the separable kernel. No gradient cross-fades a sharp source with one uniformly blurred image.

At 1×/2×, both directions use 33 taps at display resolution. At 3×, narrow kernels retain that resolution; broader kernels read a prefiltered lower-resolution first pass and use a fused 17-tap finishing pass. A one-point Gaussian prefilter suppresses downsampling aliasing, and its variance plus the linear-resampling variance is removed from the remaining custom kernel. Both tiers still change kernel width with the same gradient; the finishing shader selects between them at the corresponding radius, without an opacity blend. The finite quadrature and resampling are approximations.

At zero radius the original sharp sample is returned. Clamp crops use the current viewport’s bounds to prevent transparent-black sampling at its edges. The rejected [full-resolution iteration](../variable-kernel-full-resolution/README.md) remains available with its measurements: its 3× synchronized render median was approximately 80% higher than the old masked-uniform approximation, motivating the optimized path.

The separate 0.4→1 content fade remains. Both fields use the existing normalized profile and live safe-area/edge-container geometry, with no fixed geometry extension. The maximum remains the existing 4pt setting. Shader/filter/mask caching and single live-canvas content drawing remain; no content classification or CPU readback is added. Hard/NXKit effects are unchanged.

The general private `UIBlurEffect::effectWithVariableBlurRadius(radius, imageMask)` remains available. Its arbitrary-mask, large-radius path still uses the previously documented variance-pyramid approximation. This change specifically gives ScrollPocket’s one-dimensional radius field a direct variable kernel; it does not claim exact Apple private kernels or broad arbitrary-mask Gaussian equivalence.

## Verification

The mounted impulse test fails with the prior masked-uniform path ([before](before-tests.log)) and passes with the new kernel. At half intensity, kernel sigma halves and its variance quarters; blending a full-width blur with a sharp source instead approximately halves the variance and leaves a central sharp spike.

All **175 behavioral checks pass** ([test output](test-output.log)), including all four gradient directions and 1×/2×/3× scales, live safe-area/container changes, sparse content, mounted masks, nested glass, exact zero-radius source preservation and constant-color viewport boundaries. A three-pixel impulse keeps low-amplitude tails measurable through Skia intermediate surfaces; its known discrete variance of 2/3 is subtracted from the kernel-width check. The finite quadrature is an approximation, not an exact infinite convolution; four additional checks exercise continuity across the 3× resolution threshold. Optional picture export adds two encoding checks.

The [synthetic kernel comparison](images/comparison.png) isolates blur from content fade. Both columns use the same source and linear radius-control gradient, with the same 4pt maximum. The left is the former masked-uniform construction; the right is the actual new variable-kernel filter. It is not a native UIKit screenshot or a complete Demo optical-parity comparison.

[Build/Metal verification](verification.json) retains source and artifact hashes, cold/strict logs and raw frame CSVs. A fresh six-profile edge sweep covers light/dark at 1×/2×/3×, all four edges and configured styles, fractional scrolling, bindings, sparse content, masks, nested live glass and private-effect controls. Two ordinary push/pop smoke profiles cover navigation integration. Each sweep has 7,310 frames; strict requires the current offline archive and zero runtime shader/pipeline compilation. All GPU binaries are rebuilt for the matching source fingerprint, retaining only exact earlier raw states with provenance.

[Performance notes](performance.json) compare the pre-change 4pt path with the new path on the local Mac. These are renderer diagnostics, not phone power/temperature measurements. No physical device was installed or run. Native optical/kernel calibration remains partial.

Reproduction uses the existing raster harness: build `ScrollEdgeEffectsTests`, `MetalScrollEdgeBenchmark`, `MetalNavigationBenchmark` and `PortableScrollEdgeReference`. Run the focused test; set `NXKIT_PROGRESSIVE_BLUR_PICTURES` to export synthetic comparison PNGs. [run-metal.py](run-metal.py) runs `cold` or `strict` with explicit local paths. Merge the fresh cold harvest using `../scroll-pocket/merge_catalog.py --navigation-smoke`, configure the ordinary projects before baking, then build the macOS and unsigned iOS Demo. The retained [source hashes](sources.json) and snapshots identify the tested revision. Earlier unclamped iteration captures are excluded from final verification.

Final paired Mac measurements (three alternating repeats, 84 selected frames per version/profile): dark 3× synchronized median **3.77→4.92ms (+30.5%)**, CPU **1.40→1.55ms (+10.3%)**; light 2× synchronized median **3.50→3.40ms (−3.1%)**, CPU **1.29→1.32ms (+2.2%)**. The optimized 3× path reduces the rejected full-resolution iteration’s approximately 80% increase, but does not establish cost parity with the simpler masked-uniform approximation. No phone power/temperature claim is made.
