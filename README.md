# Planar Geometry Engine

Planar geometry computation and demo tooling for polygons, triangulation, and visualization.

The repository now supports two parallel workflows:

- development builds for day-to-day engineering
- an interview/demo release bundle for handing someone a polished, guided build

## Current Capabilities

- Create polygons manually from vertex input
- Generate random polygons
- Triangulate polygons and inspect the resulting triangles
- Export polygon and triangulation artifacts for the bundled Desmos bridge
- Run a deterministic sample demo for interviews or smoke testing

## Development Vs Interview Release

The development workflow is still the default engineering workflow. None of the existing core build commands were repurposed.

### Use these for development

- `make development-normal`
  - Builds all development executables under `build/development/normal/`
- `make development-debug`
  - Builds all development executables under `build/development/debug/`
- `make test-suite`
  - Builds and runs the TDD-style regression suite
- `make release`
  - Builds the internal optimized binaries under `build/release/`

### Use these for interviewer/demo packaging

- `make interview-release`
  - Runs tests, builds the optimized primary app, generates build provenance, and assembles `dist/interview/`
- `make interview-smoke`
  - Rebuilds the interview bundle and validates the packaged app end to end

### What changed in the dev workflow

- `make development-normal`, `make development-debug`, `make test-suite`, and `make release` still exist and still mean the same thing.
- Development builds still include all internal executables:
  - `main`
  - `driver`
  - `rand_poly_gen_driver`
  - `tester`
- The app built from `src/main.cc` now defaults to an interview-friendly guided menu.
- For developers who still want the older exploratory menu, `main` now accepts `--classic-menu`.
- The new interview bundle is additive. It does not replace the development workflow.

### What is intentionally excluded from the interview bundle

- `driver`
- `rand_poly_gen_driver`
- `tester`
- future debug or algorithm-driver binaries

Only one primary executable is shipped in the interview bundle:

- `dist/interview/bin/planar-geometry-demo`

## Build Commands

### Day-to-day development

```bash
make development-normal
make development-debug
make test-suite
```

### Internal optimized build

```bash
make release
```

This still produces the optimized internal binaries under `build/release/`. It is not the packaged interview build.

### Interview bundle creation

```bash
make interview-release
```

### Interview bundle validation

```bash
make interview-smoke
```

## Interview Bundle Layout

After `make interview-release`, the bundle lives at `dist/interview/`.

Key files:

- `dist/interview/bin/planar-geometry-demo`
  - the packaged executable handed to the interviewer
- `dist/interview/run-demo.sh`
  - launcher that runs from the bundle root so relative asset paths resolve correctly
- `dist/interview/QUICKSTART.md`
  - short operator guide for the interview build
- `dist/interview/examples/interview-demo-polygon.txt`
  - source coordinates for the curated sample polygon
- `dist/interview/tools/desmos-bridge/index.html`
  - browser visualizer
- `dist/interview/tools/desmos-bridge/build-info.js`
  - generated provenance metadata for the visualizer badge
- `dist/interview/tools/desmos-bridge/polygon-export.json`
  - seeded with the deterministic sample demo output during packaging
- `dist/interview/tools/desmos-bridge/triangulation-export.json`
  - seeded with the deterministic sample demo output during packaging

## Running The Interview Build

### Recommended

From the repository root:

```bash
make interview-release
./dist/interview/run-demo.sh
```

### Direct executable path

From `dist/interview/`:

```bash
./bin/planar-geometry-demo
```

Running from the bundle root is important because the visualizer assets use stable relative paths inside the bundle.

## Interview App Flow

The default app menu is now guided and presentation-safe:

1. `Run sample demo`
2. `Create polygon manually`
3. `Generate sample/random polygon`
4. `View stored geometry`
5. `Export latest result to visualizer`
6. `About / version`
7. `Quit`

### Deterministic sample demo

`Run sample demo` does all of the following:

1. loads a fixed sample polygon
2. prints its polygon summary
3. prints its triangulation
4. writes polygon and triangulation JSON to the bundled Desmos bridge directory
5. attempts to open the visualizer

### About / version

The `About / version` surface reports:

- app name
- build profile
- branch
- commit
- build timestamp
- dirty/clean status at build time
- supported workflow summary

### Extra CLI flags

The primary app also supports:

```bash
./build/development/normal/main --classic-menu
./build/development/normal/main --version
./build/development/normal/main --run-sample-demo --no-browser-launch
```

Environment variable:

```bash
GEOM_SKIP_BROWSER=1 ./build/development/normal/main --run-sample-demo
```

## Release Checklist

Run these in order:

```bash
make test-suite
make interview-release
make interview-smoke
./dist/interview/run-demo.sh
```

## Manual Interview Verification

1. Run `make interview-release`
2. Change into `dist/interview/`
3. Run `./run-demo.sh`
4. Choose `1` for `Run sample demo`
5. Confirm that `tools/desmos-bridge/polygon-export.json` is produced or updated
6. Confirm that `tools/desmos-bridge/triangulation-export.json` is produced or updated
7. Confirm the browser opens, or use the fallback path below
8. Open `About / version` and verify build provenance is displayed

## Desmos Bridge

A local bridge page lives at `tools/desmos-bridge/index.html`.

The bridge currently supports two artifact types:

- `polygon`
- `triangulation`

Artifact JSON now includes a top-level `schemaVersion` field in addition to `type`. The bridge validates both fields and is structured so future artifact types can be added without replacing the page shell.

### Default release flow

The interview bundle seeds the bridge with the sample demo exports, so the default `polygon-export.json` and `triangulation-export.json` files already exist after packaging.

### Manual fallback if browser launch fails

1. Open `dist/interview/tools/desmos-bridge/index.html` in a browser
2. Click `Load polygon-export.json`
3. Click `Load triangulation-export.json`

If your system opens a file manager instead of a browser, open `index.html` directly from the browser with `File -> Open`.

## Build Provenance

Both the packaged app and the bridge now carry build provenance information.

Make injects the following metadata into compiled binaries:

- commit SHA
- branch
- build timestamp in UTC
- dirty/clean status
- build profile

The bridge metadata is generated by:

```bash
make generate-build-info
```

The `interview-release` target runs this automatically before packaging.

## Testing

### Geometry regression suite

```bash
make test-suite
```

Current suite coverage includes:

- `Point::is_between`
- `collides(...)`
- `strict_collides(...)`
- `is_inside(...)`
- triangle geometry
- polygon geometry
- random polygon generation
- ear-clipping triangulation

### Interview smoke test

```bash
make interview-smoke
```

This validates that the packaged bundle:

- contains the expected files
- launches the packaged executable
- reports version/build metadata
- runs the deterministic sample demo
- writes polygon and triangulation exports with `schemaVersion: 1`

## Debug Logging

Runtime-selectable debug logging still works in development builds.

Supported runtime inputs:

- CLI: `--debug-tags=poly.gen,poly.validate`
- ENV: `GEOM_DEBUG_TAGS=poly.gen,poly.validate`
- Optional prefix behavior: `--debug-match-prefix=true|false`

Examples:

```bash
./build/development/debug/main --debug-tags=poly.gen
GEOM_DEBUG_TAGS=poly.validate ./build/development/debug/main
```

Existing untagged `DBG(...)` logging still maps to the fallback tag `legacy`.

## Future Delaunay / Voronoi Compatibility

This change does not implement Delaunay triangulation or Voronoi diagrams.

It does prepare the release structure so those can be added additively:

- the interview bundle ships one stable primary executable and a stable visualizer location
- export artifacts are structured by top-level `type`
- the bridge resolves artifact handlers by type rather than by a single hard-coded path
- the CLI surface is organized around workflows, making it straightforward to add future items such as:
  - Delaunay triangulation demo
  - Voronoi diagram demo
  - additional export/visualization modes

## Makefile Notes

Common targets:

- `make development-normal`
- `make development-debug`
- `make release`
- `make interview-release`
- `make interview-smoke`
- `make test-suite`
- `make clean`

When adding a new executable:

1. add its name to `EXES`
2. define `EXE_<name>_SRCS`
3. add shared implementation files to `COMMON_SRCS` when appropriate

When adding future visualization-capable features, prefer extending the existing artifact/export flow instead of creating a separate release bundle layout.
