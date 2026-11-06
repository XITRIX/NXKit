# Opaque regular tint transfer

The model is calibrated from native UIKit 27.0 central body controls in sRGB. These optical-matrix framebuffer files declare sRGB; unlike the older interaction hierarchy snapshots, they do not require Display P3 conversion. The common image loader still converts an embedded ICC profile when one is present. The [native controls](native-controls.json) cover two fixed supplied tint colors, RGB(0,122,255) and RGB(255,69,58), in both appearances at sizes 52×52, 80×80, 120×120, 132×56 and 240×160 points. The fit uses black, gray, white, red, green and blue backgrounds. Yellow is a withheld **background**, not an additional tint hue.

The previous regular tint used a single backdrop-luminance interpolation. That cannot reproduce gray and green backgrounds having similar tint response while red remains close to black. The corrected response includes source chroma, uses generic tint hue coordinates rather than named-color lookup, and reduces/compresses the dark highlight for larger minimum dimensions. Neutral and clear-glass transfer are outside this calibration.

Let `L=dot(sourceRGB,(.2126,.7152,.0722))`, `C=max(sourceRGB)-min(sourceRGB)`, `R=max(tint.r-max(tint.g,tint.b),0)`, and `G=max(tint.g-max(tint.r,tint.b),0)`. All channels are normalized sRGB. Rounded light coefficients are:

```text
response = clamp(L - .263*C*4*L*(1-L), 0, 1)^1.14
base = .4 - .235*R
amount = base + (1-base)*response
gain = .395 + .50*R - .045*G
bias = -.014 - .043*R + .022*G
linear = decodeSRGB(tint)
result = encodeSRGB(mix(max(linear*gain+bias,0), linear, amount))
```

The dark transfer uses a smooth size response, with `S=smoothstep(52,120,min(width,height))`:

```text
L2 = clamp(L - .15*C*4*L*(1-L), 0, 1)
knee = mix(.54,.24,S)
response = (1-exp(-L2/knee))/(1-exp(-1/knee))*mix(1,.61,S)
base = tint - tint*(1-tint)*.02
result = base*(1-.02*response) + (1-base)*(.12+.09*R)*response
```

These are empirical transfer coefficients, not recovered UIKit shader code. A bounded damped Gauss–Newton fit selected continuous hue/luminance/size parameters; rounding retains the improvement. The production change applies only to the regular tint path and keeps clear/custom tint behavior separate.

Analytic central-body RGB MAE on a 0–255 channel scale, equally weighting colors, sizes and channels:

| Appearance | Six training backgrounds, previous → current | Yellow holdout, previous → current |
| --- | ---: | ---: |
| Light | 5.075 → 0.629 | 2.527 → 0.314 |
| Dark | 2.578 → 0.796 | 4.253 → 0.568 |

The rounded light model predicts small blue on gray as `(0,108.6,230.8)` versus native `(0,108,231)`, and coral over black as approximately `(228.5,30.9,24.9)` versus `(228,30,25)`. This fixes the previously too-pale coral channels without a lookup for that color. Dark size compression also reproduces the reduced highlight in larger glass.

[Full validation values](tint-model-metrics.json) and [the validation script](../../Demo/GlassComparison/analyze-regular-tint.py) preserve all sampled native/predicted channels. This table checks the analytic transfer; integrated raster/native pair comparisons are reported separately in the [retained-join comparison](../liquid-glass-interaction/retained-joins/README.md).

Only two tint hues constrain the hue-dependent coefficients. Other tint hues, alpha tint, nonuniform artwork, different display color spaces and interaction-highlight color are not established by this fit. Saturated green backgrounds leave coherent dark residuals of a few RGB values. A better fit on these controls does not establish general UIKit material parity.

```sh
python3 Demo/GlassComparison/analyze-regular-tint.py \
  docs/liquid-glass-optics/native-controls.json \
  docs/liquid-glass-optics/tint-model-metrics.json
```
