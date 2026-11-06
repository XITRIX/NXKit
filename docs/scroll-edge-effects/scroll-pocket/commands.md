# Scroll-edge Metal verification (run only after the production-source freeze)

The benchmark reuses `PortableReference.cpp`'s actual production UIKit scene.
It records CPU submission and synchronized end-to-end times separately. Default
coverage is 325 frames per pass, three passes per profile: 975 frames/profile,
5,850 frames over light/dark at 1x/2x/3x. Two full navigation profiles add
34,994 frames, for 40,844 frames in each cold/strict verification sweep. It includes all four edges, automatic /
soft / hard / mixed styles, explicit bindings, discovery obstruction, side
interactions, hidden edges, fractional offsets, overscroll, application masks,
transparent content gaps, a live adaptive-glass current-device observer, and private image-mask blur (zero / one / ramp,
radii 0/1/8/24/48).

## Build after source freeze

```sh
set -e
cd /Users/daniilvinogradov/Documents/Dev/NXKit
cmake -S Demo/GlassComparison/RasterTests -B /private/tmp/nxkit-navigation-current-skia-raster
cmake --build /private/tmp/nxkit-navigation-current-skia-raster --target MetalScrollEdgeBenchmark MetalNavigationBenchmark ViewInteractionTests -j6
ctest --test-dir /private/tmp/nxkit-navigation-current-skia-raster -R '^ViewInteractionTests$' --output-on-failure
```

The generated ABI header is
`/private/tmp/nxkit-navigation-current-skia-raster/UIKitCore/UIKitCore-metal-generated/NXMetalShaderABI.hpp`.
Read this *after* the last build. Any later UIKit production-source change
invalidates the harvest, bake and strict runtime verification below.

## Cold eight-profile live harvest

```sh
set -e
cd /Users/daniilvinogradov/Documents/Dev/NXKit
edge_batch="$(mktemp -d /private/tmp/nxkit-scroll-pocket-metal-XXXXXX)"
edge_binary=/private/tmp/nxkit-navigation-current-skia-raster/MetalScrollEdgeBenchmark
mkdir -p "$edge_batch/raw" "$edge_batch/logs"
cp -R shaders/apple/catalog "$edge_batch/prior"
for edge_scale in 1 2 3; do
    for edge_theme in light dark; do
        env NXKIT_METAL_HARVEST_DIR="$edge_batch/raw" \
            NXKIT_METAL_CACHE_DIR="$edge_batch/cold-$edge_scale-$edge_theme" \
            NXKIT_METAL_BUNDLE_DIR="$edge_batch/no-bundle" \
            NXKIT_METAL_BUNDLED_ONLY=1 NXKIT_METAL_DIAGNOSTICS=1 \
            "$edge_binary" "$edge_batch/logs/cold-$edge_scale-$edge_theme.csv" \
            --scale "$edge_scale" "--$edge_theme" \
            > "$edge_batch/logs/cold-$edge_scale-$edge_theme.log" 2>&1
    done
done
nav_binary=/private/tmp/nxkit-navigation-current-skia-raster/MetalNavigationBenchmark
for nav_profile in 3-dark 2-light; do
    set --
    if [ "$nav_profile" = 2-light ]; then set -- --light --2x; fi
    env NXKIT_METAL_HARVEST_DIR="$edge_batch/raw" \
        NXKIT_METAL_CACHE_DIR="$edge_batch/cold-navigation-$nav_profile" \
        NXKIT_METAL_BUNDLE_DIR="$edge_batch/no-bundle" \
        NXKIT_METAL_BUNDLED_ONLY=1 NXKIT_METAL_DIAGNOSTICS=1 \
        "$nav_binary" "$edge_batch/logs/cold-navigation-$nav_profile.csv" \
        --catalog --full --interactive "$@" \
        > "$edge_batch/logs/cold-navigation-$nav_profile.log" 2>&1
done
```

The process merges *matching-current-ABI raw sources and full descriptors* from
earlier profiles in the same fresh harvest directory. Do not seed it by changing
an old manifest's ABI. Each process checks its last repeated pass is warm.

## Preserve and validate the old raw-state superset

The existing repository catalog may be retained additively after recording its
manifest SHA-256 and ABI. Validate both catalogs first with
`scripts/apple/bake_metal.py::inputs()`. Merge content-addressed `.metal` files
without changing bytes, union the `libraries.paths` labels, and deduplicate full
`render_pipelines` dictionaries using canonical JSON. Use the **fresh live**
manifest as the primary manifest and append the old ABI / manifest hash / reason
under `additionalRawStateCatalogs`. Validate the merged catalog with `inputs()`
again before replacing the repository catalog. No binary file belongs in the raw
catalog, and no old `.metallib` or binary archive may be relabelled.

The prepared helper performs this validation/merge and checks all six edge profile
summaries/CSVs and both complete navigation CSVs/logs before creating output:

```sh
set -e
edge_abi="$(python3 -c 'import pathlib,re; print(re.search(r"[0-9a-f]{64}",pathlib.Path("/private/tmp/nxkit-navigation-current-skia-raster/UIKitCore/UIKitCore-metal-generated/NXMetalShaderABI.hpp").read_text()).group(0))')"
python3 docs/scroll-edge-effects/scroll-pocket/merge_catalog.py \
    --root /Users/daniilvinogradov/Documents/Dev/NXKit \
    --prior "$edge_batch/prior" --live "$edge_batch/raw" \
    --profiles "$edge_batch/logs" --output "$edge_batch/merged" \
    --abi "$edge_abi"
```

The helper asserts the fresh raw manifest matches this build-header ABI. Publishing then
copies only the generated raw catalog (`merged/.`) into `shaders/apple/catalog`;
retain the existing README. The helper has passed an exact prior-source and
full-descriptor roundtrip plus a non-warm-profile rejection check.

## Fresh offline binaries, current installed SDKs

Configure the ordinary app projects (macOS Makefiles and iOS Xcode) before baking,
so the generated build ABI/configuration dependencies precede the output timestamp.
After `shaders/apple/catalog` contains that verified merged raw catalog:

```sh
set -e
edge_abi="$(python3 -c 'import json; print(json.load(open("shaders/apple/catalog/manifest.json"))["abi"])')"
python3 scripts/apple/bake_metal.py --harvest shaders/apple/catalog \
    --output Demo/build/macos/ThorVGApp-metal-macosx --sdk macosx \
    --target air64-apple-macos27.0 --library-target air64-apple-macos12.0 \
    --gpu-family apple7 --abi "$edge_abi"
python3 scripts/apple/bake_metal.py --harvest shaders/apple/catalog \
    --output Demo/build/ios/ThorVGApp-metal-iphoneos --sdk iphoneos \
    --target air64-apple-ios27.0 --library-target air64-apple-ios15.0 \
    --gpu-family apple7 --abi "$edge_abi"
```

Installed SDKs when prepared: macOS27.0 / iOS27.1. Native archive OS-generation
target is27.0; the portable Metal library deployment targets remain macOS12 /
iOS15. This is independent of choosing iOS26 visual behavior. Cross-generation
native archive use on an actual iOS26 device is not established by a macOS run.
The normal app CMake helper also builds these resources and checks the ABI; use
the ordinary macOS/iOS app builds to verify final embedding.

## Normal app builds

```sh
set -e
cmake --build Demo/build/macos --target ThorVGApp -j6
xcodebuild -project Demo/build/ios/ThorVGApp.xcodeproj -scheme ThorVGApp \
    -configuration Debug -sdk iphoneos -destination generic/platform=iOS \
    -derivedDataPath /private/tmp/nxkit-scroll-pocket-ios-derived -jobs 4 \
    CODE_SIGNING_ALLOWED=NO
```

Wait for other builds to finish before timing the strict runs.

## Strict eight-profile archive verification

```sh
set -e
for edge_scale in 1 2 3; do
    for edge_theme in light dark; do
        env NXKIT_METAL_CACHE_DIR="$edge_batch/strict-$edge_scale-$edge_theme" \
            NXKIT_METAL_BUNDLE_DIR=Demo/build/macos/ThorVGApp-metal-macosx \
            NXKIT_METAL_BUNDLED_ONLY=1 NXKIT_METAL_REQUIRE_ARCHIVE_HIT=1 \
            NXKIT_METAL_DIAGNOSTICS=1 \
            "$edge_binary" "$edge_batch/logs/strict-$edge_scale-$edge_theme.csv" \
            --scale "$edge_scale" "--$edge_theme" \
            > "$edge_batch/logs/strict-$edge_scale-$edge_theme.log" 2>&1
    done
done
nav_binary=/private/tmp/nxkit-navigation-current-skia-raster/MetalNavigationBenchmark
for nav_profile in 3-dark 2-light; do
    set --
    if [ "$nav_profile" = 2-light ]; then set -- --light --2x; fi
    env NXKIT_METAL_CACHE_DIR="$edge_batch/strict-navigation-$nav_profile" \
        NXKIT_METAL_BUNDLE_DIR=Demo/build/macos/ThorVGApp-metal-macosx \
        NXKIT_METAL_BUNDLED_ONLY=1 NXKIT_METAL_REQUIRE_ARCHIVE_HIT=1 NXKIT_METAL_DIAGNOSTICS=1 \
        "$nav_binary" "$edge_batch/logs/strict-navigation-$nav_profile.csv" \
        --catalog --full --interactive "$@" \
        > "$edge_batch/logs/strict-navigation-$nav_profile.log" 2>&1
done
python3 - "$edge_batch" <<'PY'
import json, pathlib, sys
root=pathlib.Path(sys.argv[1])
for path in sorted((root/'logs').glob('strict-*.json')):
    report=json.loads(path.read_text())
    assert report['libraryCompiles']==report['pipelineCompiles']==report['failures']==0, path
    assert sum(p['frames'] for p in report['passes'])==975, path
    print(path.name, 'zero runtime compilations;', sum(p['frames'] for p in report['passes']), 'frames')
import csv
for path in sorted((root/'logs').glob('strict-navigation-*.csv')):
    with path.open(newline='') as source:
        rows=list(csv.DictReader(source))
    assert len(rows)==17497, path
    assert all(int(r['library_compiles'])==int(r['pipeline_compiles'])==0 for r in rows), path
    print(path.name, 'zero runtime compilations;', len(rows), 'frames')
PY
```

Both navigation runs use the actual fullscreen Demo, all eight navigation arrangements,
and interactive commit/cancel as well as ordinary push/pop. Save all eight CSVs,
six edge summaries, logs, raw manifest, catalog/source/descriptor hashes, baked
manifest/archive hashes, and app binary/resource hashes before claiming final verification.

## Alternating paired performance and strict verification

Keep the previous benchmark executable and its matching offline bundle before
rebuilding. Their hashes and relevant source snapshots are retained in
`baseline-source/manifest.json`; restore/rebuild that baseline separately when
reproducing on a new machine. The retained local binaries are not checked in.
After both app builds finish, run:

```sh
python3 Demo/ScrollEdgeComparison/verify-pocket-metal.py \
  --batch "$edge_batch" \
  --build-dir /private/tmp/nxkit-navigation-current-skia-raster \
  --before-binary /private/tmp/nxkit-scroll-pocket-before-benchmark \
  --before-bundle /private/tmp/nxkit-scroll-pocket-before-metal \
  --bundle Demo/build/macos/ThorVGApp-metal-macosx
python3 Demo/ScrollEdgeComparison/analyze-pocket-performance.py \
  "$edge_batch/logs" docs/scroll-edge-effects/scroll-pocket/performance.json
python3 docs/scroll-edge-effects/scroll-pocket/publish.py "$edge_batch"
```

The last paired after run supplies the canonical strict dark3×/light2× edge
profiles. Four additional edge profiles and both full navigation profiles finish
the eight-profile strict sweep. All paired and strict processes prohibit runtime
compilation. Before/after order alternates across three repetitions.
