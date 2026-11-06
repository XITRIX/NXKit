# UIKit and NXKit glass optics

The comparison uses five fixed geometries: 52 × 52, 80 × 80, and 120 × 120 circles; a 132 × 56 pill; and a 240 × 160 card with continuous corners. It exercises regular glass with no tint, blue tint, and coral tint in both themes, over black, white, gray, red, green, blue, yellow, a two-channel coordinate ramp, narrow grid lines, and smooth colored stripes.

[Open the aligned overlay viewer](comparison.html). It offers an opacity slider, native and NXKit views, transparent unmatched pixels, absolute RGB differences, and red/cyan registration checks. Matching pixels are removed either exactly or when every channel differs by at most 2 / 255. The threshold is explicit; it does not change the metrics.

![UIKit, NXKit and amplified absolute differences](overview.png)

The overview is downsampled; its difference column is amplified ×4. The viewer retains full-resolution pixels.

## Capture and alignment

`OpticalReference.m` is a separate application linked to Apple's UIKit. It uses public `UIGlassEffect`, `UIVisualEffectView`, and corner-configuration APIs. The native reference is **iPadOS 27.0, build 24A434**, on a 2× iPad simulator. Every screenshot comes from the simulator framebuffer after the material has settled. Effects are interactive but untouched, with empty content views, so this matrix isolates resting materials. Blue is fixed sRGB (0, 122, 255) and coral is (255, 69, 58), rather than dynamic system colors. The scene origin and each presentation rectangle are measured in the running app; the NXKit renderer consumes those exact coordinates. There is no fitted translation, scaling, exposure, or color correction.

Color-space handling matters. These framebuffer PNGs declare **sRGB** in a PNG sRGB chunk. Older touch-fixture hierarchy snapshots used embedded **Display P3** profiles. Comparing their raw RGB bytes to NXKit's sRGB output incorrectly exaggerates tint differences. The shared `comparison_images.py` loader converts embedded profiles through LittleCMS; the framebuffer captures already use sRGB. All raw framebuffer files remain in the run directory; four [original framebuffer samples](raw) are retained here as well. Comparison crops have an explicit sRGB ICC profile.

An untouched 67,200-pixel region of each ramp, stripe, and line capture matches the shared input image byte for byte after exact 2× nearest expansion. This checks the source image, color space, and interpolation before measuring glass.

The two baselines distinguish the earlier backing-layer scale of 1 from a calibrated scale of 2. The latter isolates material changes from the scale correction. All native and calibrated NXKit images resolve to 1440 × 880 pixels for the same 720 × 440 point scene. Public frames are not changed to improve the fit.

## Metrics

[Full matrix metrics](metrics.json), [native geometry](native-metadata.json), [center-color controls](native-controls.json), and [provenance](provenance.json) accompany the viewer.

Body, rim, and refraction-band errors are measured separately in unamplified sRGB channel units from 0 to 255. The body excludes the optical perimeter. The rim uses points 0.75–2.75 points inside the edge; the refraction band uses 3–12 points. Card rim and band metrics conservatively exclude twice the corner radius around each corner, using only straight segments and avoiding a fitted approximation to UIKit's private continuous-corner spline. The three scored regions are inside the glass and exclude the exterior default shadow. Exterior shadow differences remain visible in the full-frame residual images; this patch does not change the existing default shadow. These are appearance measurements, not a full-appearance or pixel-parity assertion.

The coordinate ramp and line backgrounds expose lens displacement; uniform backgrounds isolate tint and directional lighting. Full-image exact-match counts are reported alongside glass-only counts so a large matching backdrop does not hide glass errors.

## Measured result

The main matrix contains 60 native scenes and 300 glass conditions. Values below are mean absolute RGB error in 0–255 sRGB channel units, averaged equally over the five sizes and all scene configurations.

| Region | Previous production, scale 1 | Previous material, calibrated scale 2 | Final production |
| --- | ---: | ---: | ---: |
| Body | 6.703 | 6.701 | **1.305** |
| Directional rim | 9.085 | 8.035 | **3.065** |
| Refraction band | 6.981 | 6.979 | **1.850** |

Both themes and every tint group improve in aggregate. Smooth colored stripes remain the hardest background: body error is 4.456, rim error 7.943, and refraction-band error 5.899. Individual stripe conditions can still be worse than before; the light blue 240 × 160 card has body error 5.778 versus 0.960 previously. The residual views expose these remaining errors, including local refraction phase and color-detail differences. This is not a pixel-parity claim.

The separate six-scene pastel holdout has body error **1.402**, rim error **2.223**, and refraction-band error **2.251** across 30 additional glass conditions.

## Reproduce

From the repository root, using a Python environment with Pillow and NumPy:

```sh
python Demo/GlassComparison/prepare-optical.py /private/tmp/nxkit-optical-matrix
python Demo/GlassComparison/build-optical.py /private/tmp/nxkit-container/raster-tests /private/tmp/nxkit-optical-matrix
python Demo/GlassComparison/capture-optical.py DEVICE_ID /private/tmp/nxkit-optical-matrix
python Demo/GlassComparison/compare-optical.py /private/tmp/nxkit-optical-matrix /private/tmp/nxkit-optical-matrix/after --render=/private/tmp/nxkit-optical-matrix/OpticalRenderer
```

The renderer can accept an external SkSL file as its second argument for controlled material experiments. It retains the production effect's resolved uniforms, blur, and sampling radius. `build-optical.py --renderer-only --core-archive=PATH` links the same adapter against a preserved core archive without rebuilding or modifying a shared build.

`export-optical-report.py MATRIX BEFORE_SCALE1 BEFORE_SCALE2 AFTER OUTPUT` writes the offline viewer and selected full-resolution images. The native fixture is independent of NXKit and does not access or modify Apple's private material settings.

## Scope

The test establishes behavior on the installed iPadOS 27.0 simulator. It does not establish equivalence to an iOS 27.2 physical device, every display gamut, or every continuous-corner contour. Join ordering and merged-after-release comparisons are documented separately in the interaction comparison report. Residual image differences remain visible rather than being hidden by alignment or color fitting.

The REA finding ledger records the final 60-scene coverage, corrected observed geometry, all 60 original sRGB framebuffer files, and exact untouched RGB primary pixels in evidence `ev_c9d4e7f0a2b4cd30f79d1e482b8d1a87e65734537ee0c3c9ba7f1043dc0c92ee`. This observes saved framebuffer files and metadata; it does not itself claim to have executed the native app. An earlier pilot evidence entry used the previous row placement and is superseded by this completed matrix.

## Pastel holdouts

Six additional scenes cover solid mint `(128, 200, 160)`, lavender `(180, 120, 200)`, and peach `(200, 160, 128)` in both themes. These partially saturated colors validate texture refinements between gray and the fully saturated RGB controls. They use the same five geometries and have no old-renderer baseline. [Open the holdout overlay viewer](pastels/comparison.html) or inspect its [metrics](pastels/metrics.json).

The source asset generator includes these backgrounds. Capture them into a separate run directory with `capture-optical.py DEVICE AUXILIARY_DIRECTORY mint lavender peach --neutral-only`, after preparing and building the fixture there. File-observed source colors, six raw sRGB framebuffers, and OS identity are recorded in REA evidence `ev_224e5fff69e56ec41afae373683b501ada7b844362c961a201391bb88990b7a8`.
