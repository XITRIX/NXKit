# UISlider track endpoint smoothing

2026-11-06, NXKit working tree after `bd143cc`. Native reference is UIKit on
Simulator iOS 27.0/24A434 at 3×. This tests filled-track geometry, separately from
the already measured LiquidLens lift/release springs.

The old default track jumped from 0 to almost 18.5pt on the first nonzero value,
and from almost 281.5 to 300pt at the maximum. Custom 30pt thumbs jumped 15pt.
UIKit instead tapers effective cap width across the first/last 2% of normalized
travel. The resulting fill position is quadratic in those regions and continuous
at 0, .02, .98 and1. Thumbless tracks are linear. Custom thumbs contribute their
public resting width; lifted lens size does not change the taper.

[Static evidence](../endpoint-static/README.md) recovers the exact mapper from
shipped UIKitCore. REA evidence:
`ev_35b06d5bdee12e52ea9ec688c9c7b1f9bf421dd66d34af9d7827d42daed32c97`.
This is corroborated independently by live native layer measurements and real
OS screenshots in [native](native/). No endpoint equation is evaluated while
generating the native regression expectations.

![Measured before/after endpoint geometry](comparison/endpoints.png)

The chart uses measured track-layer dimensions, rather than image similarity.
The resting opaque thumb can hide the discontinuity underneath it. Before/after
CSV captures are retained alongside it; native dots come directly from UIKit.

## Coverage and findings

- Default, thumbless,30×40pt custom thumb, endpoint images, disabled, 180/300pt
  widths, −10…10 ranges, ticks, neutral configurations and center neighborhoods.
- Both themes: 26 stable pages with tick-only defaults. Five additional light
  pages disable tick-only input to measure continuous configured rendering.
- 608 native endpoint/track expectations, maximum absolute error 0.0000611pt.
- Zero-width and narrower-than-thumb safety; dense monotonicity and continuity
  around 0/.02/.98/1. Both ends reach exactly empty/full.
- 360 Raster/Metal appearance cases cover 0/.0001/.01/.02/.25/.75/.98/.99/.9999/1,
  both themes, 1×/2×/3×, enabled/disabled and three lift poses. 870 motion frames
  retain the previous 150 Raster comparisons. See [verification](verification/).

Public `trackConfiguration.neutralValue=.5` does **not** anchor the blue fill in
these native Glass captures, including non-unit ranges. We removed the invented
bipolar fill. The private fluid `neutralPosition` code can map both bar endpoints,
but that does not establish public `neutralValue` behavior.

Configured UIKit exposes a further distinction: with `allowsTickValuesOnly=NO`,
the rendered bar follows the unsnapped requested value, while the observed public
value getter remains snapped. Geometry regressions use the actual public setter
input for continuous pages and the observed snapped value for tick-only pages.
Matching that getter/visual split and complete tick presentation is separate
remaining work; this change does not claim full configuration compatibility.

Custom track images switch native UIKit to a legacy provider with a 31×31pt thumb
and different track geometry. Its stable native pages are retained, but excluded
from the 608 Glass expectations. NXKit still uses its modern provider with custom
track images; a complete legacy-provider port remains unimplemented. It should
not be described as matching the LiquidLens endpoint law.

## Actual touch sweep

The [native drag recording and crops](native/drag/) follow one real OS contact
from minimum to maximum, beyond maximum, back to minimum and beyond minimum in
each theme. They corroborate the resting-thumb taper while the knob is lifted.
Native overdrag additionally stretches/thins the track container; this separate
elastic track deformation is not implemented by this endpoint-fill correction.
Ordered setter comparisons are in [ordering-summary.json](ordering-summary.json).
Continuous configured getter/visual splitting also causes native setter no-op
quirks; those are recorded, rather than imitated as part of this geometry fix.

## Reproduction

```sh
/usr/bin/python3 Demo/SliderComparison/capture-endpoints.py SIMULATOR /tmp/native-endpoints
/usr/bin/python3 Demo/SliderComparison/capture-endpoints.py SIMULATOR /tmp/native-continuous --continuous-ticks
/usr/bin/python3 Demo/SliderComparison/generate-endpoints.py --check
cmake --build Demo/build/ios-sim-ninja --target NXKitSliderTests SimulatorGlassBlurTests -j8
xcrun simctl spawn SIMULATOR /absolute/path/NXKitSliderTests.app/NXKitSliderTests
xcrun simctl spawn SIMULATOR /absolute/path/NXKitSliderTests.app/NXKitSliderTests --endpoints /tmp/after
/usr/bin/python3 Demo/SliderComparison/compare-endpoints.py /tmp/before /tmp/after
xcrun simctl spawn SIMULATOR /absolute/path/SimulatorGlassBlurTests --slider-only
```

Retained gzipped native trees decompress losslessly; per-case provenance hashes
verify the original JSON and OS screenshots. Generated C++ data has each original
JSON hash. Simulator checks establish portable geometry/rendering, not physical
iPhone timing or thermal performance.
