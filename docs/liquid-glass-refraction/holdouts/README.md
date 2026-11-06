# Independent wave frequencies

[Open the aligned comparison](comparison.html) · [Main report](../README.md)

Four settled UIKit framebuffer scenes change the colored-wave period to 60 and 240 points, in both themes. The five item sizes match the main fixture exactly. The candidate was chosen before these captures; they caused the additional dark chroma filter to be rejected. Every uncovered sampled source patch matches the supplied sRGB input byte for byte.

Before is the production renderer at `9345af9`, at the same 2× scale. After uses the current integrated core. No fitted alignment, exposure or color transform is applied. The color loader converts embedded ICC profiles to sRGB; these native framebuffers explicitly declare sRGB. Transparent residuals suppress exact or per-channel ≤2 matches.

| Region | Before | After |
| --- | ---: | ---: |
| Body | 6.623216 | 5.587513 |
| Rim | 14.882050 | 11.857279 |
| Optical band | 10.105339 | 7.801245 |

Scores exclude exterior shadows and conservative card-corner regions. All captures use iPadOS 27.0 build 24A434, not the user’s physical 27.2 device. Full metrics and capture/build provenance are linked in the viewer.
