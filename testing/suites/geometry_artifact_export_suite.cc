#include "suites.h"

#include <array>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "../test_assertions.h"
#include "helper.h"

namespace {

std::string read_file(const std::filesystem::path& path){
	std::ifstream input(path);
	std::ostringstream buffer;
	buffer << input.rdbuf();
	return buffer.str();
}

} // namespace

void run_geometry_artifact_export_suite(TestRunSummary& summary){
	const std::string suite_name = "geometry_artifact_export";

	std::filesystem::path output_dir = std::filesystem::path("build") / "test-artifacts";
	std::filesystem::create_directories(output_dir);

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

	std::filesystem::path delaunay_path = output_dir / "delaunay-export.json";
	std::filesystem::path voronoi_path = output_dir / "voronoi-export.json";

	bool delaunay_ok = write_delaunay_schema_file(triangulation, "test_delaunay", delaunay_path.string());
	bool voronoi_ok = write_voronoi_schema_file(diagram, "test_voronoi", voronoi_path.string());
	expect_true(summary,
	            delaunay_ok,
	            suite_name,
	            "write delaunay export",
	            "Delaunay schema export should succeed.");
	expect_true(summary,
	            voronoi_ok,
	            suite_name,
	            "write voronoi export",
	            "Voronoi schema export should succeed.");

	std::string delaunay_json = read_file(delaunay_path);
	std::string voronoi_json = read_file(voronoi_path);

	expect_true(summary,
	            delaunay_json.find("\"type\": \"delaunay_triangulation\"") != std::string::npos,
	            suite_name,
	            "delaunay export type",
	            "Delaunay export should include the expected artifact type.");
	expect_true(summary,
	            delaunay_json.find("\"triangles\"") != std::string::npos,
	            suite_name,
	            "delaunay export triangles",
	            "Delaunay export should include indexed triangles.");
	expect_true(summary,
	            voronoi_json.find("\"type\": \"voronoi_diagram\"") != std::string::npos,
	            suite_name,
	            "voronoi export type",
	            "Voronoi export should include the expected artifact type.");
	expect_true(summary,
	            voronoi_json.find("\"edges\"") != std::string::npos,
	            suite_name,
	            "voronoi export edges",
	            "Voronoi export should include finite edges.");
}
