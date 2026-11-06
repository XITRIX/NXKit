#!/usr/bin/env python3
"""Merge verified live Metal raw states with a retained content-addressed superset.

Never copies, edits, or relabels a GPU binary. The live manifest is authoritative
for the current ABI; old raw provenance is recorded explicitly.
"""
import argparse
import csv as csv_module
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def canonical(value):
    return json.dumps(value, sort_keys=True, separators=(',', ':'), allow_nan=False)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', required=True, type=Path)
    parser.add_argument('--prior', required=True, type=Path)
    parser.add_argument('--live', required=True, type=Path)
    parser.add_argument('--profiles', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    parser.add_argument('--abi', required=True)
    args = parser.parse_args()
    sys.path.insert(0, str(args.root / 'scripts' / 'apple'))
    from bake_metal import inputs

    prior_manifest, prior_sources, prior_script = inputs(args.prior)
    live_manifest, live_sources, live_script = inputs(args.live)
    if live_manifest['abi'] != args.abi:
        raise ValueError('Fresh live harvest differs from the frozen build ABI')
    profiles = []
    seen = set()
    for scale in (1, 2, 3):
        for theme in ('light', 'dark'):
            path = args.profiles / f'cold-{scale}-{theme}.json'
            report = json.loads(path.read_text())
            identity = (report['scale'], bool(report['dark']))
            if identity != (scale, theme == 'dark') or identity in seen:
                raise ValueError(f'Incorrect or duplicate profile identity: {path}')
            seen.add(identity)
            if report['abi'] != args.abi or report['failures']:
                raise ValueError(f'Failed or stale profile: {path}')
            if len(report['passes']) != 3 or any(p['frames'] != 325 for p in report['passes']):
                raise ValueError(f'Incomplete default coverage: {path}')
            if report['passes'][-1]['libraryCompiles'] or report['passes'][-1]['pipelineCompiles']:
                raise ValueError(f'Final profile pass was not warm: {path}')
            csv = path.with_suffix('.csv')
            with csv.open(newline='') as source:
                rows = list(csv_module.DictReader(source))
            if len(rows) != 975 or any(int(row['failures']) for row in rows):
                raise ValueError(f'Incomplete or failed frame CSV: {csv}')
            profiles.append({'scale': scale, 'dark': theme == 'dark', 'frames': len(rows),
                             'summarySHA256': digest(path), 'csvSHA256': digest(csv)})

    navigation_profiles = []
    for scale, theme in ((3, 'dark'), (2, 'light')):
        csv = args.profiles / f'cold-navigation-{scale}-{theme}.csv'
        log = csv.with_suffix('.log')
        with csv.open(newline='') as source:
            rows = list(csv_module.DictReader(source))
        if len(rows) != 17497:
            raise ValueError(f'Incomplete navigation frame CSV: {csv}')
        for pass_index in range(3):
            current = [row for row in rows if int(row['pass']) == pass_index]
            if len(current) != 5832:
                raise ValueError(f'Incomplete navigation pass {pass_index}: {csv}')
            if {int(row['arrangement']) for row in current} != set(range(8)):
                raise ValueError(f'Missing navigation arrangements: {csv}')
            directions = {row['direction'] for row in current}
            if directions != {'interactive-rest', 'interactive-commit', 'interactive-cancel', 'commit-settle', 'cancel-settle', 'rest', 'push', 'pop'}:
                raise ValueError(f'Missing navigation transition directions: {csv}')
        final_rows = [row for row in rows if int(row['pass']) == 2]
        if int(final_rows[-1]['pipeline_compiles']) != int(final_rows[0]['pipeline_compiles']):
            raise ValueError(f'Navigation final pass was not warm: {csv}')
        if 'Pass 2: 0 pipeline compilations,' not in log.read_text():
            raise ValueError(f'Navigation final pass did not report warm: {log}')
        navigation_profiles.append({'scale': scale, 'dark': theme == 'dark', 'frames': len(rows),
                                    'csvSHA256': digest(csv), 'logSHA256': digest(log),
                                    'options': ['--catalog', '--full', '--interactive'] + (['--light', '--2x'] if theme == 'light' else [])})

    args.output.mkdir(parents=True, exist_ok=False)
    sources = args.output / 'sources'
    sources.mkdir()
    for path in prior_sources + live_sources:
        destination = sources / path.name
        if destination.exists() and destination.read_bytes() != path.read_bytes():
            raise ValueError(f'Content-address collision: {path.name}')
        if not destination.exists():
            shutil.copyfile(path, destination)

    aliases = {}
    pipelines = {}
    for script in (prior_script, live_script):
        for entry in script['libraries']['paths']:
            aliases[entry['label']] = {'label': entry['label'], 'path': f"libraries/{entry['label']}.metallib"}
        for entry in script['pipelines']['render_pipelines']:
            pipelines[canonical(entry)] = entry
    merged_script = {'libraries': {'paths': [aliases[key] for key in sorted(aliases)]},
                     'pipelines': {'render_pipelines': [pipelines[key] for key in sorted(pipelines)]},
                     'exportFailed': False}
    (args.output / 'pipelines.mtlp-json').write_text(json.dumps(merged_script, indent=2) + '\n')
    manifest = dict(live_manifest)
    provenance = list(live_manifest.get('additionalRawStateCatalogs', []))
    provenance += prior_manifest.get('additionalRawStateCatalogs', [])
    provenance.append({'abi': prior_manifest['abi'], 'manifestSHA256': digest(args.prior / 'manifest.json'),
                       'pipelineDescriptionSHA256': digest(args.prior / 'pipelines.mtlp-json'),
                       'reason': 'Retain exact content-addressed MSL and complete native pipeline descriptors from the prior catalog as an additive raw-state superset. All Metal libraries and native archives are freshly rebuilt for this live ABI and SDK; no old GPU binary is relabelled.'})
    unique = {}
    for entry in provenance:
        unique[(entry['abi'], entry['manifestSHA256'])] = entry
    manifest['additionalRawStateCatalogs'] = list(unique.values())
    manifest['skiaRevision'] = subprocess.check_output(
        ['git', '-C', str(args.root / 'Submodules' / 'skia'), 'rev-parse', 'HEAD'], text=True).strip()
    manifest['coverage'] = {'benchmark': 'Demo/ScrollEdgeComparison/MetalScrollEdgeBenchmark.mm',
                            'benchmarkSHA256': digest(args.root / 'Demo/ScrollEdgeComparison/MetalScrollEdgeBenchmark.mm'),
                            'productionSceneSHA256': digest(args.root / 'Demo/ScrollEdgeComparison/PortableReference.cpp'),
                            'profiles': profiles, 'frames': sum(p['frames'] for p in profiles),
                            'navigationBenchmark': 'Demo/NavigationComparison/MetalNavigationBenchmark.mm',
                            'navigationBenchmarkSHA256': digest(args.root / 'Demo/NavigationComparison/MetalNavigationBenchmark.mm'),
                            'navigationProfiles': navigation_profiles,
                            'allLiveFrames': sum(p['frames'] for p in profiles + navigation_profiles),
                            'styles': ['automatic', 'soft', 'hard', 'mixed', 'hidden'],
                            'privateBlurMasks': ['zero', 'one', 'ramp'],
                            'privateBlurRadii': [0, 1, 8, 24, 48]}
    (args.output / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
    validated_manifest, validated_sources, validated_script = inputs(args.output)
    print(json.dumps({'abi': validated_manifest['abi'], 'libraries': len(validated_sources),
                      'pipelines': len(validated_script['pipelines']['render_pipelines']),
                      'freshLibraries': len(live_sources),
                      'freshPipelines': len(live_script['pipelines']['render_pipelines']),
                      'retainedPriorABI': prior_manifest['abi'], 'liveFrames': sum(p['frames'] for p in profiles + navigation_profiles)}, indent=2))


if __name__ == '__main__':
    main()
