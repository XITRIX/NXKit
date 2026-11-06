#!/usr/bin/env python3
"""Compile captured native settings without fitting or synthesizing values.

The raw keyed archives are decoded as plist data, never instantiated. --check
verifies raw-capture hashes and that normalized layer/profile sources decode
from those captures before comparing the deterministic generated C++ table.
"""
import argparse
import base64
import gzip
import hashlib
import importlib.util
import json
import math
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
EVIDENCE = ROOT / 'docs/liquid-glass-native-settings'
SOURCE = EVIDENCE / 'profiles.json'
OUTPUT = ROOT / 'UIKit/lib/NativeGlassProfileData.hpp'
ALIASES = {
    'inputBlurRadius': 'blurRadius',
    'inputBlurFillBlurRadius': 'blurFillRadius',
    'inputBlurFillNormalOpacity': 'blurFillNormalOpacity',
    'inputFaceColorMatrixBlack': 'faceBlack',
    'inputFaceColorMatrixWhite': 'faceWhite',
    'inputFaceColorMatrixSaturation': 'faceSaturation',
    'inputInnerRefractionAmount': 'innerRefractionAmount',
    'inputInnerRefractionHeight': 'innerRefractionHeight',
}
HIGHLIGHT_SCALARS = (
    'curvature', 'keyAngle', 'keyHeight', 'keyHeightScale', 'keyHeightOffset',
    'keyAmount', 'keySpread', 'keySpreadScale', 'keySpreadOffset',
    'fillAngle', 'fillHeight', 'fillHeightScale', 'fillHeightOffset',
    'fillAmount', 'fillSpread', 'fillSpreadScale', 'fillSpreadOffset',
    'diffuseAmountScale', 'diffuseSpreadScale', 'diffuseHeightScale',
)


def sha(raw):
    return hashlib.sha256(raw).hexdigest()


def field(native):
    short = native.removeprefix('input')
    return ALIASES.get(native, 'sdr' + short[3:] if short.startswith('SDR')
                       else short[0].lower() + short[1:])


def scalar(value):
    assert isinstance(value, (int, float)) and not isinstance(value, bool)
    assert math.isfinite(value)
    return repr(float(value)) + 'f'


def color(value):
    if 'components' in value:
        rgba = value['components']
        assert len(rgba) == 4
    else:
        assert value['class'] == 'LKCGColorCodingProxy'
        rgba = [value[k] for k in ('r', 'g', 'b', 'a')]
    return '{' + ', '.join(scalar(v) for v in rgba) + '}'


def literal(value):
    if isinstance(value, bool):
        return 'true' if value else 'false'
    if isinstance(value, str):
        return json.dumps(value)
    if isinstance(value, dict):
        if value.get('class') == 'LKNSValueCodingProxy':
            assert value['kind'] == 1
            return '{' + scalar(value['width']) + ', ' + scalar(value['height']) + '}'
        return color(value)
    return scalar(value)


def vibrant(filters):
    assert len(filters) == 1 and filters[0]['CAFilterType'] == 'vibrantColorMatrix'
    inputs = filters[0]['CAFilterInputs']
    assert set(inputs) <= {'inputColorMatrix', 'inputClamp', 'inputClampPreserveHue', 'inputBackdropAware'}
    matrix = inputs['inputColorMatrix']
    assert matrix['kind'] == 9
    numbers = [matrix[f'm{row}{column}'] for row in range(1, 5) for column in range(1, 6)]
    return ('{.matrix = {{' + ', '.join(scalar(v) for v in numbers) + '}}, '
            '.clamp = ' + scalar(inputs['inputClamp']) + ', '
            '.clampPreserveHue = ' + literal(inputs['inputClampPreserveHue']) + ', '
            '.backdropAware = ' + literal(inputs.get('inputBackdropAware', False)) + ', '
            '.hasBackdropAware = ' + literal('inputBackdropAware' in inputs) + '}')


def layers_for(capture, verification, unpack):
    compressed = (EVIDENCE / f'captures/{capture}.json.gz').read_bytes()
    expected = verification['files'][f'captures/{capture}.json.gz']
    assert sha(compressed) == expected, f'{capture}: raw capture hash mismatch'
    raw = json.loads(gzip.decompress(compressed))

    archive_hashes = []

    def normalize(value):
        if isinstance(value, dict):
            if 'archiveBase64' in value:
                archive = base64.b64decode(value['archiveBase64'])
                if value.get('type') == 'glassBackground':
                    archive_hashes.append(sha(archive))
                return unpack(archive)
            return {k: normalize(v) for k, v in value.items()}
        if isinstance(value, list):
            return [normalize(v) for v in value]
        return value

    normalized = normalize(raw['layerTree'])
    layers_bytes = (EVIDENCE / f'captures/{capture}.layers.json').read_bytes()
    assert normalized == json.loads(layers_bytes), f'{capture}: normalized layers differ from raw archives'
    backgrounds, effects = [], []

    def walk(node):
        value = node.get('value', {})
        for f in value.get('filters') or []:
            if f.get('CAFilterType') == 'glassBackground':
                backgrounds.append((f['CAFilterInputs'], value['scale']))
        if 'effect' in value:
            effects.append(value)
        for child in node.get('sublayers', []):
            walk(child)
    walk(normalized)
    assert len(backgrounds) == 3
    output = [e['effect'] for e in effects if e['effect']['class'] == 'CASDFOutputEffect']
    highlights = [e for e in effects if e['effect']['class'] == 'CASDFKeyFillHighlightEffect']
    gradients = [e for e in effects if e['effect']['class'] == 'CASDFGradientEffect']
    fills = [e for e in effects if e['effect']['class'] == 'CASDFFillEffect']
    assert len(output) == len(highlights) == 3 and len(gradients) == len(fills) == 1
    assert highlights[0] == highlights[2], 'Accent highlighting is expected to match regular'
    assert len(archive_hashes) == 3
    return backgrounds, output, highlights, gradients[0], fills[0], sha(layers_bytes), archive_hashes


def emit_profile(case, fields, output, highlight, gradient, fill, geometry, dark, backdrop_scale, layers_hash):
    inputs = case['inputs']
    h = highlight['effect']
    assert set(h) == set(HIGHLIGHT_SCALARS) | {'class', 'keyColor', 'fillColor', 'global'}
    g = gradient['effect']
    assert len(g['colors']) == len(g['distances']) == 3 and len(g['interpolations']) == 2
    assert all(set(t) <= {'class', 'controlPoints'} for t in g['interpolations'])
    assert all('controlPoints' not in t or len(t['controlPoints']) == 4 for t in g['interpolations'])
    row = '    { // ' + case['capture'] + ', ' + case['style'] + '\n'
    row += ''.join('        .' + field(k) + ' = ' + literal(inputs[k]) + ', // ' + k + '\n' for k in fields)
    row += '        .output = {' + scalar(output['minimum']) + ', ' + scalar(output['maximum']) + '},\n'
    row += '        .highlight = {\n'
    row += ''.join('            .' + k + ' = ' + scalar(h[k]) + ',\n' for k in HIGHLIGHT_SCALARS)
    row += '            .keyColor = ' + color(h['keyColor']) + ', .fillColor = ' + color(h['fillColor']) + ',\n'
    row += '            .global = ' + literal(h['global']) + ',\n'
    row += '            .vibrant = ' + vibrant(highlight['filters']) + ',\n        },\n'
    row += '        .accent = {\n'
    row += '            .colors = {{' + ', '.join(color(c) for c in g['colors']) + '}},\n'
    row += '            .distances = {' + ', '.join(scalar(d) for d in g['distances']) + '},\n'
    row += '            .premultiplied = ' + literal(g['premultiplied']) + ',\n'
    row += '            .interpolationClasses = {' + ', '.join(literal(t['class']) for t in g['interpolations']) + '},\n'
    row += '            .interpolationControlPoints = {{' + ', '.join('{' + ', '.join(scalar(v) for v in t.get('controlPoints', [0, 0, 0, 0])) + '}' for t in g['interpolations']) + '}},\n'
    row += '            .hasInterpolationControlPoints = {' + ', '.join(literal('controlPoints' in t) for t in g['interpolations']) + '},\n'
    row += '            .fillColor = ' + color(fill['effect']['color']) + ',\n'
    row += '            .fillCompositingFilter = ' + literal(fill['compositingFilter']) + ',\n'
    row += '            .vibrant = ' + vibrant(gradient['filters']) + ',\n        },\n'
    row += ('        .context = {"iOS 27.0", "24A434", ' + ', '.join(scalar(geometry[k]) for k in ('width', 'height', 'cornerRadius', 'displayScale'))
            + ', ' + scalar(backdrop_scale) + ', ' + literal('systemBlack' if dark else 'systemWhite') + ', "systemBlue"},\n')
    row += '        .captureName = ' + literal(case['capture']) + ',\n'
    row += '        .backdropArchiveSHA256 = ' + literal(case['sourceArchiveSHA256']) + ',\n'
    row += '        .layerSourceSHA256 = ' + literal(layers_hash) + ',\n'
    return row + '    },\n'


def generate():
    source_raw = SOURCE.read_bytes()
    data = json.loads(source_raw)
    verification = json.loads((EVIDENCE / 'verification.json').read_text())
    assert sha(source_raw) == verification['files']['profiles.json']
    decoder_spec = importlib.util.spec_from_file_location(
        'native_glass_archive_data', ROOT / 'Demo/GlassComparison/SettingsProbe/decode.py')
    decoder = importlib.util.module_from_spec(decoder_spec)
    decoder_spec.loader.exec_module(decoder)
    cases = data['profiles']
    assert len(cases) == 18
    fields = sorted(cases[0]['inputs'])
    assert len(fields) == 70 and all(set(c['inputs']) == set(fields) for c in cases)
    sources = {capture: layers_for(capture, verification, decoder.unpack)
               for capture in sorted({c['capture'] for c in cases})}
    rows = []
    for dark in (False, True):
        for clear in (False, True):
            for amount in (0, .5, 1):
                selected = [p for p in cases if p['appearance'] == ('dark' if dark else 'light')
                            and p['style'] == ('clear' if clear else 'regular') and p['effectiveTint'] == amount]
                assert len(selected) == 1
                case = selected[0]
                inputs = case['inputs']
                backgrounds, outputs, highlights, gradient, fill, layers_hash, archive_hashes = sources[case['capture']]
                for index, style in enumerate(('regular', 'clear', 'tinted')):
                    match = [c for c in cases if c['capture'] == case['capture'] and c['style'] == style]
                    assert len(match) == 1 and match[0]['inputs'] == backgrounds[index][0]
                    assert match[0]['sourceArchiveSHA256'] == archive_hashes[index]
                    if style == 'tinted':
                        regular = [c for c in cases if c['capture'] == case['capture'] and c['style'] == 'regular'][0]
                        assert match[0]['inputs'] == regular['inputs'], 'Accent must remain a separate pass'
                index = 1 if clear else 0
                rows.append(emit_profile(case, fields, outputs[index], highlights[index], gradient, fill,
                    data['geometry'], dark, backgrounds[index][1], layers_hash))
    scalar_fields = [field(k) for k in fields if isinstance(cases[0]['inputs'][k], (int, float)) and not isinstance(cases[0]['inputs'][k], bool)]
    bool_fields = [field(k) for k in fields if isinstance(cases[0]['inputs'][k], bool)]
    color_fields = [field(k) for k in fields if isinstance(cases[0]['inputs'][k], dict) and cases[0]['inputs'][k]['class'] == 'LKCGColorCodingProxy']
    result = ('// Generated by Demo/GlassComparison/NativeProfiles/generate.py; do not edit.\n'
              '// Source SHA-256: ' + sha(source_raw) + '\n'
              '// Native iOS 27.0 / 24A434, 280x80pt, radius 40pt, scale 3.\n'
              '// All 70 backdrop inputs and captured applied SDF/color settings.\n'
              '#pragma once\n#include "NativeGlassProfile.hpp"\n#include <array>\n\nnamespace NXKit::Internal {\n')
    for typename, structname, members, arrayname in (
            ('NXFloat', 'NativeGlassProfile', scalar_fields, 'nativeGlassScalarFields'),
            ('bool', 'NativeGlassProfile', bool_fields, 'nativeGlassBoolFields'),
            ('NativeGlassColor', 'NativeGlassProfile', color_fields, 'nativeGlassColorFields'),
            ('NXFloat', 'NativeGlassHighlight', HIGHLIGHT_SCALARS, 'nativeGlassHighlightScalarFields')):
        result += ('inline constexpr std::array<' + typename + ' ' + structname + '::*, ' + str(len(members)) + '> ' + arrayname + ' {{\n'
                   + ''.join('    &' + structname + '::' + member + ',\n' for member in members) + '}};\n')
    result += 'inline constexpr std::array<NativeGlassProfile, 12> nativeGlassProfiles {{\n' + ''.join(rows) + '}};\n}\n'
    return result


def generate_sizes():
    source = EVIDENCE / 'pass-semantics/all-styles-profiles.json'
    raw_source = source.read_bytes()
    data = json.loads(raw_source)
    cases = data['profiles']
    assert len(cases) == 560, 'Expected 14 geometry contexts x4 styles x10 traits'
    assert data['runtime'] == 'Version 27.0 (Build 24A434)'
    tint_anchors = sorted({case['effectiveTint'] for case in cases})
    assert tint_anchors == [0, .25, .5, .75, 1]
    base_fields = sorted(json.loads(SOURCE.read_bytes())['profiles'][0]['inputs'])
    decoder_spec = importlib.util.spec_from_file_location(
        'native_glass_size_archive_data', ROOT / 'Demo/GlassComparison/SettingsProbe/decode.py')
    decoder = importlib.util.module_from_spec(decoder_spec)
    decoder_spec.loader.exec_module(decoder)
    raw_hashes = {}
    run_ids = set()
    for capture in sorted({c['capture'] for c in cases}):
        compressed = (EVIDENCE / f'pass-semantics/all-styles/{capture}.json.gz').read_bytes()
        raw_hashes[capture] = sha(compressed)
        original = json.loads(gzip.decompress(compressed))
        assert original['os'] == data['runtime'] and len(original['surfaces']) == 56
        assert original['runID'] not in run_ids
        run_ids.add(original['runID'])
        archive_hashes = []

        def normalize(value):
            if isinstance(value, dict):
                if 'archiveBase64' in value:
                    archive = base64.b64decode(value['archiveBase64'])
                    decoded = decoder.unpack(archive)
                    if decoded.get('CAFilterType') == 'glassBackground':
                        archive_hashes.append(sha(archive))
                    return decoded
                return {k: normalize(v) for k, v in value.items()}
            if isinstance(value, list):
                return [normalize(v) for v in value]
            return value
        normalized = normalize(original['layerTree'])
        material_trees = []

        def materials(node):
            if node.get('value', {}).get('class') == '_UIMultiLayer':
                material_trees.append(node)
                return
            for child in node.get('sublayers', []):
                materials(child)
        materials(normalized)
        source_cases = [c for c in cases if c['capture'] == capture]
        assert len(material_trees) == len(archive_hashes) == len(source_cases) == 56
        for case, tree, archive_hash, surface in zip(source_cases, material_trees, archive_hashes, original['surfaces']):
            assert case['style'] == surface['style']
            assert case['geometry']['width'] == surface['width'] and case['geometry']['height'] == surface['height']
            assert case['geometry']['displayScale'] == original['scale']
            assert case['geometry']['cornerRadius'] == min(surface['width'], surface['height']) / 2
            assert case['sourceCaptureSHA256'] == raw_hashes[capture]
            assert case['sourceArchiveSHA256'] == archive_hash and case['layerTree'] == tree
            assert sorted(case['inputs']) == base_fields
            assert case['effectiveTint'] == original['effectiveTint']
    groups = {}
    for case in cases:
        geometry = case['geometry']
        style = 'clear' if case['style'] in ('clear', 'clearTinted') else 'regular'
        tinted = case['style'] in ('tinted', 'clearTinted')
        groups.setdefault((case['appearance'], style, tinted, geometry['width'], geometry['height']), []).append(case)
    assert len(groups) == 112
    rows = []
    for (appearance, style, is_tinted, width, height), group in sorted(groups.items()):
        anchors = sorted(group, key=lambda c: c['effectiveTint'])
        assert [c['effectiveTint'] for c in anchors] == tint_anchors
        assert min(width, height) in (44, 52, 62, 64, 80, 120, 160)
        row = ('    {.circular = ' + literal(width == height)
               + ', .tinted = ' + literal(is_tinted) + ', .style = UIGlassEffect::Style::' + style
               + ', .appearance = UIUserInterfaceStyle::' + appearance + ', .profiles = {{\n')
        for case in anchors:
            colored_style = 'clearTinted' if style == 'clear' else 'tinted'
            tinted = [c for c in cases if c['capture'] == case['capture'] and c['style'] == colored_style
                      and c['geometry'] == case['geometry']]
            assert len(tinted) == 1
            backgrounds, effects = [], []

            def applied(node):
                value = node.get('value', {})
                for f in value.get('filters') or []:
                    if f.get('CAFilterType') == 'glassBackground':
                        backgrounds.append((f['CAFilterInputs'], value['scale']))
                if 'effect' in value:
                    effects.append(value)
                for child in node.get('sublayers', []):
                    applied(child)
            applied(case['layerTree'])
            assert len(backgrounds) == 1 and backgrounds[0][0] == case['inputs']
            outputs = [e['effect'] for e in effects if e['effect']['class'] == 'CASDFOutputEffect']
            highlights = [e for e in effects if e['effect']['class'] == 'CASDFKeyFillHighlightEffect']
            assert len(outputs) == len(highlights) == 1
            effects.clear()
            applied(tinted[0]['layerTree'])
            gradients = [e for e in effects if e['effect']['class'] == 'CASDFGradientEffect']
            fills = [e for e in effects if e['effect']['class'] == 'CASDFFillEffect']
            assert len(gradients) == len(fills) == 1
            layer_hash = sha(json.dumps(case['layerTree'], sort_keys=True, separators=(',', ':')).encode())
            row += emit_profile(case, base_fields, outputs[0], highlights[0], gradients[0], fills[0],
                                case['geometry'], appearance == 'dark', backgrounds[0][1], layer_hash)
        rows.append(row + '    }}},\n')
    return ('// Generated by Demo/GlassComparison/NativeProfiles/generate.py; do not edit.\n'
            '// Size-profile source SHA-256: ' + sha(raw_source) + '\n'
            + ''.join('// Capture ' + name + ' SHA-256: ' + digest + '\n' for name, digest in sorted(raw_hashes.items()))
            + '// All measured 14 geometry contexts; each profile retains actual source geometry.\n'
            '#pragma once\n#include "NativeGlassProfile.hpp"\n#include <array>\n\nnamespace NXKit::Internal {\n'
            'inline constexpr std::array<NXFloat, 5> nativeGlassSizeTintAnchors {{0, .25f, .5f, .75f, 1}};\n'
            'struct NativeGlassSizeProfileSet {\n'
            '    bool circular;\n    bool tinted;\n    UIGlassEffect::Style style;\n    UIUserInterfaceStyle appearance;\n'
            '    std::array<NativeGlassProfile, nativeGlassSizeTintAnchors.size()> profiles;\n};\n'
            'inline constexpr std::array<NativeGlassSizeProfileSet, 112> nativeGlassSizeProfiles {{\n'
            + ''.join(rows) + '}};\n}\n')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    outputs = {OUTPUT: generate(), ROOT / 'UIKit/lib/NativeGlassSizeProfileData.hpp': generate_sizes()}
    for output, expected in outputs.items():
        if args.check:
            if not output.exists() or output.read_text() != expected:
                raise SystemExit('Native profile table is stale: run ' + str(Path(__file__)))
        else:
            output.write_text(expected)
    if args.check:
        print('Original 18 + sized/color 560 profiles verified against raw archives; all 70 inputs and applied SDF/color tables match')


if __name__ == '__main__':
    main()
