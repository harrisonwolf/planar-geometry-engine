#!/usr/bin/env bash
set -euo pipefail

BUNDLE_DIR="${1:-dist/release}"

require_file() {
  local path="$1"
  if [[ ! -f "${path}" ]]; then
    echo "Missing required file: ${path}" >&2
    exit 1
  fi
}

require_file "${BUNDLE_DIR}/bin/planar-geometry"
require_file "${BUNDLE_DIR}/run.sh"
require_file "${BUNDLE_DIR}/QUICKSTART.md"
require_file "${BUNDLE_DIR}/tools/desmos-bridge/index.html"
require_file "${BUNDLE_DIR}/tools/desmos-bridge/build-info.js"

(
  cd "${BUNDLE_DIR}"
  ./run.sh --version >/tmp/planar-geometry-release-version.txt
  printf '1\n2\n6\n1\n3\n1\n5\n' | ./run.sh --no-browser-launch >/tmp/planar-geometry-release-session.txt
)

require_file "${BUNDLE_DIR}/tools/desmos-bridge/latest-session.js"
require_file "${BUNDLE_DIR}/tools/desmos-bridge/latest-polygon-export.json"
require_file "${BUNDLE_DIR}/tools/desmos-bridge/latest-triangulation-export.json"
require_file "${BUNDLE_DIR}/tools/desmos-bridge/polygon-export.json"
require_file "${BUNDLE_DIR}/tools/desmos-bridge/triangulation-export.json"

grep -q 'Planar Geometry' /tmp/planar-geometry-release-version.txt
grep -q 'Stored random polygon' /tmp/planar-geometry-release-session.txt
grep -q 'Browser launch skipped.' /tmp/planar-geometry-release-session.txt
grep -q 'window.__GEOM_AUTOLOAD__' "${BUNDLE_DIR}/tools/desmos-bridge/latest-session.js"
grep -q '"type": "polygon"' "${BUNDLE_DIR}/tools/desmos-bridge/latest-polygon-export.json"
grep -q '"type": "triangulation"' "${BUNDLE_DIR}/tools/desmos-bridge/latest-triangulation-export.json"

echo "Release smoke test passed for ${BUNDLE_DIR}"
