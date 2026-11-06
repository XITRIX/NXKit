#!/bin/sh
# A compatible macOS raster Skia archive suffices; no GPU backend is started.
set -eu
glass_source=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
glass_root=$(CDPATH= cd -- "$glass_source/../.." && pwd)
glass_output=${1:?Usage: build-raster.sh output-directory}
glass_archive=${SKIA_RASTER_ARCHIVE:-"$glass_root/Submodules/skia/out/mac-arm64-angle/libskia.a"}
glass_headers=${SKIA_RASTER_HEADERS:-"$glass_root/Submodules/skia"}
glass_skia_mode=-DSK_RELEASE
if [ "${SKIA_RASTER_DEBUG:-OFF}" = ON ]; then glass_skia_mode=-DSK_DEBUG; fi
mkdir -p "$glass_output"
cd "$glass_root"
xcrun clang++ -std=c++20 -O1 "$glass_skia_mode" '-DSK_TRIVIAL_ABI=[[clang::trivial_abi]]' \
    -I"$glass_headers" -IUIKitCore/include -IUIKitCore/lib -IUIKit/include -IUIKit/lib -ISubmodules/skia -ISubmodules \
    -ISubmodules/yoga -ISubmodules/tinyxml2 "$glass_source/GlassComparisonRenderer.cpp" \
    UIKitCore/lib/BackdropEffect.cpp UIKit/lib/GlassShader.cpp UIKit/lib/UIGlassEffect.cpp \
    UIKitCore/lib/LayerCornerGeometry.cpp \
    UIKitCore/lib/UIColor.cpp UIKitCore/lib/UITraitCollection.cpp UIKitCore/lib/Geometry.cpp \
    UIKitCore/lib/NXAffineTransform.cpp UIKitCore/lib/NXTransform3D.cpp UIKitCore/lib/UIEdgeInsets.cpp \
    "$glass_archive" -framework Cocoa -framework Metal -framework CoreVideo \
    -framework OpenGL -framework IOKit -framework QuartzCore -lz -liconv -lexpat \
    -o "$glass_output/GlassComparisonRenderer"
