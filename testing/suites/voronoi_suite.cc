#include "suites.h"

#include <array>
#include <vector>

#include "../test_assertions.h"
#include "delaunay.h"
#include "voronoi.h"

void run_voronoi_suite(TestRunSummary& summary){
	const std::string suite_name = "voronoi";

	{
		DelaunayTriangulation triangulation;
		triangulation.sites = {
			Point(0.0, 0.0),
			Point(4.0, 0.0),
			Point(0.0, 4.0)
		};
		triangulation.triangles = {IndexedTriangle{0, 1, 2}};
		triangulation.neighbors = {std::array<int, 3>{-1, -1, -1}};

		VoronoiDiagram diagram = build_voronoi_diagram(triangulation);

		expect_true(summary,
		            diagram.vertices.size() == 1,
		            suite_name,
		            "single triangle vertex count",
		            "A one-triangle triangulation should yield exactly one Voronoi vertex.");
		expect_true(summary,
		            diagram.edges.empty(),
		            suite_name,
		            "single triangle finite edge count",
		            "A one-triangle triangulation should yield no finite Voronoi edges.");
		expect_near(summary,
		            diagram.vertices.at(0).get_x(),
		            2.0,
		            1e-6,
		            suite_name,
		            "single triangle circumcenter x",
		            "The Voronoi vertex should match the triangle circumcenter.");
		expect_near(summary,
		            diagram.vertices.at(0).get_y(),
		            2.0,
		            1e-6,
		            suite_name,
		            "single triangle circumcenter y",
		            "The Voronoi vertex should match the triangle circumcenter.");
	}

	{
		DelaunayTriangulation triangulation;
		triangulation.sites = {
			Point(0.0, 0.0),
			Point(4.0, 0.0),
			Point(3.0, 3.0),
			Point(0.0, 4.0)
		};
		triangulation.triangles = {
			IndexedTriangle{0, 1, 2},
			IndexedTriangle{0, 2, 3}
		};
		triangulation.neighbors = {
			std::array<int, 3>{-1, -1, 1},
			std::array<int, 3>{0, -1, -1}
		};

		VoronoiDiagram diagram = build_voronoi_diagram(triangulation);

		expect_true(summary,
		            diagram.vertices.size() == 2,
		            suite_name,
		            "two triangle vertex count",
		            "Two adjacent triangles should yield two Voronoi vertices.");
		expect_true(summary,
		            diagram.edges.size() == 1,
		            suite_name,
		            "two triangle finite edge count",
		            "Two adjacent triangles should yield one finite Voronoi edge.");
		expect_true(summary,
		            diagram.edges.at(0).start_vertex == 0 && diagram.edges.at(0).end_vertex == 1,
		            suite_name,
		            "adjacent triangle edge endpoints",
		            "The Voronoi edge should connect the two triangle circumcenters.");
		expect_true(summary,
		            diagram.edges.at(0).left_site == 0 && diagram.edges.at(0).right_site == 2,
		            suite_name,
		            "source site pair",
		            "The Voronoi edge should preserve the dual Delaunay site pair.");
	}

	{
		std::vector<Point> points{
			Point(-4.0, -1.0),
			Point(-1.5, 3.5),
			Point(2.5, 4.0),
			Point(5.0, 0.5),
			Point(3.0, -3.0),
			Point(-2.5, -4.0),
			Point(0.5, 0.25)
		};
		DelaunayTriangulation triangulation = bowyer_watson_triangulate(points);
		VoronoiDiagram diagram = build_voronoi_diagram(triangulation);

		expect_true(summary,
		            diagram.vertices.size() == triangulation.triangles.size(),
		            suite_name,
		            "vertex per triangle",
		            "Voronoi construction should produce one circumcenter per Delaunay triangle.");
		expect_true(summary,
		            diagram.edges.size() >= 3,
		            suite_name,
		            "multiple finite edges",
		            "A triangulation with an interior site should yield multiple finite Voronoi edges.");

		for(size_t i = 0; i < triangulation.triangles.size(); ++i){
			const IndexedTriangle& triangle = triangulation.triangles.at(i);
			Point center = circumcenter(triangulation.sites.at(triangle.a),
			                            triangulation.sites.at(triangle.b),
			                            triangulation.sites.at(triangle.c));
			expect_near(summary,
			            diagram.vertices.at(i).get_x(),
			            center.get_x(),
			            1e-6,
			            suite_name,
			            "vertex x matches circumcenter " + std::to_string(i),
			            "Voronoi vertices should match triangle circumcenters.");
			expect_near(summary,
			            diagram.vertices.at(i).get_y(),
			            center.get_y(),
			            1e-6,
			            suite_name,
			            "vertex y matches circumcenter " + std::to_string(i),
			            "Voronoi vertices should match triangle circumcenters.");
		}
	}
}
