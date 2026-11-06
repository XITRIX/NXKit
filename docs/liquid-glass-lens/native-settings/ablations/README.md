# Native lens pass ablations

These captures isolate the native lens passes on iPadOS27.0 /24A434 at2×
scale. The [nine fresh baseline controls](../unit-reference/ink-metrics.json)
and19 single-parameter ablations use a caller-sized116×56-point selection
lens, lifted-content mode1, `warpsContentBelow=true`, and no base glass platter.
All private configuration remains in the native research executable.

[Measurements](metrics.json), [capture identities and hashes](capture-index.json),
[requests](cases.json), compressed metadata and PNGs are retained together.
The ablations cover light appearance; the blue/cyan baseline covers both
appearances. These are framebuffer observations, not extracted UIKit source.

## Verified intervention

The native-only `LensReference.m` accepts a diagnostic request such as:

```json
{"lensFilterOverrides":{"source":0},"settings":true}
```

The five filter routes each recognize a single finite scalar. Paths
are relative to the manual lens's layer:

| Name | Layer path | Filter input | Original value |
| --- | --- | --- | ---: |
| backdrop | `.0.0` | displacementMap / inputAmount |9 |
| wrapper | `.0.2.0.0` | displacementMap / inputAmount |−17.5 |
| source | `.0.2.1.0.0.1` | displacementMap / inputAmount |−11.2 |
| foreground | `.0.2.1.0.0.2` | glassForeground / inputAberrationAmount |2.94736838 |
| inner | `.0.2.1.0.0.0` | glassBackground / inputInnerRefractionAmount |−8.4 |

A separate `foregroundMargin` diagnostic route targets the foreground
`CABackdropLayer.marginWidth` at `.0.2.1.0.0.2`, accepts only0…300, and records
layer-property readback. Its original value is100. It copies/reinstalls the
unchanged foreground filter to invalidate the render node. This margin
experiment is separate from the19 zero-input captures tabulated below.

After lift settles, the helper copies the targeted filter, changes its scalar,
clears the old filter array and installs the replacement. It flushes the
transaction and waits0.2s before capture. Metadata records original, requested
and applied values, copied identity and verification status. Independent decoding
of the captured keyed archive confirms the applied zero in all19 cases.
Setting displacement amount to zero retains the pass's geometry and mask; it
does not remove the entire layer.

An earlier attempt mutated a filter object and reassigned its unchanged array.
Its model/archive readback changed, but its foreground positive control retained
the original chromatic separation. That run was discarded. Only copied-filter
captures appear here. The valid foreground-zero control removes separation to
within0.0023pt, demonstrating an actual framebuffer intervention.

## Selected content

Four radius2pt blue markers sit8pt inside the axial edges over gray32.
Each intervention has its own empty-source optical control: ten captures.
The table reports the upper and left markers; all four are in the metrics.

| Native configuration | Top G/B gap | Left G/B gap | Left blue inward from green | Top green depth |
| --- | ---: | ---: | ---: | ---: |
| Fresh baseline |1.5487 |1.0369 |.2257 |4.781 |
| Source amount0 |1.3626 |.8735 |.2258 |5.665 |
| Wrapper amount0 |1.1336 |.8198 |.0969 |5.890 |
| Foreground aberration0 |−.0004 |−.0008 |−.0023 |4.741 |
| Backdrop amount0 |1.5487 |1.0370 |.2256 |4.781 |
| Glass inner refraction0 |1.5487 |1.0369 |.2257 |4.781 |

All measurements are points. Both selected-content displacement passes affect
where the later spectral pass sees ink. Backdrop displacement and material
inner refraction do not produce the selected-content transport. This agrees
with the independently observed [portal topology](../portal-topology.md).
It does not establish the exact intermediate texture resolution or complete
pixel parity of a portable implementation.

## Ordinary backdrop

The remaining nine captures place a black4pt-diameter dot3pt inside the upper
edge, or a black24×1pt horizontal line12.5pt inside it, over gray128.
There is no selected portal. Each of three interventions has a matched plain
background control.

| Native configuration | Near green depth | Deeper green depth |
| --- | ---: | ---: |
| Fresh baseline |6.7541 |14.3593 |
| Source amount0 |6.7545 |14.3593 |
| Foreground aberration0 |6.7850 |14.3593 |
| Backdrop amount0 |2.8348 |12.5000 |

The backdrop displacement supplies radial transport. Foreground aberration
supplies spectral separation while preserving that radial displacement.
Source displacement has no measurable corresponding backdrop effect.

## Measurement and limits

Per-channel coverage is `(image-control)/(sourceRGB-control)`, excluding
absolute denominators below16/255. Centroid weight is `max(coverage-.1,0)`;
there is no upper clamp, matching `LensRefractionTests`. Fixed ROIs and
point-to-pixel truncation also match that harness. No image registration,
color fitting, or inferred optical gain is applied.

The native lens starts at stage(302.25,260). Selected unit tests use an integer
horizontal lens origin, so a quarter-point pixel-phase difference remains.
Backdrop probes use720×560 image assets generated only in the temporary native
app: dot/rectangle coverage was analytically area-sampled at8× resolution.
They are not native UIView drawing commands. The PNG outputs and hashes are
retained; those scratch input assets are not Demo production resources.
These bounds are explicitly supplied, not measurements of automatic lens growth.

The native AIR's edge fade is linear; these ablations do not justify fitting
an easing curve. Runtime centroid measurements alone cannot identify every
sampling transform, intermediate texture or compositing detail.

Recompute and verify retained pixels and all19 archived override values:

```sh
/usr/bin/python3 docs/liquid-glass-lens/native-settings/unit-reference/measure.py --check
/usr/bin/python3 docs/liquid-glass-lens/native-settings/ablations/measure.py --check
```

The scripts require Pillow and NumPy. `--check` leaves recorded evidence intact.
