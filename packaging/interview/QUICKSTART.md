# Interview Demo Quickstart

## Fast Path

From the repository root:

```bash
make interview-release
./dist/interview/run-demo.sh
```

Inside the app, choose `1` for `Run sample demo`.

That will:

1. Load a deterministic sample polygon
2. Print its geometry summary and triangulation
3. Export polygon and triangulation JSON into `tools/desmos-bridge/`
4. Attempt to open the Desmos bridge

## Manual Fallback

If the browser does not open automatically:

1. Open `tools/desmos-bridge/index.html` from inside the interview bundle
2. Click `Load polygon-export.json`
3. Click `Load triangulation-export.json`

## Bundle Layout

- `bin/planar-geometry-demo`: packaged executable
- `run-demo.sh`: launcher that runs from the bundle root
- `examples/interview-demo-polygon.txt`: coordinates for the curated sample polygon
- `tools/desmos-bridge/`: visualizer and exported JSON artifacts

## Useful Commands

```bash
./bin/planar-geometry-demo --version
./bin/planar-geometry-demo --run-sample-demo --no-browser-launch
```
