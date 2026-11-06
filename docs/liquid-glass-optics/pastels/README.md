# Partially saturated solid-color holdouts

[Open the pixel overlay viewer](comparison.html).

These six iPadOS 27.0 framebuffer scenes validate mint `(128, 200, 160)`, lavender `(180, 120, 200)`, and peach `(200, 160, 128)` in both themes. Each scene contains 52, 80, and 120 point circles, a 132 × 56 pill, and a 240 × 160 continuous-corner card. The original 60-scene matrix is preserved separately.

The holdouts check that texture refinements retain the flat-color transfer between neutral gray and fully saturated RGB backgrounds. Their colors were not used to fit the initial tone parameters. They have no old-renderer baseline; unavailable baseline controls are disabled in the viewer.

Across the 30 additional glass conditions, final production has mean absolute RGB error of **1.402** in the body, **2.223** in the rim, and **2.251** in the refraction band, in 0–255 sRGB channel units. The scored regions exclude the exterior default shadow, which remains visible in the residual views. See [metrics](metrics.json), [native center colors](native-controls.json), and [capture geometry](native-metadata.json).

Source framebuffers declare sRGB, and untouched pixels match the specified background colors exactly. REA file-observation evidence: `ev_224e5fff69e56ec41afae373683b501ada7b844362c961a201391bb88990b7a8`. This verifies saved captures and metadata; it does not claim another native execution.
