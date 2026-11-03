#!/bin/sh
set -eu
task_root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
cd "$task_root/Submodules/skia"
replay_patch="$task_root/scripts/switch/patches/skia-builtin-effect-replay.patch"
if git apply --check "$replay_patch" 2>/dev/null; then
    git apply "$replay_patch"
elif ! git apply --reverse --check "$replay_patch"; then
    echo "Skia builtin-effect replay patch does not match this revision" >&2
    exit 1
fi
bin/gn gen out/horizon-vulkan --args='is_official_build=false target_cpu="arm64" target_os="horizon" skia_enable_graphite=true skia_enable_ganesh=false skia_use_dawn=false skia_use_gl=false skia_use_egl=false skia_use_metal=false skia_use_vulkan=true is_trivial_abi=true is_debug=false'
ninja -C out/horizon-vulkan -j "${NXKIT_BUILD_JOBS:-8}" skia skparagraph skshaper skunicode_core skunicode_icu icu skcms
