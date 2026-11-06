# Pastel stability controls

[Open the aligned comparison](comparison.html) · [Main report](../README.md)

Six settled UIKit framebuffer scenes use mint (128, 200, 160), lavender (180, 120, 200), and peach (200, 160, 128), in both themes and all five sizes. These partial-color controls protect the existing resting body response while mixed-color refraction changes.

Before is the production renderer at `9345af9`, at the same 2× scale. After uses the current integrated core. No fitted alignment, exposure or color transform is applied. The color loader converts embedded ICC profiles to sRGB; these native framebuffers explicitly declare sRGB. Transparent residuals suppress exact or per-channel ≤2 matches.

| Region | Before | After |
| --- | ---: | ---: |
| Body | 1.401635 | 1.401636 |
| Rim | 2.223500 | 2.223500 |
| Optical band | 2.251279 | 2.251280 |

Scores exclude exterior shadows and conservative card-corner regions. All captures use iPadOS 27.0 build 24A434, not the user’s physical 27.2 device. Full metrics and capture/build provenance are linked in the viewer.
