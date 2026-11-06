# Native slider endpoint cap mapping

Bounded static audit: 2026-11-06, shipped UIKitCore from iOS27.0 Simulator24A434.
This explains the cap mapping itself rather than fitting a taper to pixels.
Native runtime measurements remain independent corroboration.

## Exact equation

Let W be `trackBounds.width`, T the width returned by the public
`thumbRectForBounds:trackRect:value:` implementation, and u a normalized
endpoint of the incoming `barFrame`. For 0≤u≤1 the native map is:

```
capWidth(u) = T * min(u / .02, 1, (1-u) / .02)
endpoint(u) = capWidth(u)/2 + u * (W - capWidth(u))
```

Map both bar endpoints independently. The filled span is
`[endpoint(barFrame.minX/W), endpoint(barFrame.maxX/W)]`.
For a leading-neutral ordinary slider this reduces to width=endpoint(value
fraction). It approaches0 continuously, approaches W continuously at1,
and uses the ordinary thumb-center formula away from the first/last2%.
The cap-width ramp is linear; the resulting endpoint position is quadratic
within each end region. It is not an easing curve, a point-sized threshold,
an alpha fade, or a branch only at exactly0/1.

For W=300,T=37 the exact real-number equation gives:

| u | Effective cap width | Mapped endpoint |
| --- | ---: | ---: |
| 0 | 0 | 0 |
| .005 | 9.25 | 6.07875 |
| .01 | 18.5 | 12.065 |
| .02 | 37 | 23.76 |
| .5 | 37 | 150 |
| .98 | 37 | 276.24 |
| 1 | 0 | 300 |

These table values evaluate the recovered expression; they are not native
measurements and do not include native float-to-double rounding.

## Entry points and variants

The [mapper disassembly](disassembly/endpoint-mapper.txt) is the bounded
block `0x1c50420f0`…`0x1c5042314` of shared fluid-layout function
`0x1c5041efc`. It reads barFrame, divides each endpoint by trackBounds.width,
and compares against literal double constants.02/.98. Their encoded bits are
`0x3f947ae147ae147b` and `0x3fef5c28f5c28f5c`.

`_UILerp`, at `0x1c4121984`, is confirmed directly as
`_UILerp(t,a,b)=(1-t)*a+t*b`, so the edge-region call with a=0,b=T really
scales cap width before applying the endpoint map.

The [caller](disassembly/thumb-width-caller.txt), `0x1c5048e08`…`0x1c5049048`,
obtains the public thumb rect and passes its returned width unmodified to
the mapper. A custom thumb image therefore supplies its actual public width;
the map does not hardcode37 or use the temporarily enlarged lifted width.
The slider-style virtual getter used by the Glass visual element resolves to
the [style check](disassembly/thumb-style-switch.txt) at `0x1c5046f10`:
`sliderStyle==1`. For that thumbless branch the caller passes T=0 and a
bypass flag; the mapper retains raw barFrame instead of applying cap mapping.

No complete audit of historical nonfluid providers is claimed. The retained
code establishes the current shared fluid path used by the inspected Glass
visual element and its explicit thumbless branch.

## Neutral position

`-[_UIFluidSliderInteraction _issueUpdate]`, entry `0x1c433bf44`, clamps the
current fraction and configuration.neutralPosition to[0,1], converts the
neutral through float, constructs a bar span using their absolute difference,
and writes setBarFrame at `0x1c433c408`. Its [bounded span block](disassembly/neutral-span.txt)
supports applying the cap map to both ends of a neutral-to-current span,
including a neutral endpoint within the2% edge regions.

The [configuration bridge](disassembly/neutral-bridge.txt),
`0x1c5042b40`…`0x1c5042ba4`, conditionally reads a private protocol's
neutralPosition and forwards it into fluid configuration. This does not prove
that every public UISliderTrackConfiguration subclass's neutralValue reaches
that private protocol. Use observed public neutral behavior independently;
do not expand API semantics solely from this private bridge.

## Evidence and limits

[REA record](rea-evidence.json):
`ev_35b06d5bdee12e52ea9ec688c9c7b1f9bf421dd66d34af9d7827d42daed32c97`.
The command exits0 and retains exact native binary/disassembly hashes.
Overall capture reports truncation; the complete structured result is present
across its terminal frames. A first audit attempt timed out before producing
results; the final audit uses one bounded LLDB session and is the evidence
reported here. It does not launch UIKit or measure runtime pixels.

UIKitCore SHA256:
`5cefe8cfb213b0dfcb378feb86ddd9062fda37fb4f331b3ceb782070a20292a4`.

```sh
/usr/bin/python3 docs/slider/endpoint-static/audit.py
```

The script requires that matching extracted native artifact at its documented
local scratch path. It does not change a Simulator or physical device.
