#!/usr/bin/env python3
"""Build distributable Metal libraries and GPU archives from a verified harvest.

No GPU work runs in the app build: Apple metal/metallib/metal-source/metal-tt
compile the previously harvested, final Skia pipeline configurations offline.
"""
import argparse
import hashlib
import json
import os
import re
from pathlib import Path
import subprocess
import tempfile


def run(*command, cwd=None):
    subprocess.run([str(c) for c in command], cwd=cwd, check=True)


def tool(name, sdk):
    try:
        return subprocess.check_output(['xcrun', '-sdk', sdk, '--find', name], text=True).strip()
    except subprocess.CalledProcessError as error:
        raise RuntimeError('Apple Metal Toolchain is required: xcodebuild -downloadComponent MetalToolchain') from error


def inputs(directory):
    manifest = json.loads((directory / 'manifest.json').read_text())
    if len(manifest.get('abi', '')) != 64 or any(c not in '0123456789abcdef' for c in manifest['abi']):
        raise ValueError('Harvest has no valid shader ABI')
    if manifest.get('languageVersion') != 'metal2.3':
        raise ValueError('Harvest uses an unsupported Metal language version')
    sources = sorted((directory / 'sources').glob('*.metal'))
    if not sources or len(sources) > 10000:
        raise ValueError('Harvest is empty or exceeds 10,000 shader libraries')
    for source in sources:
        if source.stat().st_size > 20 * 1024 * 1024 or hashlib.sha256(source.read_bytes()).hexdigest() != source.stem:
            raise ValueError(f'Shader source hash mismatch: {source.name}')
    script = json.loads((directory / 'pipelines.mtlp-json').read_text())
    if script.get('exportFailed') or not script.get('pipelines', {}).get('render_pipelines'):
        raise ValueError('Harvest has incomplete pipeline descriptions')
    source_ids = {source.stem for source in sources}
    library_ids=set()
    for library in script.get('libraries', {}).get('paths', []):
        if library.get('label') not in source_ids:
            raise ValueError('Pipeline references a missing shader source')
        if library['label'] in library_ids:
            raise ValueError('Duplicate Metal library alias')
        library_ids.add(library['label'])
    pipelines=script['pipelines']['render_pipelines']
    if len(pipelines)>4096:
        raise ValueError('Harvest exceeds 4,096 pipeline descriptions')
    for pipeline in pipelines:
        for stage in ('vertex_function','fragment_function'):
            if stage=='fragment_function' and stage not in pipeline:
                continue  # Stencil-only passes have no fragment function.
            reference=pipeline.get(stage,'')
            match=re.fullmatch(r'alias:([0-9a-f]{64})#([A-Za-z_][A-Za-z_0-9]*)',reference)
            if not match or match[1] not in library_ids:
                raise ValueError('Invalid or missing Metal pipeline function alias')
    return manifest, sources, script


def bake(harvest, output, sdk, target, family, abi=None, library_target=None):
    manifest, sources, script = inputs(harvest)
    if abi and manifest['abi'] != abi:
        raise ValueError('Metal shader catalog ABI differs from this build; regenerate the catalog with MetalNavigationBenchmark')
    tools = {name: tool(name, sdk) for name in ('metal', 'metallib', 'metal-tt')}
    output.mkdir(parents=True, exist_ok=True)
    libraries = output / 'libraries'
    libraries.mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='nxkit-metal-', dir=output.parent) as temporary:
        work = Path(temporary)
        for source in sources:
            air = work / (source.stem + '.air')
            destination = libraries / (source.stem + '.metallib')
            standard='macos-metal2.3' if sdk=='macosx' else 'ios-metal2.3'
            run(tools['metal'], '-target', library_target or target, '-std='+standard, '-ffast-math', '-c', source, '-o', air)
            run(tools['metallib'], air, '-o', destination)
        script.pop('exportFailed', None)
        for library in script.get('libraries', {}).get('paths', []):
            library['path'] = str((libraries / (library['label'] + '.metallib')).resolve())
        configuration = work / 'pipelines.mtlp-json'
        configuration.write_text(json.dumps(script, indent=2) + '\n')
        archive = work / 'pipeline.binary.metallib'
        run(tools['metal-tt'], '-target', target, '-gpu-family', family, configuration, '-o', archive)
        os.replace(archive, output / archive.name)
    manifest |= {'sdk': sdk, 'target': target, 'gpuFamily': family, 'libraries': len(sources),
                 'libraryTarget':library_target or target,
                 'pipelineDescriptionSHA256': hashlib.sha256((harvest / 'pipelines.mtlp-json').read_bytes()).hexdigest()}
    # Runtime preparation uses aliases only; no build-machine paths enter the bundle.
    for library in script['libraries']['paths']:
        library['path']='libraries/'+library['label']+'.metallib'
    (output/'pipelines.json').write_text(json.dumps(script,indent=2)+'\n')
    temporary = output / 'manifest.json.tmp'
    temporary.write_text(json.dumps(manifest, indent=2) + '\n')
    os.replace(temporary, output / 'manifest.json')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--harvest', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    parser.add_argument('--sdk', choices=['macosx', 'iphoneos', 'iphonesimulator'], required=True)
    parser.add_argument('--target', required=True, help='AIR target, e.g. air64-apple-ios16.0')
    parser.add_argument('--gpu-family', default='metal3')
    parser.add_argument('--abi', help='Require the shader ABI generated by this CMake build')
    parser.add_argument('--library-target', help='Minimum OS AIR target for the portable shader libraries')
    args = parser.parse_args()
    bake(args.harvest.resolve(), args.output.resolve(), args.sdk, args.target, args.gpu_family,args.abi,args.library_target)


if __name__ == '__main__':
    main()
