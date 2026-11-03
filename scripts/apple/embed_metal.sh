#!/bin/sh
# CMake escapes this Xcode placeholder in target bundle generator expressions.
# Expand only that known placeholder, without evaluating arbitrary shell text.
# A utility target's BUILT_PRODUCTS_DIR can differ from the app target's directory.
set -eu
source_directory=$1
IFS= read -r destination < "$2"
case "$destination" in
    *'${EFFECTIVE_PLATFORM_NAME}'*)
        prefix=${destination%%'${EFFECTIVE_PLATFORM_NAME}'*}
        suffix=${destination#*'${EFFECTIVE_PLATFORM_NAME}'}
        destination="$prefix${EFFECTIVE_PLATFORM_NAME-}$suffix"
        ;;
esac
mkdir -p "$destination"
/usr/bin/ditto "$source_directory" "$destination"
