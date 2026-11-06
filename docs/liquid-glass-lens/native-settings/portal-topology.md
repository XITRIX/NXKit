# Native lens portal topology

Four fresh read-only captures on iPadOS27.0 /24A434 establish the same live
portal targets for both explicit lens styles and both appearances. They use
the116×56-point manual lens, tint preference0.5, `probe: edges`, `base: none`,
and `contentSetup: none`. Each request recreates the lens and has a fresh UUID.
The source fixture invokes no new private setters for this investigation.

[Normalized topology](portal-topology.json) retains exact source paths,
filter values, capture hashes, source/header hashes and request identities.
[Compressed raw captures](portal-captures/index.json) retain all seven lenses
in each fixture. The private calls remain in the native research executable.

## Observed graph

Paths below are relative to the manual lens layer. `P` is
`.0.2.1.0.0`, the MaterialProvider container. Its four ordered children are:

1. `P.0`: CABackdropLayer with `glassBackground`. Its `@0` child is the
   background SDF output; geometry container `P.0.0.0` contains the SDF element.
2. `P.1`: CALayer with `displacementMap`. It has an SDF mask portal and a
   separate color portal.
3. `P.2`: CABackdropLayer with `glassForeground`, followed by its SDF mask.
4. `P.3`: CASDFLayer with key/fill highlight and `vibrantColorMatrix`.

The other ClearGlassView branch is `.0.2.0`. It contains the
`liftedContentWarpWrapper` (`.0.2.0.0`) and `innerShadowView` (`.0.2.0.1`).
The wrapper applies displacement−17.5 using its own11.2-point SDF and holds
the caller's lifted-content portal.

| Portal | Exact target | Role |
| --- | --- | --- |
| `.0.2.0.0.1` | nil in these no-content cases | Caller-provided lifted-content input |
| `P.1.0.0` | `P.0.0.0` | Source-displacement SDF geometry |
| `P.1.1` | `.0.2.0` | Source-displacement color input: lifted-content wrapper and inner-shadow branch |
| `P.2.0.0` | `P.0.0.0` | Foreground SDF geometry |
| `P.3.0` | `P.0.0.0` | Highlight SDF geometry |

`P.1.1` sets `hidesSourceLayer=true` and `matchesOpacity=true`. The three
geometry portals set `hidesSourceLayer=false` and `matchesOpacity=false`.
These targets were resolved by comparing live CALayer object identities with
a read-only map of the current window tree. They are not inferred from class
names or archived object summaries. The repeated `inputSourceSublayerName:
@0` identifies each filter's SDF mask child; it does not mean that the color
input is the first material sibling.

## Rendering consequence

The source-displacement pass does **not** sample the finished glassBackground
result. It samples the separate lifted-content branch. In these explicit
no-content controls, the caller portal is nil. Applying source displacement
to the entire glass-background image therefore introduces a warp that is not
present in this native graph.

The foreground filter is a later backdrop layer. Its source texture receives
the preceding composition, including glassBackground and any output of the
selected-content source pass. Its SDF input comes from the geometry portal.
This routing agrees with the shipped `glass_foreground_sdf_lpf` AIR, which
accepts separate source and SDF coordinates. The highlight is a final sibling;
it must not be baked into the image before source displacement or foreground
spectral sampling. These are graph and shader-routing observations; they do
not establish complete pixel parity or arbitrary portal behavior.

## Inner shadow and contour

All four `innerShadowView` layers report black shadow color, opacity
0.0599999987, radius3, offset(0,7), and `masksToBounds=true`. Their public
`shadowPath` and `mask` are nil, background is nil, border width is0,
`hasContents=false`, and there are no sublayers. The node has a shadow
configuration but no captured drawable silhouette; these captures do not
support inventing an additional opaque capsule or a hard border for it.

The material's independent ring-shadow settings are present:
blur radius3, stroke width4, offset8, mask0. Selection-style ring opacity
is0.1000000015 in both appearances; small-style opacity is0.150000006 in light
and0.400000006 in dark. No ordinary layer border appears on the captured
material/inner-shadow nodes. The dark contour should first be investigated
through this native ring composition and the separate last-stage highlight.
This report does not prove which individual contribution accounts for every
edge pixel in the supplied device image.

## Reproduce

```sh
/usr/bin/python3 Demo/GlassComparison/build-lens.py /private/tmp/lens-portals
/usr/bin/python3 Demo/GlassComparison/LensSettingsProbe/capture.py \
  --device IPAD_SIMULATOR_UDID --fixture /private/tmp/lens-portals \
  --output /private/tmp/lens-portals --cases LAYER_CASES_JSON --tints 0.5
```

The retained index contains the four exact case dictionaries. Runtime
inspection explicitly reads `sourceLayer`, public shadow-path commands,
mask presence and ordinary CALayer state. It does not invoke arbitrary
zero-argument methods or change native layer properties.
