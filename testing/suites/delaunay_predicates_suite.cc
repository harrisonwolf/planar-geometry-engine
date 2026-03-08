#include "suites.h"

#include "../test_assertions.h"
#include "delaunay.h"

void run_delaunay_predicates_suite(TestRunSummary& summary){
	const std::string suite_name = "delaunay_predicates";

	expect_true(summary,
	            point_lexicographic_less(Point(0.0, 1.0), Point(1.0, 0.0)),
	            suite_name,
	            "lexicographic compare",
	            "Points should sort by x first, then y.");

	expect_true(summary,
	            orient2d(Point(0.0, 0.0), Point(3.0, 0.0), Point(1.0, 2.0)) > 0.0,
	            suite_name,
	            "orient2d ccw",
	            "Counter-clockwise triplets should have positive orientation.");

	Point center = circumcenter(Point(0.0, 0.0), Point(4.0, 0.0), Point(0.0, 4.0));
	expect_near(summary,
	            center.get_x(),
	            2.0,
	            1e-6,
	            suite_name,
	            "circumcenter x",
	            "Circumcenter x-coordinate should match the known right triangle center.");
	expect_near(summary,
	            center.get_y(),
	            2.0,
	            1e-6,
	            suite_name,
	            "circumcenter y",
	            "Circumcenter y-coordinate should match the known right triangle center.");

	expect_true(summary,
	            circumcircle_contains(Point(1.0, 1.0),
	                                  Point(0.0, 0.0),
	                                  Point(4.0, 0.0),
	                                  Point(0.0, 4.0)),
	            suite_name,
	            "circumcircle contains interior",
	            "A point strictly inside the circumcircle should be detected.");

	expect_true(summary,
	            !circumcircle_contains(Point(6.0, 6.0),
	                                   Point(0.0, 0.0),
	                                   Point(4.0, 0.0),
	                                   Point(0.0, 4.0)),
	            suite_name,
	            "circumcircle excludes exterior",
	            "A point outside the circumcircle should not be detected.");

	expect_true(summary,
	            !circumcircle_contains(Point(0.0, 0.0),
	                                   Point(0.0, 0.0),
	                                   Point(4.0, 0.0),
	                                   Point(0.0, 4.0)),
	            suite_name,
	            "circumcircle strict boundary",
	            "Triangle vertices should not count as strictly inside the circumcircle.");
}
