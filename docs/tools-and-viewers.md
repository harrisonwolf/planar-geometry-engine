# Tools And Viewers

The repo ships two local browser tools:

- `tools/desmos-bridge/` for polygon and triangulation exports
- `tools/delaunay-voronoi-viewer/` for Delaunay and Voronoi artifacts

## Desmos Bridge

`tools/desmos-bridge/index.html` is the polygon-workflow viewer used by the stable and interview bundles.

### Supported artifact types

- `polygon`
- `triangulation`

Current JSON payloads include `type` and `schemaVersion`, and the bridge rejects unsupported schema versions.

### Important files

| Path | Purpose |
| --- | --- |
| `tools/desmos-bridge/index.html` | Desmos-backed import/render UI |
| `tools/desmos-bridge/build-info.js` | generated provenance badge payload |
| `tools/desmos-bridge/latest-session.js` | autoload payload for the most recent export session |
| `tools/desmos-bridge/latest-polygon-export.json` | most recent polygon export |
| `tools/desmos-bridge/latest-triangulation-export.json` | most recent triangulation export |
| `tools/desmos-bridge/polygon-export.json` | compatibility copy of the polygon export |
| `tools/desmos-bridge/triangulation-export.json` | compatibility copy of the triangulation export |

`src/helper.cc` writes the polygon and triangulation payloads, and `src/polygon_app_support.cc` routes app flows to these paths.

## Delaunay / Voronoi Viewer

`tools/delaunay-voronoi-viewer/index.html` is the development-only viewer for point-set geometry produced by `delaunay_driver`.

### Important files

| Path | Purpose |
| --- | --- |
| `tools/delaunay-voronoi-viewer/index.html` | SVG-based viewer for Delaunay/Voronoi artifacts |
| `tools/delaunay-voronoi-viewer/latest-session.js` | autoload payload for the latest point-set export |
| `tools/delaunay-voronoi-viewer/latest-delaunay-export.json` | latest Delaunay triangulation export |
| `tools/delaunay-voronoi-viewer/latest-voronoi-export.json` | latest Voronoi export |
| `tools/delaunay-voronoi-viewer/delaunay-export.json` | compatibility copy of the Delaunay export |
| `tools/delaunay-voronoi-viewer/voronoi-export.json` | compatibility copy of the Voronoi export |

`src/delaunay_driver.cc` writes these artifacts and can launch the viewer after export.

## Browser Fallback Paths

If automatic browser launch fails, open the viewer directly from disk.

### Stable release bundle

1. open `dist/release/tools/desmos-bridge/index.html`
2. load `latest-polygon-export.json` or `polygon-export.json`
3. load `latest-triangulation-export.json` or `triangulation-export.json`

### Interview bundle

1. open `dist/interview/tools/desmos-bridge/index.html`
2. load `polygon-export.json`
3. load `triangulation-export.json`

The interview bundle is seeded during packaging, so those compatibility files already exist after `make interview-release`.

### Repo-local Delaunay/Voronoi viewer

1. run `./build/development/normal/delaunay_driver`
2. if the browser does not open, open `tools/delaunay-voronoi-viewer/index.html`
3. load `latest-delaunay-export.json` and `latest-voronoi-export.json`, or use `latest-session.js` autoload

## Related Support Files

| Path | Purpose |
| --- | --- |
| `scripts/generate_build_info.sh` | generates the bridge provenance script |
| `include/polygon_app_support.h` | defines polygon app export paths and bridge path defaults |
| `examples/interview-demo-polygon.txt` | deterministic polygon used by the interview sample demo |
