# Planar Geometry Quickstart

## Fast Path

From the repository root:

```bash
make release-bundle
./dist/release/run.sh
```

## Release Workflows

Inside the app you can:

1. Create a polygon
2. View stored geometry
3. Export the latest stored polygon and triangulation to the Desmos visualizer
4. Inspect build/version information

## Visualizer Export

`Export latest result to visualizer` writes:

- `tools/desmos-bridge/latest-polygon-export.json`
- `tools/desmos-bridge/latest-triangulation-export.json`
- `tools/desmos-bridge/latest-session.js`

The app also refreshes the compatibility copies:

- `tools/desmos-bridge/polygon-export.json`
- `tools/desmos-bridge/triangulation-export.json`

If the browser does not open automatically, open `tools/desmos-bridge/index.html` from inside the bundle and load the latest export files manually.

## Useful Commands

```bash
./run.sh --version
./run.sh --no-browser-launch
```
