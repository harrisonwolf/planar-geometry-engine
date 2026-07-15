# Testing And Verification

The project uses one aggregate TDD runner, a terrain JSON contract smoke, and two packaging smoke tests.

## Automated Checks

| Command | What it validates | Source of truth |
| --- | --- | --- |
| `make test-suite` | 14 independent geometry/terrain suites plus the terrain JSON contract, validation errors, and input-order invariance | `testing/tester.cc`, `testing/tdd_suite.cc`, `testing/suites/*.cc`, `testing/terrain_driver_smoke.sh` |
| `make release-smoke` | stable bundle contents, launcher behavior, `--version`, interactive polygon flow, and Desmos export files | `scripts/release_smoke.sh` |
| `make interview-smoke` | interview bundle contents, `--version`, deterministic sample demo, and seeded Desmos export files | `scripts/interview_smoke.sh` |

## Regression Suite Coverage

`testing/tdd_suite.cc` aggregates these 14 independent suites (the aggregate runner is not itself a suite):

- point-between predicates
- strict point-between predicates
- segment collision helpers
- strict collision helpers
- point-inside helpers
- triangle geometry
- polygon geometry
- random polygon generation
- ear-clipping triangulation
- Delaunay predicates and circumcircle behavior
- Delaunay triangulation behavior and validation
- Voronoi diagram construction
- geometry artifact export schemas
- terrain TIN, slope/aspect, earthwork, flow, and validation

Run it with:

```bash
make test-suite
```

The runner prints assertion counts and reports suite/name/message triples for failures. After it passes, `make test-suite` also runs the terrain driver smoke.

## Smoke-Test Behavior
### Terrain JSON smoke


`testing/terrain_driver_smoke.sh` sends a plane through `terrain_driver` in
forward and reversed point order. It asserts byte-identical v1 JSON, exact
analytic mesh/earthwork/slope fields, stable validation failures, and explicit
rejection of unsupported reservoir options. See
[`terrain-analysis.md`](terrain-analysis.md) for the contract and evidence
boundary.


### Stable release smoke test

`scripts/release_smoke.sh` verifies that `dist/release/` contains the packaged app, launcher, quickstart, and Desmos bridge assets. It then:

1. runs `./run.sh --version`
2. pipes a short interactive session through the stable app
3. verifies `latest-session.js`, latest export files, and compatibility copies
4. asserts that expected strings are present in the captured output

### Interview smoke test

`scripts/interview_smoke.sh` verifies that `dist/interview/` contains the packaged demo app, launcher, quickstart, sample polygon, and bridge assets. It then:

1. runs `./bin/planar-geometry-demo --version`
2. runs `./bin/planar-geometry-demo --run-sample-demo --no-browser-launch`
3. verifies seeded export files and `schemaVersion: 1`
4. asserts that the expected version/sample-demo strings are present in the captured output

## Manual Verification

### Stable release spot-check

```bash
make release-bundle
./dist/release/run.sh
```

Recommended checks:

1. create a polygon manually or generate a random polygon
2. confirm stored geometry can be viewed
3. run `Export latest result to visualizer`
4. verify `dist/release/tools/desmos-bridge/latest-polygon-export.json`
5. verify `dist/release/tools/desmos-bridge/latest-triangulation-export.json`
6. verify `./dist/release/run.sh --version`

### Interview bundle spot-check

```bash
make interview-release
cd dist/interview
./run-demo.sh
```

Recommended checks:

1. choose `1` for `Run sample demo`
2. confirm `tools/desmos-bridge/polygon-export.json` exists or updates
3. confirm `tools/desmos-bridge/triangulation-export.json` exists or updates
4. open `About / version` and verify build provenance is shown
5. if browser launch fails, use the manual viewer flow documented in [`tools-and-viewers.md`](tools-and-viewers.md)

## Debug Logging

Runtime-selectable debug logging is available in debug builds through `include/logger.h`.

Supported inputs:

- CLI: `--debug-tags=poly.gen,poly.validate`
- ENV: `GEOM_DEBUG_TAGS=poly.gen,poly.validate`
- prefix matching toggle: `--debug-match-prefix=true|false`

Examples:

```bash
./build/development/debug/main --debug-tags=poly.gen
./build/development/debug/main --debug-tags=poly --debug-match-prefix=true
GEOM_DEBUG_TAGS=poly.validate ./build/development/debug/main
```

Existing untagged `DBG(...)` output falls back to the `legacy` tag.

## Testing Support Files

| Path | Purpose |
| --- | --- |
| `testing/tester.cc` | executable entrypoint for the regression runner |
| `testing/tdd_suite.cc` | aggregates and dispatches all geometry suites |
| `testing/tdd_suite.h` | shared test summary and failure structures |
| `testing/test_assertions.cc` / `testing/test_assertions.h` | lightweight assertion helpers used by suites |
| `testing/suites/suites.h` | central declarations for all registered suites |
| `testing/suites/*.cc` | concrete suite implementations |
| `testing/suites/terrain_suite.cc` | analytic and validation coverage for the terrain library |
| `testing/terrain_driver_smoke.sh` | versioned JSON CLI and input-order smoke |
| `testing/cat_tests/*` | static input/output files for the cat-test workflow that predates the TDD harness |
