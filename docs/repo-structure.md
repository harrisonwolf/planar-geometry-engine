# Repo Structure

This file expands the top-level directory list from `README.md` and details the current nested subdirectories by root section.

## `artifacts/`

- `artifacts/`: generated artifact root.
- `artifacts/objects/`: generated object-artifact storage.

## `build/`

- `build/`: generated local build output root.
- `build/development/`: generated development builds.
- `build/development/debug/`: debug-profile development binaries and objects.
- `build/development/debug/src/`: debug object files mirroring `src/`.
- `build/development/debug/testing/`: debug test objects.
- `build/development/debug/testing/suites/`: debug suite objects.
- `build/development/normal/`: normal-profile development binaries and objects.
- `build/development/normal/src/`: normal-profile object files mirroring `src/`.
- `build/development/normal/testing/`: normal-profile test objects.
- `build/development/normal/testing/suites/`: normal-profile suite objects.
- `build/release/`: internal optimized release binaries and objects.
- `build/release/src/`: release object files mirroring `src/`.
- `build/release/testing/`: release test objects.
- `build/release/testing/suites/`: release suite objects.
- `build/test-artifacts/`: generated test artifact output.

## `dist/`

- `dist/`: packaged bundle root.
- `dist/interview/`: packaged interview/demo bundle.
- `dist/interview/bin/`: interview bundle executable.
- `dist/interview/examples/`: bundled interview sample inputs.
- `dist/interview/tools/`: bundled browser tools for the interview build.
- `dist/interview/tools/desmos-bridge/`: bundled Desmos bridge and seeded exports.
- `dist/release/`: packaged stable release bundle.
- `dist/release/bin/`: stable bundle executable.
- `dist/release/tools/`: bundled browser tools for the stable build.
- `dist/release/tools/desmos-bridge/`: bundled Desmos bridge and latest exports.

## `docs/`

- `docs/`: supporting documentation root.

Current files:

- `build-and-packaging.md`: packaging targets, bundle layouts, and provenance flow.
- `repo-structure.md`: this directory-map reference.
- `testing-and-verification.md`: regression coverage, smoke tests, and manual checks.
- `tools-and-viewers.md`: local browser tools, exports, and fallback paths.

## `examples/`

- `examples/`: checked-in sample geometry input root.

## `include/`

- `include/`: header files for geometry types, helpers, and app support.

## `packaging/`

- `packaging/`: bundle-time launchers and quickstart files.
- `packaging/interview/`: interview bundle launcher and quickstart source files.
- `packaging/release/`: stable bundle launcher and quickstart source files.

## `scripts/`

- `scripts/`: helper scripts for compilation, smoke tests, and metadata generation.

## `src/`

- `src/`: C++ implementation files and executable entrypoints.

## `testing/`

- `testing/`: regression harness, fixtures, and test utilities.
- `testing/cat_tests/`: legacy cat-test fixtures.
- `testing/suites/`: individual regression suites.

## `tools/`

- `tools/`: repo-local browser viewers and exported artifact files.
- `tools/delaunay-voronoi-viewer/`: Delaunay/Voronoi SVG viewer and latest point-set exports.
- `tools/desmos-bridge/`: polygon/triangulation Desmos bridge and latest exports.
