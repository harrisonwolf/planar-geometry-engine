#!/usr/bin/env bash
set -euo pipefail

BUNDLE_DIR="${1:-dist/interview}"

require_file() {
  local path="$1"
  if [[ ! -f "${path}" ]]; then
    echo "Missing required file: ${path}" >&2
    exit 1
  fi
}

require_file "${BUNDLE_DIR}/bin/planar-geometry-demo"
require_file "${BUNDLE_DIR}/run-demo.sh"
require_file "${BUNDLE_DIR}/QUICKSTART.md"
require_file "${BUNDLE_DIR}/examples/interview-demo-polygon.txt"
require_file "${BUNDLE_DIR}/tools/desmos-bridge/index.html"
require_file "${BUNDLE_DIR}/tools/desmos-bridge/build-info.js"

(
  cd "${BUNDLE_DIR}"
  ./bin/planar-geometry-demo --version >/tmp/planar-geometry-demo-version.txt
  ./bin/planar-geometry-demo --run-sample-demo --no-browser-launch >/tmp/planar-geometry-demo-sample.txt
)

require_file "${BUNDLE_DIR}/tools/desmos-bridge/latest-session.js"
require_file "${BUNDLE_DIR}/tools/desmos-bridge/latest-polygon-export.json"
require_file "${BUNDLE_DIR}/tools/desmos-bridge/latest-triangulation-export.json"
require_file "${BUNDLE_DIR}/tools/desmos-bridge/polygon-export.json"
require_file "${BUNDLE_DIR}/tools/desmos-bridge/triangulation-export.json"

grep -q 'window.__GEOM_AUTOLOAD__' "${BUNDLE_DIR}/tools/desmos-bridge/latest-session.js"
grep -q '"type": "polygon"' "${BUNDLE_DIR}/tools/desmos-bridge/latest-polygon-export.json"
grep -q '"schemaVersion": 1' "${BUNDLE_DIR}/tools/desmos-bridge/latest-polygon-export.json"
grep -q '"type": "triangulation"' "${BUNDLE_DIR}/tools/desmos-bridge/latest-triangulation-export.json"
grep -q '"schemaVersion": 1' "${BUNDLE_DIR}/tools/desmos-bridge/latest-triangulation-export.json"
grep -q '"type": "polygon"' "${BUNDLE_DIR}/tools/desmos-bridge/polygon-export.json"
grep -q '"schemaVersion": 1' "${BUNDLE_DIR}/tools/desmos-bridge/polygon-export.json"
grep -q '"type": "triangulation"' "${BUNDLE_DIR}/tools/desmos-bridge/triangulation-export.json"
grep -q '"schemaVersion": 1' "${BUNDLE_DIR}/tools/desmos-bridge/triangulation-export.json"
grep -q 'Planar Geometry Demo' /tmp/planar-geometry-demo-version.txt
grep -q 'Running sample demo' /tmp/planar-geometry-demo-sample.txt

echo "Interview smoke test passed for ${BUNDLE_DIR}"
