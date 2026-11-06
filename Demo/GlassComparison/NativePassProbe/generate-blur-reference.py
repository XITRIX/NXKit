"""Generate deterministic regression samples from native blur framebuffer captures."""
from pathlib import Path
import argparse
import json

parser = argparse.ArgumentParser()
parser.add_argument('--check', action='store_true')
args = parser.parse_args()
root = Path(__file__).resolve().parents[3]
evidence = root / 'docs/liquid-glass-native-settings/pass-semantics'


def write(name, lines):
    target = root / 'UIKit/tests' / name
    generated = '\n'.join(lines) + '\n'
    if args.check:
        assert target.read_text() == generated, f'{name} is stale'
    else:
        target.write_text(generated)


lines = [
    '// Generated from native framebuffer captures: pass-semantics/kernel-step/scanlines.json.',
    '#pragma once', '#include <array>',
    'struct NativeBlurReference { float radius, scale; std::array<unsigned char,384> samples; };',
    'inline const std::array<NativeBlurReference,32> nativeBlurReferences {{',
]
for name in ['kernel-step', 'kernel-step-production']:
    samples = json.loads((evidence / name / 'scanlines.json').read_text())
    for appearance, scale in [('half', .5), ('one', .125)]:
        for row in samples[appearance]:
            # Center128pt, sampled at the native3x framebuffer resolution.
            values = row['samples'][318:702]
            assert len(values) == 384
            lines.append('{' + str(float(row['radius'])) + 'f,' + str(scale) + 'f,{'
                         + ','.join(map(str, values)) + '}},')
lines.append('}};')
write('NativeGlassBlurReference.hpp', lines)

lines = [
    '// Independent native square-wave captures; not used for the step fit.',
    '#pragma once', '#include <array>',
    'struct NativeBlurStripe { float radius, scale; int period; std::array<unsigned char,384> samples; };',
    'inline const std::array<NativeBlurStripe,32> nativeBlurStripes {{',
]
samples = json.loads((evidence / 'isolated-kernel/scanlines.json').read_text())
for appearance, scale in [('half', .5), ('one', .125)]:
    for radius in [0, 1, 2, 5]:
        for period, row in zip([4, 8, 16, 32], samples[f'{appearance}-radius-{radius}'][:4]):
            values = row['samples']
            assert len(values) == 384
            lines.append('{' + str(float(radius)) + 'f,' + str(scale) + 'f,' + str(period)
                         + ',{' + ','.join(map(str, values)) + '}},')
lines.append('}};')
write('NativeGlassBlurStripeReference.hpp', lines)
