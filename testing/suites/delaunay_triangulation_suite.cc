#include "suites.h"

#include <algorithm>
#include <sstream>
#include <vector>

#include "../test_assertions.h"
#include "delaunay.h"

namespace {

std::string point_key(const Point& point){
	std::ostringstream out;
	out << point.get_x() << "," << point.get_y();
	return out.str();
}

std::string triangle_key(const DelaunayTriangulation& triangulation, const IndexedTriangle& triangle){
	std::vector<Point> points{
		triangulation.sites.at(triangle.a),
		triangulation.sites.at(triangle.b),
		triangulation.sites.at(triangle.c)
	};
	std::sort(points.begin(), points.end(), point_lexicographic_less);

	std::ostringstream out;
	for(const Point& point : points){
		out << "[" << point_key(point) << "]";
	}
	return out.str();
}

std::vector<std::string> canonical_triangle_keys(const DelaunayTriangulation& triangulation){
	std::vector<std::string> keys;
	keys.reserve(triangulation.triangles.size());
	for(const IndexedTriangle& triangle : triangulation.triangles){
		keys.push_back(triangle_key(triangulation, triangle));
	}
	std::sort(keys.begin(), keys.end());
	return keys;
}

bool same_edge(const Point& a1, const Point& b1, const Point& a2, const Point& b2){
	return (a1 == a2 && b1 == b2) || (a1 == b2 && b1 == a2);
}

bool triangulation_has_edge(const DelaunayTriangulation& triangulation, const Point& a, const Point& b){
	for(const IndexedTriangle& triangle : triangulation.triangles){
		const Point& ta = triangulation.sites.at(triangle.a);
		const Point& tb = triangulation.sites.at(triangle.b);
		const Point& tc = triangulation.sites.at(triangle.c);
		if(same_edge(ta, tb, a, b) || same_edge(tb, tc, a, b) || same_edge(tc, ta, a, b)){
			return true;
		}
	}
	return false;
}

} // namespace

void run_delaunay_triangulation_suite(TestRunSummary& summary){
	const std::string suite_name = "delaunay_triangulation";

	{
		std::vector<Point> points{
			Point(0.0, 0.0),
			Point(0.0, 3.0),
			Point(2.0, 1.0)
		};
		DelaunayTriangulation triangulation = bowyer_watson_triangulate(points);

		expect_true(summary,
		            triangulation.triangles.size() == 1,
		            suite_name,
		            "three points yield one triangle",
		            "Three non-collinear points should produce exactly one triangle.");
		expect_true(summary,
		            is_delaunay(triangulation),
		            suite_name,
		            "three point output is delaunay",
		            "The triangle generated from three points should satisfy the Delaunay property.");
	}

	{
		std::vector<Point> points{
			Point(0.0, 0.0),
			Point(4.0, 0.0),
			Point(3.0, 3.0),
			Point(0.0, 4.0)
		};
		DelaunayTriangulation triangulation = bowyer_watson_triangulate(points);

		expect_true(summary,
		            triangulation.triangles.size() == 2,
		            suite_name,
		            "quadrilateral triangle count",
		            "A convex quadrilateral should triangulate into exactly two triangles.");
		expect_true(summary,
		            triangulation_has_edge(triangulation, Point(0.0, 0.0), Point(3.0, 3.0)),
		            suite_name,
		            "expected delaunay diagonal",
		            "The known Delaunay diagonal should appear in the triangulation.");
		expect_true(summary,
		            !triangulation_has_edge(triangulation, Point(4.0, 0.0), Point(0.0, 4.0)),
		            suite_name,
		            "rejected non-delaunay diagonal",
		            "The non-Delaunay diagonal should not appear in the triangulation.");
	}

	{
		std::vector<Point> points_a{
			Point(0.0, 0.0),
			Point(4.0, 0.0),
			Point(3.0, 3.0),
			Point(0.0, 4.0)
		};
		std::vector<Point> points_b{
			Point(3.0, 3.0),
			Point(0.0, 4.0),
			Point(0.0, 0.0),
			Point(4.0, 0.0)
		};
		DelaunayTriangulation triangulation_a = bowyer_watson_triangulate(points_a);
		DelaunayTriangulation triangulation_b = bowyer_watson_triangulate(points_b);

		expect_true(summary,
		            canonical_triangle_keys(triangulation_a) == canonical_triangle_keys(triangulation_b),
		            suite_name,
		            "permutation invariant output",
		            "Sorting input sites should make triangulation output deterministic across input permutations.");
	}

	{
		std::vector<Point> points{
			Point(0.0, 0.0),
			Point(1.0, 0.0),
			Point(1.0, 0.0),
			Point(0.0, 2.0)
		};
		DelaunayTriangulation triangulation = bowyer_watson_triangulate(points);

		expect_true(summary,
		            triangulation.triangles.empty(),
		            suite_name,
		            "reject duplicate points",
		            "Duplicate input points should be rejected.");
	}

	{
		DelaunayTriangulation non_delaunay;
		non_delaunay.sites = {
			Point(0.0, 0.0),
			Point(4.0, 0.0),
			Point(3.0, 3.0),
			Point(0.0, 4.0)
		};
		non_delaunay.triangles = {
			IndexedTriangle{0, 1, 3},
			IndexedTriangle{1, 2, 3}
		};

		expect_true(summary,
		            !is_delaunay(non_delaunay),
		            suite_name,
		            "detect non-delaunay split",
		            "A hand-built triangulation with the wrong diagonal should fail the Delaunay check.");
	}
}
