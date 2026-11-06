#!/bin/sh
# Run prepare.py first. This app links Apple's UIKit, independently of NXKit.
set -eu
glass_source=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
glass_output=${1:?Usage: build-native.sh output-directory-containing-scene.png}
glass_sdk=$(xcrun --sdk iphonesimulator --show-sdk-path)
glass_app="$glass_output/NativeGlassReference.app"
mkdir -p "$glass_app"
cp "$glass_source/Info.plist" "$glass_app/Info.plist"
cp "$glass_output/scene.png" "$glass_app/scene.png"
xcrun --sdk iphonesimulator clang -target arm64-apple-ios26.0-simulator \
    -isysroot "$glass_sdk" -fobjc-arc -framework UIKit -framework QuartzCore \
    -framework Foundation "$glass_source/main.m" -o "$glass_app/NativeGlassReference"
codesign --force --sign - "$glass_app"
