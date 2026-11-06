# Independent wide-lens control

This held-out native 27.0 / 24A434, 2× pair uses a 200×56-point lens. It changes
only lens width and the declared side-marker positions from the 116×56 baseline;
foreground margin remains 100, with no diagnostic overrides.

Top/bottom G/B separation is 1.99717/2.00357 pt. Left/right tangential separation
is .78780/.78649 pt, with blue .33274/.33398 pt inward from green.
[Exact measurements and provenance](metrics.json), stage PNGs, requests and
compressed metadata are retained. The geometry was not used to choose an
optical gain or capture-extent rule.

The final portable renderer's largest axial G/B gap error is **.051 pt**;
its largest side-normal gap error is **.044 pt**. Mean RGB centroid position
error across all four markers is .137 pt, with a maximum of .341 pt.
[Portable measurements](portable-metrics.json) retain each channel's errors,
renderer/source/ABI hashes, and the declared geometry. Portable blue/control
PNGs are retained alongside the native pair. These residuals remain measurable;
this comparison does not establish complete optical parity.

The portable virtual extent is `logical size + sqrt(2) × margin`. That is an
inferred reconstruction policy validated here, not a recovered native allocator
rule; native texture padding and allocation dimensions remain unmeasured.

Verify all held-out measurements using declared/captured model geometry:

```sh
/usr/bin/python3 docs/liquid-glass-lens/native-settings/unit-reference/check-holdouts.py
```
