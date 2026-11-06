#!/usr/bin/env python3
"""Inspect CASDFOutputEffect copy/padding semantics in one matching shipped binary."""
import hashlib
import json
from pathlib import Path
import subprocess

binary = Path('/private/tmp/nxkit-glass-settings/binaries/QuartzCore')
scratch = Path('/private/tmp/nxkit-lens-shadow-padding')
scratch.mkdir(exist_ok=True)
command = ['xcrun', 'lldb', '-b', '-o', 'target create ' + str(binary),
           '-o', 'disassemble -s 0x1c3b365f4 -c 24',
           '-o', 'disassemble -s 0x1c3a63194 -c 8',
           '-o', 'disassemble -s 0x1c3a6350c -c 9',
           '-o', 'disassemble -s 0x1c3a80ac4 -c 3',
           '-o', 'disassemble -s 0x1c3a80b14 -c 16',
           '-o', 'disassemble -s 0x1c3a62db4 -c 12', '-o', 'quit']
result = subprocess.run(command, capture_output=True, text=True, check=True)
assembly = result.stdout
assert 'CASDFOutputEffect configureLayer:transaction:' in assembly
assert 'strb   w8, [x19, #0x98]' in assembly
assert 'CA::OGL::emit_combine' in assembly
assert 'ldp    s0, s8, [x19, #0x18]' in assembly
assert 'fneg   s9, s0' in assembly
(scratch / 'output-effect-cpu.txt').write_text(assembly)
print(json.dumps({'scope': 'Static inspection only; no native UIKit execution.',
                  'binarySHA256': hashlib.sha256(binary.read_bytes()).hexdigest(),
                  'configureEntry': '0x1c3b365f4', 'effectType': 1,
                  'applyTypeBranch': '0x1c3a63194',
                  'applyOutputBranch': '0x1c3a6350c',
                  'operation': 'emit_combine(TextureFunction1) of existing field surface',
                  'paddingBranch': '0x1c3a80ac4',
                  'paddingPair': ['-minimum', 'maximum'],
                  'roiBranch': '0x1c3a62db4',
                  'conclusion': 'output min/max contribute field support; no radial alpha gate in type1 apply',
                  'unknown': 'Complete native allocator/field-support rasterization and finite-tail error threshold'}))
