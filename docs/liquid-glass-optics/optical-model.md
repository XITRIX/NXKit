# Measured glass optics model

The regular-glass changes replace a surrounding white hairline with directed light, soften the refraction transfer, and distinguish frosted transmission from sharp surface detail. These are measured approximations to UIKit's appearance. They do not identify Apple's private shader or establish pixel parity.

The [comparison viewer and final matrix](README.md) show the integrated implementation. The experiments below explain the selected model and rejected alternatives; their isolated scores should not be presented as final matrix scores.

## Reference and measurement

The fresh reference is UIKit on the **iOS 27.0 simulator, build 24A434**, at 2× scale. Five fixed sizes cover 52 × 52, 80 × 80, and 120 × 120 circles, a 132 × 56 pill, and a 240 × 160 continuous-corner card. Both themes use identical, settled geometry. Coordinate ramps, narrow grid lines, uniform colors, and smooth colored stripes separate material, lighting, and refraction differences. The fixtures use public UIKit APIs.

Native framebuffer PNGs declare sRGB. Older touch-fixture hierarchy snapshots instead embed Display P3 profiles and require color conversion before comparison. Raw RGB subtraction across those spaces had overstated resting-tint errors. No fitted registration or color correction is applied to comparison images.

The initial source baseline was the working tree at `ee91d4a99bbdbd9c578349386dc25e70becf41cf`, including the prior staged glass corrections. Scale-correct experiments explicitly set each glass backing layer to 2×. Setting only the root layer's scale had left individual shaders rendering on a one-point grid and being enlarged afterward.

Candidate refraction scores use the band 2–12 points inside the glass, excluding the card's corners. Five sizes, two themes, and ramp/line backgrounds give 20 conditions. Adding colored stripes gives 30. Errors are mean absolute, unamplified RGB channel differences on a 0–255 scale. A separately labelled diagnostic removes the measured interior RGB bias before scoring the band; this adjustment is never applied to images or production rendering.

## Directed edge light

Retained native straight-edge controls show bright top and bottom poles with nearly flat side intensities. For a 300 × 160 card at 2×:

| Theme and background | Unlit side | First top-edge pixel |
| --- | ---: | ---: |
| Light, black | 134 | 175 |
| Dark, black | 31 | 85 |
| Light, gray | 200 | 233 |
| Dark, gray | 114 | 157 |

The native head begins fading immediately. Light-on-black excess over the side is approximately 41, 24, 10, 7, and 5 at depths 0.25, 0.75, 1.25, 1.75, and 2.25 points. There is no broad white plateau. Small circles show the same orientation and similar pole strength.

The former shader mixed white into a hairline around every edge, independently of its directional reflection lobes. The regular model now uses symmetric directional additive light with a narrow head and longer tail. Its calibrated profile is proportional to:

```text
v = max(depthInside - 0.55, 0)
direction = abs(dot(outwardNormal, lightDirection)) ^ directionalPower
light = direction * (1 - bodyLuma)^0.3
      * (0.1539 * exp(-v / 0.25) + 0.0569 * exp(-v / 2.5))
```

Production applies the public strength and width controls around this reference profile. Clear glass retains its separate existing lighting path.

With corrected backing scale, the fresh gray card's pole-minus-side error improves **2.440 → 0.833**; the first four points improve **7.669 → 0.747**. A weak absorptive edge remains appropriate: native light glass over white has an interior near 254 and unlit edge near 249–250. Removing all edge darkening would also miss the reference.

## Refraction transfer

The previous inward displacement used a squared ramp through the bevel. Its source-coordinate slope changes abruptly at the outer endpoint and produces a strong one-sided fold. Regular glass now uses a cubic Hermite envelope with half that displacement amplitude:

```text
u = clamp(depthInside / bevelWidth, 0, 1)
profile = 1 - u*u*(3 - 2*u)
displacement = 20 * (refractionStrength / 0.55) * profile
```

This gives a smooth transfer with zero envelope slope at both endpoints. Clear glass keeps its previous transfer. Bevel width and sampling bounds remain separate from displacement strength.

Twelve Hermite candidates crossed amplitudes 12, 20, 28, and 40 with three bevel widths. With identical material parameters and directed lighting:

| Candidate | Raw optical-band error | Interior-bias diagnostic | Line-pattern correlation |
| --- | ---: | ---: | ---: |
| Previous squared ramp, amplitude 40 | 6.236 | 4.393 | 0.582 |
| Selected Hermite, amplitude 20, existing width | 5.951 | 4.036 | 0.619 |
| Hermite, unchanged amplitude 40 and width | 6.717 | 5.298 | 0.364 |

The selected transfer improves the aggregate rather than every isolated condition. In this experiment, some 80-point line controls worsened by roughly 0.13–0.20 RGB. Periodic grids also admit displacement aliases; coordinate ramps help constrain them but remain sensitive to material color response. These observations support the chosen approximation, not an exact reconstruction of native optical geometry.

## Frosted body and sharp detail

Native small glass responds differently to bright and dark detail. A small light pill preserves sharp white lines while suppressing a black line that dark appearance retains strongly. A single Gaussian blur followed by linear contrast cannot reproduce that polarity difference.

The neutral regular model therefore keeps a frosted body and adds a limited sharp component. It takes half the positive sharp-minus-frosted difference in light appearance, or half the negative difference in dark appearance. Its strength fades smoothly as the shortest dimension grows from 64 to 160 points. These bounds describe the approximation; they are not a measured UIKit mode threshold.

Default neutral regular frost interpolates from 6 to 12 points in light appearance and uses 12 in dark appearance. Explicitly tinted regular glass retains its existing frost defaults. Explicit frost overrides still take precedence. The implementation saves the undispersed frosted RGB, assembles chromatic dispersion, then adds detail computed against that saved sample. Consequently, zero frost yields zero sharp-minus-frosted detail without replacing the dispersion result.

Changing frost endpoints alone worsened the selected transfer's aggregate error, **5.951 → 6.072**. Combining the broader body with the selective detail instead improved it to **4.966**. Increasing that detail component to full strength was less successful. These exploratory frost sweeps selected the small endpoint below 100 points and the large endpoint otherwise; production uses the smooth size interpolation described above and is checked separately by the final matrix.

With the later material-tone candidate, adding these optics improves the 30-condition raw error **8.921 → 6.666**, with no condition worsening by more than 0.05 RGB. Ramp error remains effectively unchanged; line error improves **5.287 → 3.095**. This is evidence for the combined model rather than for the blur defaults in isolation.

## Rejected alternatives and remaining differences

- Keeping the full displacement amplitude with Hermite worsened both error and line correlation.
- An additional full-amplitude sharp-surface path invented a duplicate black stripe in the pill's centre that native pixels do not contain. It was rejected despite an initially plausible low-resolution silhouette.
- Single-Gaussian fitting did not establish a unique native dark-card blur width: higher harmonics were near the noise floor, allowing blur and transmission gain to trade off. Reported fit widths are diagnostics, not Apple's parameters.
- A surrounding white hairline, a broad highlight plateau, and frost-only changes failed the measured controls.

**Mixed-color material response remains a tradeoff.** The later tone fit improves fixed primary-color controls, but colored stripes with the preferred optics score **14.399**, compared with **10.889** using the previous material parameters and the same optics. This difference remains visible in the viewer. Endpoint color fidelity does not establish the correct response to every mixture or texture; no scene detector or fixture-specific color correction hides that limitation.

A generic absolute-chroma vibrancy experiment illustrated why independent controls matter. It scaled only saturation above one by `chroma^gamma`, preserving gray and fully saturated primary colors. With actual production defaults, gamma 1 improved colored-stripe body error **9.864 → 4.157**, but worsened the held-out coordinate-ramp body error **1.919 → 9.018**. Gamma 2 worsened the ramp further, to **12.121**. Gray-line controls were effectively unchanged. Absolute chroma weighting was therefore rejected despite its attractive stripe result; it confuses naturally muted colors with color lost through filtering.

A narrower retained-chroma experiment instead scales excess saturation by `clamp(transmittedChroma / max(sharpChroma, 0.0001), 0, 1)`, comparing samples at the same refracted location. It preserves constant colors and largely preserves linear gradients, while reducing amplification of color lost through filtering. With actual production defaults, stripe body error improves **9.864 → 7.915**, optical-band error **14.536 → 13.042**, and interior error **12.075 → 10.352**. Every stripe element improves in both themes. Ramp body error changes only **1.91936 → 1.92141**, within the small quantization differences observed here. The tested rule uses transmitted RGB after detail and dispersion, affects only excess saturation above one, and retains explicit desaturation values unchanged. This is a general filter approximation, not a hue table or a claim about Apple's implementation.

These controls establish behavior on the tested simulator, not equivalence to the user's physical iOS 27.2 device. They also do not establish exact native continuous-corner contours or production Metal/Dawn performance. Merged-item ownership, reversed tint ordering, and retained joins are covered by the separate [join comparison](../liquid-glass-interaction/retained-joins/README.md).
