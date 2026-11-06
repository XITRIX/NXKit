# Tab bar press: UIKit and NXKit

Open [the aligned pixel viewer](comparison.html) or the [before/after overview](overview.png).
Each overview scene shows resting above held. The native fixture is a real
four-item `UITabBar` receiving XCTest-delivered OS touch-down, hold, a short
movement, and release. UIKit owns its material, model bounds and transforms.
Production NXKit receives its own routed touch; native geometry is **not**
injected into the NXKit renderer.

Evidence: iPhone 17 Pro simulator, **iOS 27.0 / 24A434**, native output normalized
to 2×. Six black/gray, blue and brown cases cover both themes. Four later
coral/green captures validate the response without changing its coefficients.
The supplied physical-device screenshots motivate the cases, but are not used
as pixel ground truth because their application backgrounds differ.

## Observed geometry

| Element | Resting model size | Held model size / transform |
| --- | --- | --- |
| Four-tab platter | 360×62 pt | Model unchanged; scale 1.039272×1.039272, visual size 374.1379×64.4349 pt |
| Selection lens | 95×54 pt | 111×70 pt, before ancestor inflation and its own elastic response |

The platter's existing inflation already matches. NXKit's previous lens grew
only 8 points total; the corrected Demo adds 8 points on **each edge**, giving
16 points total on both axes. The same growth agrees with the older native
compact-tab traces (36→52 pt height). Model-bound error in all ten phone cases
is below 0.001 pt; held platter scale error is below 0.00001.

## Observed highlight

The fixed five-by-five-pixel probe lies between the first two item centers,
outside symbols, captions, selection and the perimeter. On dark glass over
black, UIKit changes RGB 32→44; the old white overlay changed 32→121. The new
response changes 32→44. On blue and brown, UIKit preserves/increases chroma
rather than blending every channel toward white.

Long neutral regular surfaces now use a moderate color gain, slight chroma
gain, and a neutral-only lift. The size/aspect transition is smooth. Clear
glass, explicitly tinted regular material and short-button feedback retain
their previous paths. Resting shading and platter inflation are unchanged.
This is a measured approximation, not a recovered Apple shader or a claim
that every wide UIKit control uses identical parameters.

For the six original cases, see `metrics.json` for before/current full-crop
errors and probe errors. Maximum channel error at the held probe is 0–5/255;
press-change error after removing resting bias is 0–3/255. On the four
independent coral/green cases, press-change error is at most 8/255. The script
gates model bounds, platter scale and press-change error against native output.

## Alignment and limitations

Native stage/window metadata determines the crop; both sides share the same
platter origin and size. No fitted translations, rescaling, color matching or
pixel warping is used. The viewer offers a 50% overlay, absolute difference,
and removal of pixels matching within 2/255. Original snapshots, compressed
full layer/touch traces, source hashes, capture manifests and renderer hashes
are retained under `native/` and `provenance.json`.

The full-crop MAE includes remaining lens optics, resting selection fill,
selected-icon vibrancy, platform symbol/font metrics, shadows and rim shading.
Those residuals remain visible; accurate lift geometry and press probes do
not establish complete tab-bar pixel parity. Capture timing perturbs native
physics, so contact-1 is chosen during the stationary pre-drag hold and its
actual time/active-touch flag are checked. Physical iOS 27.2, Metal/Dawn output,
other aspect ratios and arbitrary backdrop motion remain unverified.

## Reproduce

```sh
python3 Demo/GlassComparison/capture-lens-motion.py DEVICE /tmp/tab-press-native Demo/GlassComparison/tab-press-cases.json
cmake --build /tmp/nxkit-container/raster-tests --target TabPressRenderer LensTabTests GlassScreenTests MaterialTests
python3 Demo/GlassComparison/compare-tab-press.py /tmp/tab-press-native /tmp/nxkit-container/raster-tests/TabPressRenderer docs/liquid-glass-lens/tab-press
python3 Demo/GlassComparison/capture-lens-motion.py DEVICE /tmp/tab-press-validation Demo/GlassComparison/tab-press-validation-cases.json
python3 Demo/GlassComparison/compare-tab-press.py /tmp/tab-press-validation /tmp/nxkit-container/raster-tests/TabPressRenderer docs/liquid-glass-lens/tab-press --append
```

The retained before PNGs came from the renderer before this change. The optional
fourth path argument accepts that renderer's saved PPMs; it is not needed to
run native-output gates. `LensTabTests` additionally checks growth, touch-down
approach, tracking, lift during pan handoff/holds, release/cancellation and the
source-row punchout.
