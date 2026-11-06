# Native slider lift video frames

These are native UIKit recordings of real XCTest touches on the iOS 27 Simulator,
not an animation authored to resemble UIKit. The full-frame recordings and their
SHA-256 hashes are retained in `manifest.json`; each unique active frame-marker
ID has a 1020 × 330 sRGB PNG crop. `contact-sheet.png` is a visual overview.

| Case | Unique marker IDs captured | Trace IDs missing from video | Duplicate encoded marker frames |
| --- | ---: | ---: | ---: |
| Stationary light | 131 | 20 | 50 |
| Stationary dark | 134 | 15 | 39 |
| Tap light | 84 | 14 | 31 |
| Tap dark | 72 | 12 | 18 |
| Interrupted light | 102 | 15 | 33 |
| Interrupted dark | 70 | 27 | 45 |

## What is observed

The recordings are 1320 × 2868 at 3× display scale. The slider occupies logical
`[80, 200, 300, 50]`; crops use `[60, 170, 340, 110]`. A separate 16-bit marker at
logical `[80, 320, 128, 12]` encodes `0x8000 | frameID`, least significant bit
first. The manifest records the original video timestamp, marker ID, matching
native trace timestamp, display/target timestamp, contact-relative time, value,
tracking and highlight state for every encoded active-marker frame. Repeated
marker IDs point to the first saved crop; their original timestamps remain in
the manifest. Missing marker IDs are listed rather than interpolated.

There are 608 decoded stationary-recording frames and 1054 interruption-recording
frames. The container's `r_frame_rate=600/1` is a timestamp time-base artifact;
this is variable-rate video and does **not** establish a 600 FPS presentation
rate. Use each frame's original timestamp.

## Alignment and color limits

The marker is committed after the display callback in which layers are sampled.
Matching its ID associates a visible frame with a native trace sample; it does
not prove zero-latency synchronization. Allow approximately one compositor
frame of phase uncertainty when comparing layer curves to visible images.
The exact native sampling/commit relationship remains unknown.

The source videos declare Display P3 primaries (`smpte432`), sRGB transfer
(`iec61966-2-1`), BT.709 YUV matrix and limited range. Extraction explicitly
converts P3 to sRGB with ffmpeg `zscale`, then embeds an sRGB ICC profile. Merely
decoding to `rgb24` would leave P3 RGB values incorrectly interpreted as sRGB.
Gamut clipping, video chroma subsampling and encoder compression remain; use
these crops for motion/shape comparisons, **not lossless screenshot channel-error
measurements**. The separately captured PNG screenshot references serve the
latter purpose.

## Reproduce and verify

From the repository root, using the retained recordings and gzipped native
traces under `../native/{stationary,interruption}/CASE/trace.json.gz`:

```sh
/usr/bin/python3 Demo/SliderComparison/extract-lift-video.py \
  docs/slider/lift-motion/video \
  /private/tmp/nxkit-slider-lift-video-regenerated \
  --traces docs/slider/lift-motion/native --retain-sources
```

Requires ffmpeg/ffprobe with `zscale`, Pillow and numpy. The helper reads local
files only; it does not launch a Simulator, synthesize UIKit state or perform
image registration. `manifest.json` retains source-video, source-trace and
every cropped-image SHA-256 hash. To verify retained image/source integrity:

```sh
/usr/bin/python3 - <<'PY'
import gzip, hashlib, json
from pathlib import Path
p = Path('docs/slider/lift-motion/video')
m = json.loads((p / 'manifest.json').read_text())
for f, expected in m['imageSHA256'].items():
    assert hashlib.sha256((p / f).read_bytes()).hexdigest() == expected, f
for r in m['recordings']:
    assert hashlib.sha256((p / r['source']).read_bytes()).hexdigest() == r['sourceSHA256']
    for c in r['cases']:
        raw = gzip.decompress((p / c['retainedTrace']).read_bytes())
        assert hashlib.sha256(raw).hexdigest() == c['traceSHA256'], c['id']
print('599 crops, both recordings and six native traces verified')
PY
```
