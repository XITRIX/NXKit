#!/usr/bin/env python3
"""Translate read-only native lens archives into portable scalar tables.

Usage: generate.py CAPTURE_DIRECTORY [--check] [--fixtures PATH]
Requires the complete 144-case matrix. Decodes keyed archives as plist data;
never instantiates native classes. The optional JSON is an exact, unrounded
fixture of every selected manual lens, including all69 glass filter inputs.
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
OUTPUT = ROOT / 'UIKit/lib/NativeLensProfileData.hpp'


def module(path, name):
    spec = importlib.util.spec_from_file_location(name, path)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


DECODER = module(ROOT / 'Demo/GlassComparison/SettingsProbe/decode.py', 'lens_archive_data')
GLASS = module(ROOT / 'Demo/GlassComparison/NativeProfiles/generate.py', 'lens_glass_generator')


def sha(raw):
    return hashlib.sha256(raw).hexdigest()


def normalize(value):
    if isinstance(value, dict):
        if 'archiveBase64' in value:
            return DECODER.unpack(base64.b64decode(value['archiveBase64']))
        return {k: normalize(v) for k, v in value.items()}
    if isinstance(value, list):
        return [normalize(v) for v in value]
    return value


def walk(node):
    yield node
    for child in node.get('sublayers', []):
        yield from walk(child)


def filters(node):
    return node.get('value', {}).get('filters') or []


def effect(node, class_name):
    matches = [n['value'] for n in walk(node)
               if n.get('value', {}).get('effect', {}).get('class') == class_name]
    assert len(matches) == 1, (class_name, len(matches))
    return matches[0]


def one_filter(tree, type_name):
    matches = [(n, f['CAFilterInputs']) for n in walk(tree) for f in filters(n)
               if f['CAFilterType'] == type_name]
    assert len(matches) == 1, (type_name, len(matches))
    return matches[0]


def displacement(node, inputs):
    layer = effect(node, 'CASDFGlassDisplacementEffect')
    elements = [n['value'] for n in walk(node)
                if n.get('value', {}).get('class') == 'CASDFElementLayer']
    assert len(elements) <= 1
    e = layer['effect']
    return {'amount': inputs['inputAmount'],
            **{k: e[k] for k in ('height', 'curvature', 'angle', 'maskOffset')},
            'effectOffset': layer['effectOffset'],
            'gradientOvalization': elements[0]['gradientOvalization'] if elements else 0,
            'hasGradientOvalization': bool(elements),
            'sourceSublayerName': inputs['inputSourceSublayerName']}


def extract(path):
    raw = path.read_bytes()
    capture = json.loads(gzip.decompress(raw))
    assert capture['osBuild'] == 'Version 27.0 (Build 24A434)'
    rows = [x for x in capture['lensSettings'] if x['identifier'] == 'manual-lens']
    assert len(rows) == 1
    lens = rows[0]
    tree = normalize(lens['layerTree'])
    spec = lens['specGetters']
    get = lens['getters']
    width, height = capture['manualBounds'][2:]
    lifted = bool(get['lifted'])
    selection = get['style'] == 1
    assert get['style'] in (0, 1)
    request = capture['request']
    assert bool(request['lifted']) == lifted and bool(request['style']) == selection
    result = {'capture': path.name, 'captureSHA256': sha(raw),
              'selection': selection, 'dark': bool(request['dark']),
              'lifted': lifted, 'size': [width, height],
              'effectiveTint': capture['effectiveTint'],
              'displayScale': capture['scale'], 'spec': spec,
              'layerSourceSHA256': sha(json.dumps(tree, sort_keys=True).encode())}
    ds = [displacement(n, f['CAFilterInputs']) for n in walk(tree)
          for f in filters(n) if f['CAFilterType'] == 'displacementMap']
    result['displacements'] = ds
    if not lifted:
        return result
    assert len(ds) == 3
    gn, glass = one_filter(tree, 'glassBackground')
    fn, foreground = one_filter(tree, 'glassForeground')
    hn = effect(tree, 'CASDFKeyFillHighlightEffect')
    vm = [f for f in hn['filters'] if f['CAFilterType'] == 'vibrantColorMatrix']
    assert len(vm) == 1
    result.update(glass=glass, foreground=foreground,
                  foregroundCaptureMargin=fn['value']['marginWidth'],
                  output=effect(gn, 'CASDFOutputEffect')['effect'],
                  foregroundOutput=effect(fn, 'CASDFOutputEffect')['effect'],
                  highlight=hn['effect'], vibrant=vm,
                  backdropBlurRadius=one_filter(tree, 'gaussianBlur')[1]['inputRadius'])
    # The archive hashes below identify the actual69-input material, without
    # silently supplying the two absent regular-glass clamp inputs.
    original = lens['layerTree']
    archives = [base64.b64decode(f['archiveBase64']) for n in walk(original)
                for f in filters(n) if f['type'] == 'glassBackground']
    assert len(archives) == 1
    result['backdropArchiveSHA256'] = sha(archives[0])
    return result


def equivalent(a, b):
    if isinstance(a, bool) or isinstance(b, bool):
        return type(a) == type(b) and a == b
    if isinstance(a, (int, float)) and isinstance(b, (int, float)):
        return math.isfinite(a) and math.isfinite(b) and abs(a-b) <= 2e-5
    if isinstance(a, dict) and isinstance(b, dict):
        return a.keys() == b.keys() and all(equivalent(a[k], b[k]) for k in a)
    if isinstance(a, list) and isinstance(b, list):
        return len(a) == len(b) and all(equivalent(x, y) for x, y in zip(a, b))
    return a == b


def resolved(base, size):
    result = json.loads(json.dumps(base))
    side = min(size)
    selection = base['selection']
    source = result['displacements'][2]
    source['amount'] = -.2 * side if selection else 0
    source['height'] = .16 * side if selection else 0
    fg = result['foreground']
    fg['inputAberrationAmount'] = side / 19 if selection else 0
    fg['inputAberrationOffset'] = side / 1.8
    fg['inputEdgeStart'] = -.2 * side
    glass = result['glass']
    glass['inputInnerRefractionAmount'] = -(.15 if selection else .3) * side
    glass['inputInnerRefractionHeight'] = min(20, (.1 if selection else .18) * side)
    glass['inputShadowAmount'] = .25 * side
    progress = min(1, max(0, (side-80)/40))
    for field in ('keySpread', 'fillSpread'):
        result['highlight'][field] = math.radians(80 + 2.5 * progress)
    return result


def settings(case):
    keys = ('displacements', 'glass', 'foreground', 'output', 'foregroundOutput',
            'highlight', 'vibrant', 'backdropBlurRadius', 'foregroundCaptureMargin', 'spec')
    return {k: case[k] for k in keys}


def validate(cases):
    assert len(cases) == 144, f'Expected144 cases, found{len(cases)}'
    actual = {(c['selection'], c['dark'], c['lifted'], tuple(c['size']), c['effectiveTint']) for c in cases}
    sizes = ((37, 24), (52, 52), (116, 56), (398, 62), (80, 80), (160, 120))
    expected = {(s, d, l, size, t) for s in (False, True) for d in (False, True)
                for l in (False, True) for size in sizes for t in (0, .5, 1)}
    assert actual == expected, 'Native lens matrix is incomplete or duplicated'
    for case in cases:
        if case['lifted'] and case['effectiveTint'] != 0:
            anchor = next(c for c in cases if c['selection'] == case['selection']
                          and c['dark'] == case['dark'] and c['lifted']
                          and c['size'] == case['size'] and c['effectiveTint'] == 0)
            assert settings(anchor) == settings(case), (case['capture'], 'native preference dependence')
    bases = []
    for selection in (False, True):
        for dark in (False, True):
            base = next(c for c in cases if c['selection'] == selection and c['dark'] == dark
                        and c['lifted'] and c['size'] == [116, 56] and c['effectiveTint'] == 0)
            for case in cases:
                if case['selection'] != selection or case['dark'] != dark:
                    continue
                assert equivalent(base['spec'], case['spec']), (case['capture'], 'spec mismatch')
                if case['lifted']:
                    target = resolved(base, case['size'])
                    assert equivalent(settings(target), settings(case)), (case['capture'], 'settings/rule mismatch')
            bases.append(base)
    return bases


def obj(values, fields=None):
    return '{' + ', '.join('.' + k + ' = ' + GLASS.literal(values[k])
                           for k in (fields or tuple(values))) + '}'


def emit_spring(spring):
    fields = ('response', 'dampingRatio', 'retargetImpulse', 'trackingResponse',
              'trackingDampingRatio', 'trackingRetargetImpulse', 'responseSmoothing',
              'dampingRatioSmoothing', 'trackingResponseSmoothing', 'trackingDampingRatioSmoothing',
              'inertialTargetSmoothingRatio', 'inertialProjectionDeceleration', 'deceleration')
    return (obj(spring, fields)[:-1] + ', .behaviorType = ' + str(spring['behaviorType'])
            + ', .smoothingAndProjectionEnabled = '
            + GLASS.literal(spring['smoothingAndProjectionEnabled']) + '}')


def emit_glass(case):
    fields = sorted(json.loads((ROOT / 'docs/liquid-glass-native-settings/profiles.json').read_text())['profiles'][0]['inputs'])
    inputs = case['glass']
    assert set(inputs) - set(fields) == {'inputBleedColorMatrixFillColor'}
    assert set(fields) - set(inputs) == {'inputClamp', 'inputClampPreserveHue'}
    values = dict(inputs, inputClamp=1., inputClampPreserveHue=False)
    result = '{\n' + ''.join('            .' + GLASS.field(k) + ' = ' + GLASS.literal(values[k]) + ',\n' for k in fields)
    result += '            .output = {' + GLASS.scalar(case['output']['minimum']) + ', ' + GLASS.scalar(case['output']['maximum']) + '},\n'
    h = case['highlight']
    result += '            .highlight = ' + obj(h, GLASS.HIGHLIGHT_SCALARS)[:-1]
    result += ', .keyColor = ' + GLASS.color(h['keyColor']) + ', .fillColor = ' + GLASS.color(h['fillColor'])
    result += ', .global = ' + GLASS.literal(h['global']) + ', .vibrant = ' + GLASS.vibrant(case['vibrant']) + '},\n'
    result += ('            .context = {"iOS 27.0", "24A434", 116.f, 56.f, 28.f, '
               + GLASS.scalar(case['displayScale']) + ', 1.f, "fixture", "systemBlue"},\n')
    for field, value in [('captureName', case['capture']), ('backdropArchiveSHA256', case['backdropArchiveSHA256']),
                         ('layerSourceSHA256', case['layerSourceSHA256'])]:
        result += '            .' + field + ' = ' + json.dumps(value) + ',\n'
    return result + '        }'


def emit_profile(case):
    result = '    { // ' + case['capture'] + '\n'
    for field, d in zip(('backdrop', 'contentWrapper', 'source'), case['displacements']):
        result += '        .' + field + ' = ' + obj(d) + ',\n'
    result += '        .backdropBlurRadius = ' + GLASS.scalar(case['backdropBlurRadius']) + ',\n'
    fg = case['foreground']
    ff = ('refractionAmount', 'refractionHeight', 'refractionOffset', 'aberrationAmount',
          'aberrationHeight', 'aberrationOffset', 'aberrationAngle', 'edgeStart', 'edgeEnd',
          'edgeOpacityStart', 'edgeOpacityEnd', 'sourceSublayerName')
    result += '        .foreground = ' + obj({f: fg['input' + f[0].upper() + f[1:]] for f in ff})[:-1]
    result += ', .captureMargin = ' + GLASS.scalar(case['foregroundCaptureMargin'])
    result += ', .output = {' + GLASS.scalar(case['foregroundOutput']['minimum']) + ', ' + GLASS.scalar(case['foregroundOutput']['maximum']) + '}},\n'
    result += '        .glass = ' + emit_glass(case) + ',\n'
    result += '        .bleedColorMatrixFillColor = ' + GLASS.color(case['glass']['inputBleedColorMatrixFillColor']) + ',\n'
    result += '        .hasGlassClamp = false, .hasGlassClampPreserveHue = false,\n'
    spec = case['spec']
    sf = ('liftedDisplacement', 'unliftedDisplacement', 'contentWrapperDisplacement',
          'unliftedBlurRadius', 'lensHangTime', 'innerShadowOffsetY', 'innerShadowOpacity', 'innerShadowRadius')
    result += '        .spec = ' + obj(spec, sf)[:-1]
    result += ', .liftSpring = ' + emit_spring(spec['liftSpring']) + ', .unliftSpring = ' + emit_spring(spec['unLiftSpring'])
    result += ', .className = ' + json.dumps(spec['_internal_class']) + '},\n'
    return result + '    },\n'


def generate(cases, bases):
    result = ('// Generated by Demo/GlassComparison/LensSettingsProbe/generate.py; do not edit.\n'
            '// Raw144-case iOS27.0/24A434 lens matrix verified, without fitting.\n'
            '// NativeGlassProfile clamp fallbacks are explicit, absent from the native lens archive.\n'
            + ''.join('// ' + c['capture'] + ' SHA256 ' + c['captureSHA256'] + '\n' for c in cases)
            + '#pragma once\n#include "NativeLensProfile.hpp"\n#include <array>\n\nnamespace NXKit::Internal {\n'
            + 'inline constexpr NXFloat nativeLensWideHighlightSpread = ' + GLASS.scalar(math.radians(82.5)) + ';\n'
            + 'inline constexpr std::array<NativeLensProfile, 4> nativeLensProfiles {{\n'
            + ''.join(emit_profile(c) for c in bases) + '}};\n')
    # These values come directly from each native archive, independently of
    # the C++ resolver's formulas, to support pointwise regression checks.
    lifted = [c for c in cases if c['lifted']]
    result += ('struct NativeLensExpectedSample {\n'
               '    bool selection, dark;\n    std::array<NXFloat, 2> size;\n    NXFloat preference;\n'
               '    NXFloat sourceAmount, sourceHeight, aberrationAmount, aberrationOffset, edgeStart;\n'
               '    NXFloat innerRefractionAmount, innerRefractionHeight, shadowAmount, highlightSpread;\n'
               '    std::string_view capture;\n};\n'
               'inline constexpr std::array<NativeLensExpectedSample, ' + str(len(lifted)) + '> nativeLensExpectedSamples {{\n')
    for c in lifted:
        values = [c['effectiveTint'], c['displacements'][2]['amount'], c['displacements'][2]['height'],
                  c['foreground']['inputAberrationAmount'], c['foreground']['inputAberrationOffset'],
                  c['foreground']['inputEdgeStart'], c['glass']['inputInnerRefractionAmount'],
                  c['glass']['inputInnerRefractionHeight'], c['glass']['inputShadowAmount'], c['highlight']['keySpread']]
        result += ('    {' + GLASS.literal(c['selection']) + ', ' + GLASS.literal(c['dark'])
                   + ', {' + ', '.join(GLASS.scalar(x) for x in c['size']) + '}, '
                   + ', '.join(GLASS.scalar(v) for v in values) + ', ' + json.dumps(c['capture']) + '},\n')
    return result + '}};\n}\n'


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('captures', type=Path)
    parser.add_argument('--check', action='store_true')
    parser.add_argument('--fixtures', type=Path)
    args = parser.parse_args()
    cases = [extract(p) for p in sorted(args.captures.glob('*.json.gz'))]
    bases = validate(cases)
    expected = generate(cases, bases)
    if args.check:
        assert OUTPUT.read_text() == expected, 'Native lens profile table is stale'
    else:
        OUTPUT.write_text(expected)
    if args.fixtures:
        args.fixtures.write_text(json.dumps({'runtime': 'iOS27.0/24A434',
            'evidence': 'Read-only CAFilter archives and explicit spec getters',
            'nativeBases': bases, 'cases': cases}, indent=2, sort_keys=True) + '\n')
    print('Verified144 native lens cases; all lifted settings match measured shortest-side rules and preferences0/.5/1')


if __name__ == '__main__':
    main()
