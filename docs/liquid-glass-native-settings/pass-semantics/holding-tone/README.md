# Holding-tone isolation

Two fresh native UIKit processes use the coordinate-coded refraction fixture,
neutral face, zero blur/refraction/bleed/shadow/highlights and white=0.5.
`x-holding-half` enables the whole interior; `x-holding-gradient` uses signed
distance endpoints -32 and 0. Each JSON contains fresh run identity, original
archive, requested mutations and readbacks. PNGs are simulator framebuffers.
The shared fixture source and driver are under `NativePassProbe`.

Readable QuartzCore AIR establishes a premultiplied multiplicative-white pass:

```
t = enabled * (1 - saturate((sdf - distance0)/(distance1 - distance0)))
a = saturate(material.a)
target = (material.rgb * white * a / max(material.a, 1e-6), a)
result = mix(material, target, t)
```

This is applied after material coverage and filter highlighting. It is not a
luminance ceiling in face transfer. Over opaque input the flat framebuffer
check has mean error 0.218 and maximum 0.5/255 over 450 components. The simple
full-resolution geometric-SDF gradient check has mean error 0.357, maximum
6.336/255; its native reduced-resolution capture-grid phase is not modeled.
These captures corroborate the AIR law without establishing arbitrary HDR or
physical-device behavior. See `evidence.json` for the REA identity and limits.

Reproduce after building/installing `RefractionProbe.m`:

```
python3 Demo/GlassComparison/NativePassProbe/capture-refraction.py --holding-tone --device DEVICE --output REPORT
```
