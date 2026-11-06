#!/usr/bin/env python3
"""Decode retained native control CAFilter archives as plist data, without UIKit calls."""
import base64
import gzip
import hashlib
import importlib.util
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[3]
spec = importlib.util.spec_from_file_location('archive_decoder', ROOT / 'Demo/GlassComparison/SettingsProbe/decode.py')
decoder = importlib.util.module_from_spec(spec)
spec.loader.exec_module(decoder)
EVIDENCE = 'ev_00804c3e2e408ea042163c6889f41fa905db7c931b6708550354b3b1cb0865b7'


def nodes(tree, path=''):
    yield path, tree
    for index, child in enumerate(tree.get('sublayers', [])):
        yield from nodes(child, path + '.' + str(index))


def filters(tree):
    result = []
    for path, node in nodes(tree):
        value = node.get('value', {})
        for archived in value.get('filters') or []:
            raw = base64.b64decode(archived['archiveBase64'])
            decoded = decoder.unpack(raw)
            result.append({'path': path, 'layerClass': value.get('class'),
                           'scale': value.get('scale'), 'marginWidth': value.get('marginWidth'),
                           'filterType': decoded.get('CAFilterType'),
                           'inputs': decoded.get('CAFilterInputs', {}),
                           'archiveSHA256': hashlib.sha256(raw).hexdigest()})
    return result


def main():
    captures = sorted((HERE.parent / 'captures').glob('*.json.gz'))
    assert len(captures) == 144, len(captures)
    roles = [('tab', 'stage.1.0.1', 1, 1),
             ('switch', 'stage.2.0.2', 0, 1),
             ('slider', 'stage.3.0.1', 0, 0)]
    samples, identities, signatures = [], [], {}
    for capture in captures:
        raw = capture.read_bytes()
        content = json.loads(gzip.decompress(raw))
        assert content['osBuild'] == 'Version 27.0 (Build 24A434)'
        identities.append({'file': '../captures/' + capture.name,
                           'sha256': hashlib.sha256(raw).hexdigest()})
        for role, expected_path, style, warp in roles:
            matches = [lens for lens in content['lensSettings'] if lens['path'] == expected_path]
            assert len(matches) == 1, (capture.name, role)
            lens = matches[0]
            get = lens['getters']
            assert (get['lifted'], get['style'], get['warpsContentBelow']) == (0, style, warp)
            chain = filters(lens['layerTree'])
            if role == 'tab':
                blur = [f for f in chain if f['filterType'] == 'gaussianBlur']
                assert len(blur) == 1
                assert blur[0]['path'] == '.0.0'
                assert blur[0]['inputs'] == {'inputNormalizeEdges': True, 'inputRadius': 2.0}
                assert blur[0]['scale'] == .25
                assert [f['filterType'] for f in chain] == ['opacityPair', 'gaussianBlur', 'colorMatrix', 'displacementMap']
            else:
                assert [f['filterType'] for f in chain] == ['opacityPair']
            native_spec = lens['specGetters']
            scalars = {key: native_spec[key] for key in
                       ['unliftedBlurRadius', 'unliftedDisplacement', 'liftedDisplacement']}
            assert scalars == {'unliftedBlurRadius': 0 if role == 'tab' else 6,
                               'unliftedDisplacement': 0 if role == 'tab' else 50,
                               'liftedDisplacement': 9}
            row = {'role': role, 'path': expected_path, 'dark': bool(content['request']['dark']),
                   'style': style, 'lifted': False, 'warpsContentBelow': bool(warp),
                   'spec': scalars, 'filters': chain}
            # Filter hashes and serialized values should be stable across unrelated
            # manual-lens styles, sizes, lift requests and glass preferences.
            signature = json.dumps(row, sort_keys=True)
            signatures.setdefault((role, row['dark']), set()).add(signature)
            if capture.name.startswith('0.5-style0-') and '-lift0-116x56.' in capture.name:
                samples.append({'capture': '../captures/' + capture.name, **row})
    assert all(len(group) == 1 for group in signatures.values())
    result = {'inheritedRuntimeEvidenceID': EVIDENCE, 'captureCount': len(captures),
              'scope': 'Retained settled actual tab/switch/slider controls; no new UIKit execution.',
              'allControlsResting': True, 'variantsPerRoleAndTheme': 1,
              'samples': samples, 'sourceIdentities': identities}
    (HERE / 'settled-controls.json').write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps({'captures': len(captures), 'controlRows': len(captures) * 3,
                      'tabRestingGaussianRadius': 2, 'tabBackdropScale': .25,
                      'nativeSliderWarpsContentBelow': False,
                      'smallVariantRestingDisplacement': 50, 'smallVariantRestingBlur': 6}))


if __name__ == '__main__':
    main()
