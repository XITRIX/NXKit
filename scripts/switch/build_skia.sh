#!/bin/sh
set -eu
task_root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
exec "$task_root/Submodules/skia/tools/build-horizon.sh" "$@"
