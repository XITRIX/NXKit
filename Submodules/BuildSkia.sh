#!/bin/sh
bin/gn gen out/ios-arm64-angle --args='is_official_build=false target_cpu="arm64" skia_use_gl=true skia_use_metal=false is_trivial_abi=true target_os="ios"'
bin/gn gen out/mac-arm64-angle --args='is_official_build=false target_cpu="arm64" skia_use_gl=true skia_use_metal=false is_trivial_abi=true skia_use_angle=true'