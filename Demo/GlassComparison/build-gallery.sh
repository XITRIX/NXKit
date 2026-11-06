#!/bin/sh
set -eu
gallery_source=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
gallery_output=${1:?Usage: build-gallery.sh output-directory}
gallery_sdk=$(xcrun --sdk iphonesimulator --show-sdk-path)
gallery_app="$gallery_output/NativeGalleryReference.app"
mkdir -p "$gallery_app"
cp "$gallery_source/Info.plist" "$gallery_app/Info.plist"
/usr/libexec/PlistBuddy -c 'Set :CFBundleIdentifier org.nxkit.NativeGalleryReference' "$gallery_app/Info.plist"
/usr/libexec/PlistBuddy -c 'Set :CFBundleExecutable NativeGalleryReference' "$gallery_app/Info.plist"
/usr/libexec/PlistBuddy -c 'Set :CFBundleName Native Gallery Reference' "$gallery_app/Info.plist"
cp "$gallery_source/../resources/img/glass_demo_scene.png" "$gallery_app/scene.png"
xcrun --sdk iphonesimulator clang -target arm64-apple-ios26.0-simulator \
    -isysroot "$gallery_sdk" -fobjc-arc -framework UIKit -framework QuartzCore \
    -framework Foundation -framework CoreGraphics "$gallery_source/GalleryReference.m" -o "$gallery_app/NativeGalleryReference"
codesign --force --sign - "$gallery_app"
