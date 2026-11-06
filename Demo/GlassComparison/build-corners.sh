#!/bin/sh
set -eu
corner_source=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
corner_output=${1:?Usage: build-corners.sh output-directory}
corner_sdk=$(xcrun --sdk iphonesimulator --show-sdk-path)
corner_app="$corner_output/NativeCornerReference.app"
mkdir -p "$corner_app"
cp "$corner_source/Info.plist" "$corner_app/Info.plist"
/usr/libexec/PlistBuddy -c 'Set :CFBundleIdentifier org.nxkit.NativeCornerReference' "$corner_app/Info.plist"
/usr/libexec/PlistBuddy -c 'Set :CFBundleExecutable NativeCornerReference' "$corner_app/Info.plist"
/usr/libexec/PlistBuddy -c 'Set :CFBundleName Native Corner Reference' "$corner_app/Info.plist"
xcrun --sdk iphonesimulator clang -target arm64-apple-ios26.0-simulator \
    -isysroot "$corner_sdk" -fobjc-arc -framework UIKit -framework QuartzCore \
    -framework Foundation -framework CoreGraphics "$corner_source/CornerReference.m" -o "$corner_app/NativeCornerReference"
codesign --force --sign - "$corner_app"
