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

std::string triangle_key_from_points(std::vector<Point> points){
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

	{
		const Point p0(-25.09, 90.14);
		const Point p1(46.40, 19.73);
		const Point p2(-68.80, -68.80);
		const Point p3(-88.38, 73.24);
		const Point p4(20.22, 41.61);
		const Point p5(-95.88, 93.98);
		const Point p6(66.49, -57.53);
		const Point p7(-63.64, -63.32);
		const Point p8(-39.15, 4.95);
		const Point p9(-13.61, -41.75);

		std::vector<Point> points{p0, p1, p2, p3, p4, p5, p6, p7, p8, p9};
		DelaunayTriangulation triangulation = bowyer_watson_triangulate(points);

		std::vector<std::string> expected_keys{
			triangle_key_from_points({p2, p3, p5}),
			triangle_key_from_points({p9, p2, p6}),
			triangle_key_from_points({p9, p4, p8}),
			triangle_key_from_points({p3, p0, p5}),
			triangle_key_from_points({p4, p0, p8}),
			triangle_key_from_points({p0, p3, p8}),
			triangle_key_from_points({p9, p7, p2}),
			triangle_key_from_points({p7, p9, p8}),
			triangle_key_from_points({p7, p3, p2}),
			triangle_key_from_points({p3, p7, p8}),
			triangle_key_from_points({p1, p9, p6}),
			triangle_key_from_points({p9, p1, p4}),
			triangle_key_from_points({p0, p1, p4})
		};
		std::sort(expected_keys.begin(), expected_keys.end());

		expect_true(summary,
		            triangulation.triangles.size() == expected_keys.size(),
		            suite_name,
		            "image example corrected triangle count",
		            "The provided 10-point example should produce 13 Delaunay triangles.");
		expect_true(summary,
		            canonical_triangle_keys(triangulation) == expected_keys,
		            suite_name,
		            "image example corrected triangle set",
		            "The provided 10-point example should match the corrected Delaunay triangle set exactly.");
	}
}
