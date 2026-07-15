// Terrain analysis on a triangulated irregular network (TIN).
//
// This module layers terrain metrics, balanced-pad earthwork, and multiple
// flow direction (MFD) accumulation onto the engine's Delaunay mesh. It is
// computation-only; data acquisition, coordinate transforms, and rendering
// belong to callers.
#ifndef TERRAIN_H
#define TERRAIN_H

#include <vector>

#include "delaunay.h"

// Input validation is public so command-line and library callers report the
// same failure classes before triangulation. Duplicate XY coordinates are not
// assigned an arbitrary elevation: callers must resolve them deliberately.
enum class TerrainInputStatus {
	ok,
	point_elevation_count_mismatch,
	insufficient_points,
	non_finite_coordinate,
	non_finite_elevation,
	duplicate_xy,
	collinear_xy,
};

TerrainInputStatus validate_terrain_input(
	const std::vector<Point>& ground_points,
	const std::vector<double>& elevations);

// Stable machine-facing code for a validation status.
const char* terrain_input_status_code(TerrainInputStatus status);

// A TIN: a planar Delaunay mesh plus an elevation for each site. z is parallel
// to mesh.sites (same index order), so z[i] is the height of mesh.sites[i].
struct Tin {
	DelaunayTriangulation mesh;
	std::vector<double> z;
};

// Builds a TIN from raw (x, y) ground points and matching elevations.
//
// The triangulator sorts its input lexicographically, so mesh.sites order does
// not match caller order. This helper re-associates elevations by exact XY.
// Invalid inputs return an empty TIN; validate_terrain_input identifies why.
Tin build_tin(const std::vector<Point>& ground_points,
              const std::vector<double>& elevations);

// Geometry of one surface facet. slope is rise/run (dimensionless). Aspect is
// the downhill azimuth clockwise from +y (north), in degrees [0, 360). A flat
// facet has aspect -1 because its downhill direction is undefined.
struct TriangleMetrics {
	double planar_area = 0.0;
	double surface_area = 0.0;
	double slope = 0.0;
	double slope_radians = 0.0;
	double aspect_degrees = -1.0;
};

TriangleMetrics triangle_metrics(const Tin& tin, const IndexedTriangle& triangle);
std::vector<TriangleMetrics> all_triangle_metrics(const Tin& tin);

// mean_slope is weighted by planar footprint area, not facet count.
struct SlopeStats {
	double min_slope = 0.0;
	double mean_slope = 0.0;
	double max_slope = 0.0;
};

SlopeStats slope_stats(const Tin& tin);

// Earthwork relative to a horizontal pad. cut is ground removed; fill is void
// added. balanced_elevation is the level where cut equals fill.
struct CutFillResult {
	double cut = 0.0;
	double fill = 0.0;
	double balanced_elevation = 0.0;
};

// Facets crossing the pad are split exactly along their zero-height contour.
CutFillResult cut_fill(const Tin& tin, double pad_elevation);

// Planar-area-weighted mean surface elevation. Returns 0 for an empty TIN.
double balanced_pad_elevation(const Tin& tin);

// Per-site drainage values, parallel to tin.mesh.sites.
//
// accumulation starts with one unit at every site. That is appropriate for a
// roughly uniform sample lattice; it is not area-weighted rainfall for a
// variable-density survey point cloud.
struct FlowResult {
	std::vector<double> accumulation;
	std::vector<int> steepest_descent_neighbor;
	std::vector<double> steepest_descent_azimuth;
};

// Routes flow to all lower neighbors, weighted by (drop / distance)^exponent.
FlowResult flow_accumulation(const Tin& tin, double exponent = 1.15);

#endif
