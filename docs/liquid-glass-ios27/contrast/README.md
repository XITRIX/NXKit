# Fixed-size contrast controls

The same native UIKit fixture was run on iPadOS 26.4.1 (23E254a) and iPadOS
27.0 (24A434). Each configuration fixes the card at 120×64 or 300×160 points,
with light or dark root traits, then changes only the background through
black, white, gray, checker, and black again. Labels use dynamic label colors.

On 26.4.1, small regular glass selects dark content appearance over black and
light content appearance over white in either root theme. Large regular glass
keeps the root appearance. The 520×56 bar behaves like small glass; clear
glass keeps the root appearance. Mid-gray/checker regular content follows the
root theme in these controls.

On 27.0, all three materials keep the root appearance on every tested
background and size. This validates the distinction between optical backdrop
tinting and automatic content-theme selection on the tested runtimes.

See `results.json` for native content traits, resolved label color, observed
foreground ink and body samples; `light-contrast.png` and `dark-contrast.png` compare the captures.
This fixed-size control is separate from the retained resize-history audit.
It does not establish a documented contract for future iOS versions.

Reproduction:

```sh
python3 Demo/GlassComparison/capture-contrast.py DEVICE_26_ID /tmp/prepared/NativeGlassReference.app /tmp/contrast26
python3 Demo/GlassComparison/capture-contrast.py DEVICE_27_ID /tmp/prepared/NativeGlassReference.app /tmp/contrast27
python3 Demo/GlassComparison/analyze-contrast.py /tmp/contrast26 /tmp/contrast27 /tmp/contrast-report
```
