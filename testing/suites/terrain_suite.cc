#include "suites.h"

#include <cmath>
#include <functional>
#include <limits>
#include <string>
#include <vector>

#include "../test_assertions.h"
#include "terrain.h"

namespace {

constexpr double kPi = 3.14159265358979323846;

Tin make_grid_tin(int nx, int ny, const std::function<double(double, double)>& height){
	std::vector<Point> points;
	std::vector<double> elevations;
	for(int ix = 0; ix < nx; ++ix){
		for(int iy = 0; iy < ny; ++iy){
			double x = static_cast<double>(ix);
			double y = static_cast<double>(iy);
			points.push_back(Point(x, y));
			elevations.push_back(height(x, y));
		}
	}
	return build_tin(points, elevations);
}

int site_index(const Tin& tin, double x, double y){
	for(size_t i = 0; i < tin.mesh.sites.size(); ++i){
		if(std::fabs(tin.mesh.sites[i].get_x() - x) < 1e-9 &&
		   std::fabs(tin.mesh.sites[i].get_y() - y) < 1e-9){
			return static_cast<int>(i);
		}
	}
	return -1;
}

double total_planar_area(const Tin& tin){
	double area = 0.0;
	for(const TriangleMetrics& metrics : all_triangle_metrics(tin)){
		area += metrics.planar_area;
	}
	return area;
}

} // namespace

void run_terrain_suite(TestRunSummary& summary){
	const std::string suite_name = "terrain";

	// Tilted plane z=2x: analytic slope, aspect, area, and earthwork.
	{
		Tin tin = make_grid_tin(4, 4, [](double x, double){ return 2.0 * x; });
		expect_true(summary, !tin.mesh.triangles.empty(), suite_name,
		            "tilted plane triangulates",
		            "A valid regular surface should build a non-empty TIN.");
		expect_near(summary, total_planar_area(tin), 9.0, 1e-6, suite_name,
		            "tilted plane footprint area",
		            "The [0,3] by [0,3] footprint has area 9.");

		SlopeStats slope = slope_stats(tin);
		expect_near(summary, slope.min_slope, 2.0, 1e-9, suite_name,
		            "tilted plane min slope", "z=2x has slope 2 everywhere.");
		expect_near(summary, slope.mean_slope, 2.0, 1e-9, suite_name,
		            "tilted plane mean slope", "Area weighting preserves a uniform slope.");
		expect_near(summary, slope.max_slope, 2.0, 1e-9, suite_name,
		            "tilted plane max slope", "z=2x has slope 2 everywhere.");

		bool surface_ratio_ok = true;
		bool aspect_ok = true;
		for(const TriangleMetrics& metrics : all_triangle_metrics(tin)){
			if(std::fabs(metrics.surface_area - metrics.planar_area * std::sqrt(5.0)) > 1e-9){
				surface_ratio_ok = false;
			}
			if(std::fabs(metrics.aspect_degrees - 270.0) > 1e-6){
				aspect_ok = false;
			}
		}
		expect_true(summary, surface_ratio_ok, suite_name,
		            "tilted plane surface area ratio",
		            "Surface area should scale by sqrt(1+slope^2).");
		expect_true(summary, aspect_ok, suite_name,
		            "tilted plane aspect faces west",
		            "A surface rising east drains west.");

		double balanced = balanced_pad_elevation(tin);
		expect_near(summary, balanced, 3.0, 1e-9, suite_name,
		            "tilted plane balanced elevation",
		            "The area-weighted mean elevation is 3.");
		CutFillResult earthwork = cut_fill(tin, balanced);
		expect_near(summary, earthwork.cut, 6.75, 1e-6, suite_name,
		            "tilted plane cut volume",
		            "The analytic cut volume at elevation 3 is 6.75.");
		expect_near(summary, earthwork.fill, 6.75, 1e-6, suite_name,
		            "tilted plane fill volume",
		            "The analytic fill volume at elevation 3 is 6.75.");

		FlowResult flow = flow_accumulation(tin);
		int interior = site_index(tin, 2.0, 2.0);
		int west_edge = site_index(tin, 0.0, 2.0);
		int east_edge = site_index(tin, 3.0, 2.0);
		expect_true(summary, interior >= 0, suite_name,
		            "tilted plane interior site", "The expected site must exist.");
		if(interior >= 0){
			expect_near(summary, flow.steepest_descent_azimuth[interior], 270.0, 1e-6,
			            suite_name, "tilted plane flow direction",
			            "Steepest descent should point due west.");
		}
		expect_true(summary,
		            west_edge >= 0 && east_edge >= 0 &&
		                flow.accumulation[west_edge] > flow.accumulation[east_edge],
		            suite_name, "tilted plane accumulates downhill",
		            "Accumulation should grow toward the downhill edge.");
	}

	// Irregular cone: proves the core is not restricted to a raster lattice.
	{
		std::vector<Point> points{Point(0.0, 0.0)};
		std::vector<double> elevations{10.0};
		const int ring_count = 8;
		for(int k = 0; k < ring_count; ++k){
			double angle = 2.0 * kPi * k / ring_count;
			points.push_back(Point(4.0 * std::cos(angle), 4.0 * std::sin(angle)));
			elevations.push_back(0.0);
		}
		Tin tin = build_tin(points, elevations);
		expect_true(summary,
		            tin.mesh.triangles.size() == static_cast<size_t>(ring_count),
		            suite_name, "irregular cone triangulates",
		            "A center and irregular ring should form one facet per ring edge.");

		SlopeStats slope = slope_stats(tin);
		expect_true(summary, slope.min_slope > 0.0, suite_name,
		            "cone has positive slope", "Every cone facet is tilted.");
		expect_near(summary, slope.max_slope, slope.min_slope, 1e-9, suite_name,
		            "cone slope is uniform", "Symmetric cone facets share a slope.");

		FlowResult flow = flow_accumulation(tin);
		int apex = site_index(tin, 0.0, 0.0);
		expect_true(summary, apex >= 0, suite_name,
		            "cone apex exists", "The apex must remain in the TIN.");
		if(apex >= 0){
			expect_near(summary, flow.accumulation[apex], 1.0, 1e-9, suite_name,
			            "cone apex sheds water",
			            "No other site drains into the highest point.");
			expect_true(summary, flow.steepest_descent_neighbor[apex] >= 0, suite_name,
			            "cone apex has descent", "The apex must have a lower neighbor.");
		}

		CutFillResult base = cut_fill(tin, 0.0);
		expect_true(summary, base.cut > 0.0, suite_name,
		            "cone cuts above base", "The cone lies above a base-level pad.");
		expect_near(summary, base.fill, 0.0, 1e-9, suite_name,
		            "cone has no base fill", "Nothing lies below the base-level pad.");
	}

	// Flat surface: zero slope/earthwork and no drainage direction.
	{
		Tin tin = make_grid_tin(3, 3, [](double, double){ return 5.0; });
		SlopeStats slope = slope_stats(tin);
		expect_near(summary, slope.min_slope, 0.0, 1e-12, suite_name,
		            "flat minimum slope", "A level surface has zero slope.");
		expect_near(summary, slope.mean_slope, 0.0, 1e-12, suite_name,
		            "flat mean slope", "A level surface has zero mean slope.");
		expect_near(summary, slope.max_slope, 0.0, 1e-12, suite_name,
		            "flat maximum slope", "A level surface has zero slope.");
		expect_near(summary, balanced_pad_elevation(tin), 5.0, 1e-12, suite_name,
		            "flat balanced elevation", "The balanced pad is at grade.");
		CutFillResult grade = cut_fill(tin, 5.0);
		expect_near(summary, grade.cut, 0.0, 1e-12, suite_name,
		            "flat zero cut", "No material is above a pad at grade.");
		expect_near(summary, grade.fill, 0.0, 1e-12, suite_name,
		            "flat zero fill", "No void is below a pad at grade.");

		FlowResult flow = flow_accumulation(tin);
		bool inert = true;
		for(size_t i = 0; i < flow.accumulation.size(); ++i){
			if(std::fabs(flow.accumulation[i] - 1.0) > 1e-12 ||
			   flow.steepest_descent_neighbor[i] != -1){
				inert = false;
			}
		}
		expect_true(summary, inert, suite_name,
		            "flat flow is inert",
		            "Each flat site retains its own unit and has no descent direction.");
	}

	// Validation returns stable, caller-visible failure classes.
	{
		using Status = TerrainInputStatus;
		expect_true(summary,
		            validate_terrain_input({Point(0, 0), Point(1, 0)}, {1.0}) ==
		                Status::point_elevation_count_mismatch,
		            suite_name, "validation count mismatch",
		            "Point/elevation count mismatch must be explicit.");
		expect_true(summary,
		            validate_terrain_input({Point(0, 0), Point(1, 0)}, {1.0, 2.0}) ==
		                Status::insufficient_points,
		            suite_name, "validation too few",
		            "Fewer than three sites cannot define a terrain footprint.");
		double nan = std::numeric_limits<double>::quiet_NaN();
		expect_true(summary,
		            validate_terrain_input({Point(nan, 0), Point(1, 0), Point(0, 1)},
		                                   {1.0, 2.0, 3.0}) == Status::non_finite_coordinate,
		            suite_name, "validation finite XY",
		            "Non-finite horizontal coordinates must be rejected.");
		expect_true(summary,
		            validate_terrain_input({Point(0, 0), Point(1, 0), Point(0, 1)},
		                                   {1.0, nan, 3.0}) == Status::non_finite_elevation,
		            suite_name, "validation finite elevation",
		            "Non-finite elevation must be rejected.");
		expect_true(summary,
		            validate_terrain_input({Point(0, 0), Point(1, 0), Point(0, 0)},
		                                   {1.0, 2.0, 3.0}) == Status::duplicate_xy,
		            suite_name, "validation duplicate XY",
		            "Duplicate coordinates need a caller-selected elevation policy.");
		expect_true(summary,
		            validate_terrain_input({Point(0, 0), Point(1, 1), Point(2, 2)},
		                                   {1.0, 2.0, 3.0}) == Status::collinear_xy,
		            suite_name, "validation collinear XY",
		            "Collinear sites do not span a terrain footprint.");
		expect_true(summary,
		            std::string(terrain_input_status_code(Status::duplicate_xy)) == "duplicate_xy",
		            suite_name, "validation code is stable",
		            "Machine callers need a stable duplicate-XY code.");

		Tin rejected = build_tin({Point(0, 0), Point(1, 0), Point(0, 0)},
		                         {1.0, 2.0, 3.0});
		expect_true(summary, rejected.mesh.triangles.empty() && rejected.z.empty(),
		            suite_name, "invalid input yields empty TIN",
		            "The library must fail closed after validation.");
	}
}
