#!/usr/bin/env python3
"""Read-only, build-specific inspection of shipped QuartzCore and extracted AIR."""
import hashlib
import json
from pathlib import Path
import subprocess

binary = Path('/private/tmp/nxkit-glass-settings/binaries/QuartzCore')
air = Path('/private/tmp/nxkit-lens-air-audit/fixed_frag_lpf_cpf.ll')
output = Path('/private/tmp/nxkit-lens-opacity-pair')
output.mkdir(exist_ok=True)
command = ['xcrun', 'lldb', '-b', '-o', 'target create ' + str(binary),
           '-o', 'disassemble -s 0x1c398c824 -c 139', '-o', 'quit']
assembly = subprocess.run(command, text=True, capture_output=True, check=True).stdout
(output / 'opacity-pair-cpu.txt').write_text(assembly)
assert 'render_opacity_pair_filter' in assembly
assert 'mov    w9, #0x63' in assembly
assert 'bind_filter_surface' in assembly
source = air.read_text()
assert 'i8 99, label %2409' in source
start = source.index('\n2409:')
end = source.index('\n2421:', start)
case = source[start:end]
assert '%2413 = fsub fast float 1.000000e+00, %2412' in case
assert '%2415 = fmul fast float %2412, %2414' in case
assert '%2416 = fsub fast float 1.000000e+00, %2415' in case
assert '%2417 = fdiv fast float %2413, %2416' in case
(output / 'opacity-pair-air.txt').write_text(case)
print(json.dumps({'scope': 'Static inspection only; no UIKit execution.',
                  'binarySHA256': hashlib.sha256(binary.read_bytes()).hexdigest(),
                  'airSHA256': hashlib.sha256(air.read_bytes()).hexdigest(),
                  'cpuFunction': 'CA::OGL::render_opacity_pair_filter',
                  'cpuEntry': '0x1c398c824', 'fixedFragmentOpcode': 99,
                  'airCase': 2409,
                  'expression': 'premultipliedRGBA * (1-t)/(1-t*alpha)',
                  'amount': 'clamped to [0,1] by CPU; near-one path returns without emitting quads',
                  'routing': 'one composed input surface, no child-branch selection'}))
