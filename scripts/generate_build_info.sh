#!/usr/bin/env bash
set -euo pipefail

OUTPUT_PATH="${1:-tools/desmos-bridge/build-info.js}"

safe_git() {
  git "$@" 2>/dev/null || true
}

commit_full="${GIT_COMMIT:-}"
if [[ -z "$commit_full" ]]; then
  commit_full="$(safe_git rev-parse HEAD | tr -d '\n')"
fi

commit_short=""
if [[ -n "$commit_full" ]]; then
  commit_short="${commit_full:0:7}"
fi

branch="${GIT_BRANCH:-}"
if [[ -z "$branch" ]]; then
  branch="$(safe_git rev-parse --abbrev-ref HEAD | tr -d '\n')"
fi

build_time_utc="${BUILD_TIME_UTC:-}"
if [[ -z "$build_time_utc" ]]; then
  build_time_utc="$(date -u +"%Y-%m-%dT%H:%M:%SZ")"
fi

porcelain_status="$(safe_git status --porcelain)"
dirty="false"
if [[ -n "$porcelain_status" ]]; then
  dirty="true"
fi

if [[ -z "$commit_full" ]]; then
  commit_full="unknown"
  commit_short="unknown"
fi
if [[ -z "$branch" ]]; then
  branch="unknown"
fi

mkdir -p "$(dirname "$OUTPUT_PATH")"

cat > "$OUTPUT_PATH" <<JS
window.__BUILD_INFO__ = {
  commit: {
    full: "${commit_full}",
    short: "${commit_short}"
  },
  branch: "${branch}",
  dirty: ${dirty},
  buildTimeUtc: "${build_time_utc}"
};
JS

echo "Wrote build info to ${OUTPUT_PATH}"
