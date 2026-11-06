#!/usr/bin/env python3
"""Build-specific, read-only static audit of UIKit's fluid-slider cap mapping."""
import hashlib
import json
from pathlib import Path
import struct
import subprocess

binary = Path('/private/tmp/nxkit-glass-settings/binaries/UIKitCore')
scratch = Path('/private/tmp/nxkit-slider-endpoint-static')
scratch.mkdir(exist_ok=True)
pieces = {
    'endpoint-mapper.txt': ('0x1c50420f0', '0x1c5042314'),
    'thumb-width-caller.txt': ('0x1c5048e08', '0x1c5049048'),
    'thumb-style-switch.txt': ('0x1c5046f10', '0x1c5046f54'),
    'neutral-span.txt': ('0x1c433c12c', '0x1c433c1d4'),
    'neutral-bridge.txt': ('0x1c5042b40', '0x1c5042ba4'),
    'lerp.txt': ('0x1c4121984', '0x1c412199c'),
}
hashes = {}
command = ['xcrun', 'lldb', '-b', '-o', 'target create ' + str(binary)]
for start, end in pieces.values():
    command += ['-o', 'disassemble -s ' + start + ' -e ' + end]
command += ['-o', 'quit']
output = subprocess.run(command, text=True, capture_output=True, check=True).stdout
for name, (start, end) in pieces.items():
    marker = '(lldb) disassemble -s ' + start + ' -e ' + end
    text = output.split(marker + '\n', 1)[1].split('(lldb)', 1)[0]
    (scratch / name).write_text(text)
    hashes[name] = hashlib.sha256(text.encode()).hexdigest()
mapper = (scratch / 'endpoint-mapper.txt').read_text()
assert 'movk   x8, #0x3f94, lsl #48' in mapper
assert 'movk   x8, #0x3fef, lsl #48' in mapper
assert 'objc_msgSend$barFrame' in mapper
assert 'objc_msgSend$trackBounds' in mapper
assert '_UILerp' in mapper
assert 'fadd   d12, d1, d0' in mapper
caller = (scratch / 'thumb-width-caller.txt').read_text()
assert 'objc_msgSend$thumbRectForBounds:trackRect:value:' in caller
assert 'fmov   x1, d8' in caller
assert 'mov    w3, #0x1' in caller and 'mov    w3, #0x0' in caller
style = (scratch / 'thumb-style-switch.txt').read_text()
assert 'objc_msgSend$sliderStyle' in style and 'cmp    x20, #0x1' in style
neutral = (scratch / 'neutral-span.txt').read_text()
assert 'objc_msgSend$neutralPosition' in neutral and 'fabd   d2, d0, d14' in neutral
print(json.dumps({
    'scope': 'Static UIKitCore inspection; no UIKit execution or native pixel fitting.',
    'binarySHA256': hashlib.sha256(binary.read_bytes()).hexdigest(),
    'ranges': pieces, 'disassemblySHA256': hashes,
    'thresholds': [struct.unpack('<d', struct.pack('<Q', value))[0]
                   for value in [0x3f947ae147ae147b, 0x3fef5c28f5c28f5c]],
    'capWidth': 'C(u)=T*min(u/.02,1,(1-u)/.02), for 0<=u<=1',
    'mappedEndpoint': 'E(u)=C(u)/2+u*(W-C(u))',
    'filledSpan': '[E(barFrame.minX/W),E(barFrame.maxX/W)]',
    'T': 'public thumbRectForBounds:trackRect:value: returned width; custom images included',
    'thumbless': 'sliderStyle==1 caller passes bypass=1 and T=0; raw barFrame retained',
    'neutral': 'configuration.neutralPosition is clamped to [0,1], converted through float; bar span uses abs(value-neutral)',
    'unknown': 'Public neutralValue-to-private-neutralPosition bridge for every configuration class; exhaustive legacy visual-provider coverage',
}))
