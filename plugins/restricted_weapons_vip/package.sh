#!/bin/bash
# Assembles build/package/restricted_weapons_vip: the library and manifest from build/, everything else from dist/.
set -euo pipefail
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
OUT="$ROOT/build/package/restricted_weapons_vip"
rm -rf "$OUT"
mkdir -p "$OUT/bin"
cp "$ROOT/build/plugins/restricted_weapons_vip/librestricted_weapons_vip.so" "$OUT/bin/"
cp "$ROOT/build/plugins/restricted_weapons_vip/restricted_weapons_vip.pplugin" "$OUT/"
if [ -d "$HERE/dist" ]; then cp -r "$HERE/dist/." "$OUT/"; fi
echo "package: $OUT"
