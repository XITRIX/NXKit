# Spatial refraction — 2026-11-05

The reported pinched text was a shader translation error. The earlier adapter
summed inner and outer smoothstep displacements over the same bevel. Flat-color
and texture-variance tests could not establish spatial correctness.

`RefractionProbe.m` independently instantiates native `UIGlassEffect` on
64/120/160pt circles and horizontal/vertical capsules. A coordinate-coded RGB
backdrop measures actual source positions from simulator framebuffer PNGs.
Fresh filter copies neutralize color, lighting and blur while independently
varying inner/outer amount, height and refraction-mask opacity/distances.
The fixture uses process-local settings and restores no global preferences.

Observed on iOS27.0/24A434, iPhone17Pro,3×:

* Inner displacement is `amount * (1-sqrt(1-t*t))`, with
  `t=clamp(1+signedDistance/height,0,1)`. Profile values come from the actual
  attached filter archive; the120/160pt controls have a20pt inner height.
* Outer refraction is a separate sample, not an amount added to the inner ray.
  It uses the same circular profile and its own height/amount.
* The separate outer sample is blended using `refractionOpacity` times a
  **linear** signed-distance mask between `refractionDistance0/1`. With native
  defaults−1/0, only the last point near the rim receives it. Expanding that
  mask to−16/0 independently verifies the outer ray and the linear weighting.
* Setting refraction opacity to zero leaves inner refraction active and makes
  outer amount/height changes pixel-identical to identity. This distinguishes
  the two-pass law from the previous summed displacement.

The six inner-law controls contain365 decoded samples across both axes,
three independent amount/height arrangements and five geometries. Mean errors
are0.20–0.27pt, maximum below0.45pt (8-bit ramp quantization plus reduced-scale
capture). `analyze-refraction.py` asserts the independent stated law. The
production retained renderer is checked separately by
`compare-refraction.py`; deliberate64pt ramp discontinuities are excluded from
coordinate decoding when the capture footprint crosses them.

REA evidence IDs, run UUIDs, OS/geometry identity, original filter archives and
screenshots are retained here. `rea-evidence.json` records process-tree and
filesystem-observation limits rather than treating readback as render proof.
The outer pass's blur/color ordering, HDR optics and arbitrary unmeasured corner
geometries remain reconstruction details; this is not a recovered Apple shader.
