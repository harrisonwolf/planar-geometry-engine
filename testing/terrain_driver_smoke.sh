#!/usr/bin/env bash
set -euo pipefail

driver=${1:-build/development/normal/terrain_driver}
tmp_dir=$(mktemp -d)
trap 'rm -rf "$tmp_dir"' EXIT

fail() {
  echo "terrain_driver_smoke: $*" >&2
  exit 1
}

plane_a='4
0 0 0
0 1 0
1 0 2
1 1 2
'
plane_b='4
1 1 2
1 0 2
0 1 0
0 0 0
'

printf '%s' "$plane_a" | "$driver" --format=json --units=metres --top-k=0 >"$tmp_dir/plane-a.json"
printf '%s' "$plane_b" | "$driver" --format=json --units=metres --top-k=0 >"$tmp_dir/plane-b.json"
cmp -s "$tmp_dir/plane-a.json" "$tmp_dir/plane-b.json" || fail "JSON changed with input order"
grep -Fq '"schema_version":"planar-terrain-analysis/v1","status":"ok"' "$tmp_dir/plane-a.json" || fail "missing versioned success envelope"
grep -Fq '"mesh":{"site_count":4,"triangle_count":2}' "$tmp_dir/plane-a.json" || fail "unexpected plane mesh"
grep -Fq '"balanced_pad":{"elevation":1,"cut_volume":0.25,"fill_volume":0.25}' "$tmp_dir/plane-a.json" || fail "unexpected analytic earthwork"
grep -Fq '"slope":{"minimum":2,"mean":2,"maximum":2}' "$tmp_dir/plane-a.json" || fail "unexpected analytic slope"

assert_error() {
  local expected_code=$1
  local input=$2
  shift 2
  set +e
  printf '%s' "$input" | "$driver" --format=json --units=metres "$@" >"$tmp_dir/error.json"
  local status=$?
  set -e
  [[ $status -eq 2 ]] || fail "$expected_code returned exit $status instead of 2"
  grep -Fq '"status":"error"' "$tmp_dir/error.json" || fail "$expected_code omitted error status"
  grep -Fq "\"code\":\"$expected_code\"" "$tmp_dir/error.json" || fail "expected error code $expected_code"
}

assert_error duplicate_xy '3
0 0 1
1 0 2
0 0 3
'
assert_error non_finite_coordinate '3
nan 0 1
1 0 2
0 1 3
'
assert_error collinear_xy '3
0 0 1
1 1 2
2 2 3
'
assert_error unsupported_option "$plane_a" --level=5

echo "terrain_driver JSON smoke passed"
