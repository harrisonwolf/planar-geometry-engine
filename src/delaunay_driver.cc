#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include "delaunay.h"
#include "helper.h"
#include "logger.h"
#include "voronoi.h"

using namespace std;

namespace {

struct RuntimeOptions {
	bool use_sample = false;
	bool skip_browser_launch = false;
};

struct DriverPaths {
	string viewer_dir = "tools/delaunay-voronoi-viewer";
	string latest_delaunay_output_path = viewer_dir + "/latest-delaunay-export.json";
	string latest_voronoi_output_path = viewer_dir + "/latest-voronoi-export.json";
	string legacy_delaunay_output_path = viewer_dir + "/delaunay-export.json";
	string legacy_voronoi_output_path = viewer_dir + "/voronoi-export.json";
	string latest_session_output_path = viewer_dir + "/latest-session.js";
	string viewer_path = viewer_dir + "/index.html";
};

RuntimeOptions parse_runtime_options(int argc, char* argv[]){
	RuntimeOptions options;
	for(int i = 1; i < argc; ++i){
		string arg = argv[i];
		if(arg == "--sample"){
			options.use_sample = true;
		}else if(arg == "--no-browser-launch"){
			options.skip_browser_launch = true;
		}
	}
	return options;
}

int read_point_count(){
	cout << "How many points should be triangulated?\n";
	while(true){
		int value = 0;
		if(cin >> value && value >= 3){
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			return value;
		}

		cin.clear();
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
		cout << "Please enter an integer greater than or equal to 3.\n";
	}
}

vector<Point> read_point_set(){
	int count = read_point_count();
	vector<Point> points;
	points.reserve(static_cast<size_t>(count));

	cout << "Enter each point as: x y\n";
	for(int i = 0; i < count; ++i){
		cout << "Point " << (i + 1) << ": ";
		points.push_back(read_point());
	}

	return points;
}

vector<Point> make_sample_point_set(){
	return vector<Point>{
		Point(-4.0, -1.0),
		Point(-1.5, 3.5),
		Point(2.5, 4.0),
		Point(5.0, 0.5),
		Point(3.0, -3.0),
		Point(-2.5, -4.0),
		Point(0.5, 0.25)
	};
}

string read_text_file(const string& path){
	ifstream input(path);
	ostringstream buffer;
	buffer << input.rdbuf();
	return buffer.str();
}

bool write_autoload_session_file(const DriverPaths& paths){
	string delaunay_json = read_text_file(paths.latest_delaunay_output_path);
	string voronoi_json = read_text_file(paths.latest_voronoi_output_path);
	if(delaunay_json.empty() || voronoi_json.empty()){
		return false;
	}

	ofstream output(paths.latest_session_output_path);
	if(!output.is_open()){
		return false;
	}

	output << "window.__DELAUNAY_VIEWER_AUTOLOAD__ = {\n";
	output << "  schemaVersion: 1,\n";
	output << "  artifacts: [\n";
	output << delaunay_json << ",\n";
	output << voronoi_json << "\n";
	output << "  ]\n";
	output << "};\n";
	return true;
}

bool export_latest_artifacts(const DelaunayTriangulation& triangulation,
                             const VoronoiDiagram& voronoi,
                             const DriverPaths& paths,
                             bool skip_browser_launch){
	std::filesystem::create_directories(paths.viewer_dir);

	bool latest_delaunay_ok = write_delaunay_schema_file(
		triangulation, "sample_delaunay", paths.latest_delaunay_output_path);
	bool latest_voronoi_ok = write_voronoi_schema_file(
		voronoi, "sample_voronoi", paths.latest_voronoi_output_path);
	bool legacy_delaunay_ok = write_delaunay_schema_file(
		triangulation, "sample_delaunay", paths.legacy_delaunay_output_path);
	bool legacy_voronoi_ok = write_voronoi_schema_file(
		voronoi, "sample_voronoi", paths.legacy_voronoi_output_path);
	bool latest_session_ok = latest_delaunay_ok && latest_voronoi_ok && write_autoload_session_file(paths);

	if(!latest_delaunay_ok || !latest_voronoi_ok || !legacy_delaunay_ok || !legacy_voronoi_ok || !latest_session_ok){
		cout << "Failed to export one or more Delaunay/Voronoi artifacts.\n";
		return false;
	}

	cout << "Exported Delaunay artifact to "
	     << std::filesystem::absolute(paths.latest_delaunay_output_path).string() << "\n";
	cout << "Exported Voronoi artifact to "
	     << std::filesystem::absolute(paths.latest_voronoi_output_path).string() << "\n";
	cout << "Exported viewer autoload session to "
	     << std::filesystem::absolute(paths.latest_session_output_path).string() << "\n";
	cout << "Also refreshed compatibility copies at delaunay-export.json and voronoi-export.json.\n";

	if(skip_browser_launch){
		cout << "Browser launch skipped.\n";
		cout << "Open " << paths.viewer_path << " and use the latest-load buttons, or import latest-delaunay-export.json plus latest-voronoi-export.json manually.\n";
		return true;
	}

	if(!open_delaunay_viewer_page(paths.viewer_path, true)){
		cout << "If the browser did not open, manually open " << paths.viewer_path << "\n";
		cout << "Then use the latest-load buttons or import latest-delaunay-export.json and latest-voronoi-export.json.\n";
	}

	return true;
}

void print_summary(const vector<Point>& points,
                   const DelaunayTriangulation& triangulation,
                   const VoronoiDiagram& voronoi){
	cout << "Input sites: " << points.size() << "\n";
	cout << "Delaunay triangles: " << triangulation.triangles.size() << "\n";
	cout << "Voronoi vertices: " << voronoi.vertices.size() << "\n";
	cout << "Voronoi finite edges: " << voronoi.edges.size() << "\n";
	cout << "Delaunay validation: " << (is_delaunay(triangulation) ? "passed" : "failed") << "\n";
}

} // namespace

int main(int argc, char* argv[]){
	logger::apply_runtime_inputs(argc, argv);

	RuntimeOptions options = parse_runtime_options(argc, argv);
	vector<Point> points = options.use_sample ? make_sample_point_set() : read_point_set();

	DelaunayTriangulation triangulation = bowyer_watson_triangulate(points);
	if(triangulation.triangles.empty()){
		cout << "Delaunay triangulation failed.\n";
		return 1;
	}

	VoronoiDiagram voronoi = build_voronoi_diagram(triangulation);
	if(voronoi.vertices.empty() && voronoi.edges.empty()){
		cout << "Voronoi diagram generation failed.\n";
		return 1;
	}

	print_summary(points, triangulation, voronoi);
	DriverPaths paths;
	if(!export_latest_artifacts(triangulation, voronoi, paths, options.skip_browser_launch)){
		return 1;
	}

	return 0;
}
