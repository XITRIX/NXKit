#!/bin/sh
set -eu
container_source=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
container_output=${1:?Usage: build-container.sh output-directory}
container_sdk=$(xcrun --sdk iphonesimulator --show-sdk-path)
container_app="$container_output/NativeContainerReference.app"
mkdir -p "$container_app"
cp "$container_source/Info.plist" "$container_app/Info.plist"
/usr/libexec/PlistBuddy -c 'Set :CFBundleIdentifier org.nxkit.NativeContainerReference' "$container_app/Info.plist"
/usr/libexec/PlistBuddy -c 'Set :CFBundleExecutable NativeContainerReference' "$container_app/Info.plist"
/usr/libexec/PlistBuddy -c 'Set :CFBundleName Native Container Reference' "$container_app/Info.plist"
xcrun --sdk iphonesimulator clang -target arm64-apple-ios26.0-simulator \
    -isysroot "$container_sdk" -fobjc-arc -framework UIKit -framework QuartzCore \
    -framework Foundation -framework CoreGraphics "$container_source/ContainerReference.m" -o "$container_app/NativeContainerReference"
codesign --force --sign - "$container_app"
