"""Independently translated DesignLibrary CPU equations, checked against native archives.

No Apple binary/shader/disassembly is required. Source locations and image hashes
are documented in color-transfer/native-cpu-accent.json.
"""
from pathlib import Path
import colorsys
import json

ROOT = Path(__file__).resolve().parents[3] / 'docs/liquid-glass-native-settings'
HOLDOUTS = json.loads((ROOT / 'color-transfer/accent-holdouts.json').read_text())
PALETTE = json.loads((ROOT / 'pass-semantics/accent-palette.json').read_text())['samples']


def matrix(rgb, dark_regular):
    hue, lightness, saturation = colorsys.rgb_to_hls(*rgb)
    result = [0.0] * 20
    result[18] = 1.0
    if max(rgb) - min(rgb) < 0.0003452669770922512:
        denominator = (0.8 + 0.2 * lightness) * (1 - 0.2 * lightness)
        for row in range(3):
            result[row * 5 + row] = 0.25 / denominator
            result[row * 5 + 4] = (0.95 * lightness - 0.2) / denominator
        return result
    if dark_regular:
        adjusted = colorsys.hls_to_rgb(hue, min(1, 1.1 * lightness), 0.95 * saturation)
        slope = [(a - c) / 0.6 for a, c in zip(adjusted, rgb)]
        bias = [c - 0.15 * d for c, d in zip(rgb, slope)]
    else:
        # Native permits saturation above one, then clamps the generated RGB.
        adjusted = [max(0, min(1, c)) for c in
                    colorsys.hls_to_rgb(hue, 0.6 * lightness, 1.2 * saturation)]
        slope = [(c - a) / 0.85 for a, c in zip(adjusted, rgb)]
        bias = [c - d for c, d in zip(rgb, slope)]
    for row in range(3):
        for column, weight in enumerate((0.2126, 0.7152, 0.0722)):
            result[row * 5 + column] = slope[row] * weight
        result[row * 5 + 4] = bias[row]
    return result


def verify():
    samples = list(HOLDOUTS)
    for sample in PALETTE:
        native = sample['vibrant']['inputColorMatrix']
        samples.append({**sample, 'color': sample['rgb'], 'matrix': [
            native[f'm{row}{column}'] for row in range(1, 5) for column in range(1, 6)]})
    errors = []
    for sample in samples:
        actual = matrix(sample['color'], sample['appearance'] == 'dark'
                        and sample['style'] == 'tinted')
        error = max(abs(a - b) for a, b in zip(actual, sample['matrix']))
        assert error < 1e-6, (sample['name'], sample['appearance'], sample['style'], error)
        errors.append(error)
    result = {'nativeMatrices': len(samples), 'heldOutMatrices': len(HOLDOUTS),
              'coefficientComparisons': len(samples) * 20,
              'maximumCoefficientError': max(errors),
              'meanMaximumCoefficientError': sum(errors) / len(errors)}
    print(json.dumps(result, indent=2))
    return result


if __name__ == '__main__':
    verify()
