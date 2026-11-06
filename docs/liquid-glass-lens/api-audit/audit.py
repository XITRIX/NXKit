#!/usr/bin/env python3
"""Bounded read-only disassembly of the retained iOS27 UIKitCore lens API."""
import hashlib
import json
from pathlib import Path
import subprocess

binary = Path('/private/tmp/nxkit-glass-settings/binaries/UIKitCore')
output_dir = Path(__file__).resolve().parent / 'disassembly'
pieces = {
    'rest-background-border-shadow.txt': ('0x1c54c5c00', '0x1c54c6108'),
    'shadow-value-object.txt': ('0x1c415acc4', '0x1c415b014'),
    'content-mode.txt': ('0x1c54c5b1c', '0x1c54c5bac'),
    'content-container.txt': ('0x1c54c58a8', '0x1c54c5b1c'),
    'rest-background-policy.txt': ('0x1c54cad1c', '0x1c54cadd8'),
    'lift-entry.txt': ('0x1c54c798c', '0x1c54c7fb0'),
    'completion-block-bridge.txt': ('0x1c50d1fa4', '0x1c50d1fbc'),
    'completion-normal.txt': ('0x1c54ccd1c', '0x1c54ccd44'),
    'completion-cleanup.txt': ('0x1c54c99b8', '0x1c54c9b14'),
    'alongside-block-bridge.txt': ('0x1c52649c8', '0x1c52649d0'),
}
command = ['xcrun', 'lldb', '-b', '-o', 'target create ' + str(binary)]
for start, end in pieces.values():
    command += ['-o', 'disassemble -s ' + start + ' -e ' + end]
command += ['-o', 'quit']
result = subprocess.run(command, text=True, capture_output=True, check=True).stdout
hashes = {}
for name, (start, end) in pieces.items():
    marker = '(lldb) disassemble -s ' + start + ' -e ' + end + '\n'
    body = result.split(marker, 1)[1].split('(lldb)', 1)[0]
    (output_dir / name).write_text(body)
    hashes[name] = hashlib.sha256(body.encode()).hexdigest()
border = (output_dir / 'rest-background-border-shadow.txt').read_text()
assert 'setBorderWidth:' in border and 'fmov   d8, #3.00000000' in border
assert 'setShadowPathIsBounds:' in border and 'setPunchoutShadow:' in border
assert 'setShadowRadius:' in border and 'setShadowOpacity:' in border
mode = (output_dir / 'content-mode.txt').read_text()
assert 'cmp    x9, #0x1' in mode and 'setHidesSourceView:' in mode
completion = (output_dir / 'completion-block-bridge.txt').read_text()
assert 'and    w1, w0, #0x1' in completion and 'and    w2, w8, #0x1' in completion
print(json.dumps({'scope': 'Static analysis of shipped UIKitCore; not UIKit execution',
    'binarySHA256': hashlib.sha256(binary.read_bytes()).hexdigest(),
    'ranges': pieces, 'disassemblySHA256': hashes,
    'shadowFields': {'opacity': 'double; default0', 'color': 'UIColor strong; default black',
                     'offset': 'CGSize; default zero', 'radius': 'double; default0'},
    'restingBorder': 'same color on root and glass layers; width lifted?0:3',
    'restingShadow': 'restingBackground.layer; shadowPathIsBounds/punchoutShadow enabled; nil resets radius/offset/opacity0',
    'contentMode': 'mode==1 sets internal source portal hidesSourceView; setter refreshes placement nonanimated',
    'callbacks': 'alongside void(); completion void(BOOL,BOOL); normal grouped callback passes YES,NO',
    'unknowns': ['completion boolean semantic names and interruption runtime delivery',
                 'official enum spellings for style/contentMode', 'Swift-only/inherited API coverage']}))
