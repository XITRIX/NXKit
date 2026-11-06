# Native blur reconstruction and capture resampling — 2026-11-05

Target: UIKit on iOS27.0/24A434, iPhone17Pro simulator, display scale3.
`KernelStepProbe.m` and `KernelStepProductionProbe.m` place eight independent
390×80pt regular-glass surfaces at x5, y70+95×index, radius40 over a vertical
black/white boundary at x200. Fresh filter copies isolate blur with identity
face transfer and disabled refraction, fill, bleed, shadows and highlights.
Only process-local preferences change.

REA evidence `ev_e74ae0671aced1ffe0d1ef7663fd759e574a27699282cd2bb8a1684bb4a2c8e1`
records two fresh launches at preference .5 and1 for radii0/.5/1/2/3/5/8/12.
`ev_f157f49cbf70b06a92b28c22edc58f4c49311d1c8f2f81b1c6756553808f64a4`
records another two launches for .375/.75/1.5/2.5/4/6/10/20. Raw JSON readbacks,
PNGs and replay manifests remain in `kernel-step` and `kernel-step-production`.
Initial directory snapshots were absent, so overall replay truncation is true;
final snapshots include all four output files and hashes. REA observes the
driver, not internal GPU execution. `verification.json` hashes local evidence.

Subsequent static evidence is the shipped QuartzCore default.metallib from the
same runtime, SHA256
`5d59e4e26569b90f3e7e6331be5cf8931aab96718306ee62a72862e4e5192073`.
Apple's disassembler exposes numeric shader equations. No Apple binary or
full disassembly is copied into this repository. The recovered fragment mip
stencil has13 taps: center .1054970026; near axes .09018599987; near diagonals
.07709799707; far axes .05634099990. Near/far offsets are1.960085034 and
3.920675993 previous-level texels. The compute alternative uses rounded half
weights and exact2/4 offsets; selection and resource preparation matter.

CPU GlassBackgroundFilter::render supplies the radius factor1.6. Given radius
R, physical scale D, and capture scale C, r=1.6*R*D*C. Shader LOD is
max(0,log2(r<2 ? 1+.5*r : r)). Importantly, native preparation may skip the
base mip when its minimum active radius reaches2 captured pixels. It halves
sampling scale **before** this nonlinear law: the equivalent original-atlas
LOD becomes1+max(0,log2(.5*r<2 ? 1+.25*r : .5*r)). Subtracting1 after the
logarithm produces the wrong response between radii2 and4.

The portable default selects the complete shipped all-fragment path, including
its skip-base preparation. The alternative compute stencil is retained
separately; mixing compute and fragment stages is not treated as an equivalent
reference path. The all-fragment path matches the retained native step and
stripe fixtures better; this is selection between native implementations, not
a fitted kernel or tolerance adjustment. Device-specific native capability
flags are not queried by the portable renderer.

The production renderer now uses an explicit reduced-resolution pyramid and
trilinear atlas sampling, shared by the independent main/fill/bleed/shadow
radii. Blur opacity selects radius, rather than mixing clear and fixed blur.
A single packed child bounds final material sampler count. The original sharp
backdrop remains separate for final composition. The earlier fitted two-
Gaussian lookup is removed from production. Its earlier measured errors are
historical and do not validate this replacement.

`NativeGlassBlurTests` exercises the actual Core pyramid and the same sampling
helper as production against32 native step controls and32 independent stripe
controls. It preserves the existing strict half-byte step and1.8-byte stripe
mean limits. The parent task records the final post-integration run; a passing
C++ build alone is not equivalence evidence. HDR, device GPU branch selection,
and arbitrary transformations require their own validation.

The earlier square-wave captures also exposed a capture aliasing bug: native
.125 capture averages an8px source footprint. The Core capture now averages
up to4×4 bilinear pairs on the reduced-resolution grid. At .5 it uses one
bilinear pair, matching a2px footprint. This is independent from mip filtering.

Core's pyramid descriptor and Gaussian mixture descriptor are generic rendering
extensions. Existing single-sigma setters retain their public Gaussian units
and clear the pyramid override; explicit zero sigma remains sharp. Named
inputs retain independent Gaussian/mixture sampling. Core tests cover copies,
invalid descriptors, normalized taps, and preservation of earlier state.
