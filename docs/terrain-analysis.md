# Terrain Analysis Contract

The terrain module is a computation-only layer over the engine's two-dimensional
Delaunay triangulation. It builds a triangulated irregular network (TIN), reports
facet slope and aspect, computes cut/fill against a horizontal pad, and routes
multiple-flow-direction (MFD) accumulation across mesh edges.

It does not acquire terrain data, transform coordinate reference systems, define a
survey datum, clip a surface to a parcel, or render a map. Those responsibilities
belong to the caller.

## Supported command line interface

Build the development-only executable:

```bash
make build/development/normal/terrain_driver
```

The input grammar is a declared count followed by exactly that many XYZ records:

```text
N
x_1 y_1 z_1
...
x_N y_N z_N
```

Horizontal coordinates and elevations must be metres. Run the versioned JSON
contract with:

```bash
./build/development/normal/terrain_driver \
  --format=json --units=metres --top-k=10 < points.xyz
```

Supported options are `--format=text|json`, `--units=metres`, and a
non-negative `--top-k=N`. Reservoir level/stage analysis is not part of the
supported base terrain capability; `--level` and `--stage` fail explicitly.

## JSON schema

Machine output uses `"schema_version":"planar-terrain-analysis/v1"`. Consumers
should require the exact schema version before interpreting fields.

A successful result has this shape:

```json
{
  "schema_version": "planar-terrain-analysis/v1",
  "status": "ok",
  "units": {
    "horizontal": "metre",
    "elevation": "metre",
    "area": "square_metre",
    "volume": "cubic_metre",
    "slope": "rise_over_run",
    "aspect": "degree_clockwise_from_north",
    "flow_accumulation": "unit_input_per_site"
  },
  "input": {"point_count": 15},
  "mesh": {"site_count": 15, "triangle_count": 16},
  "balanced_pad": {
    "elevation": 628.97,
    "cut_volume": 421.85,
    "fill_volume": 421.85
  },
  "slope": {"minimum": 0.08, "mean": 0.11, "maximum": 0.12},
  "flow": {
    "top_k": 1,
    "top": [
      {
        "rank": 1,
        "site_index": 0,
        "x": 0,
        "y": 0,
        "z": 628,
        "accumulation": 3
      }
    ]
  }
}
```

The values above only illustrate the schema. The driver writes compact JSON at
15 significant digits. Site indices refer to the engine's deterministic,
lexicographically sorted mesh sites, not the caller's input order. `top_k` is
the number of entries actually emitted and is capped by the site count.

On failure, JSON output has the same versioned envelope:

```json
{
  "schema_version": "planar-terrain-analysis/v1",
  "status": "error",
  "error": {"code": "duplicate_xy", "message": "..."}
}
```

Exit status is 0 for success, 2 for option/input/validation errors, and 3 when
validated input cannot be analysed. Stable terrain validation codes are:

- `point_elevation_count_mismatch`
- `insufficient_points`
- `non_finite_coordinate`
- `non_finite_elevation`
- `duplicate_xy`
- `collinear_xy`

The CLI also reports specific parsing and option errors, including
`missing_point_count`, `invalid_point_count`, `incomplete_point_data`,
`invalid_numeric_value`, `unexpected_input`, `invalid_top_k`,
`unsupported_units`, `unsupported_option`, and `unknown_option`.

## Numerical semantics

- The TIN is the Delaunay triangulation of distinct XY sites. Elevations remain
  associated with their exact XY coordinates after the triangulator sorts sites.
- Slope is dimensionless rise/run. Mean slope is weighted by planar facet area.
- Aspect is downhill azimuth in degrees clockwise from north. A flat facet has
  no defined aspect and the library represents it as -1.
- The balanced pad elevation is the planar-area-weighted mean of the piecewise
  linear TIN surface. At that level, exact zero-contour splitting makes cut and
  fill volumes equal up to floating-point tolerance.
- MFD routes each site's initial unit to all lower neighbours in proportion to
  downhill gradient raised to exponent 1.15. Accumulation is therefore sample
  input per site, not an area- or rainfall-weighted hydrologic quantity.

## Input limits and evidence boundary

The library can triangulate irregular XYZ samples; it is not restricted to a
regular raster lattice. That mathematical capability is narrower than a claim
that an end-to-end application ingests surveyed point clouds.

Callers remain responsible for all of the following:

- proving source provenance, units, horizontal CRS, and vertical datum;
- resolving duplicate XY observations deliberately;
- filtering, classifying, or thinning point-cloud returns;
- defining a study boundary and clipping or adding boundary constraints;
- handling breaklines, holes, concavities, or constrained triangulation;
- choosing an area-weighted hydrology model for variable-density samples.

The current Delaunay surface covers the convex hull of its input sites and does
not enforce parcel boundaries or breaklines. MFD begins with one unit per site,
so variable sample density can bias accumulation. These constraints should be
carried into any downstream claim or report.

## Verification

`make test-suite` runs the independent terrain regression suite and the JSON
driver smoke test. The suite covers an analytic tilted plane, an irregular cone,
a flat surface, validation failures, balanced earthwork, slope/aspect, and flow.
The driver smoke asserts the v1 schema, analytic values, stable error codes, and
byte-identical output for reversed input order.
