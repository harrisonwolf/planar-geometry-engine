#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "delaunay.h"
#include "helper.h"
#include "logger.h"
#include "voronoi.h"

using namespace std;

namespace {

struct RuntimeOptions {
	bool use_sample = false;
	bool use_random = false;
	bool skip_browser_launch = false;
	bool valid = true;
	bool coord_max_explicit = false;
	int random_count = 0;
	int coord_max = 10;
	string error_message;
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
		}else if(arg == "--random"){
			options.use_random = true;
		}else if(arg == "--no-browser-launch"){
			options.skip_browser_launch = true;
		}else if(arg.rfind("--random-count=", 0) == 0){
			try {
				options.random_count = stoi(arg.substr(15));
				options.use_random = true;
			} catch(const exception&){
				options.valid = false;
				options.error_message = "Invalid value for --random-count.";
				return options;
			}
		}else if(arg.rfind("--coord-max=", 0) == 0){
			try {
				options.coord_max = stoi(arg.substr(12));
				options.coord_max_explicit = true;
			} catch(const exception&){
				options.valid = false;
				options.error_message = "Invalid value for --coord-max.";
				return options;
			}
		}
	}

	if(options.use_sample && options.use_random){
		options.valid = false;
		options.error_message = "Choose either --sample or --random, not both.";
		return options;
	}
	if(options.use_random && options.random_count < 0){
		options.valid = false;
		options.error_message = "--random-count must be greater than or equal to 3.";
		return options;
	}
	if(options.use_random && options.coord_max < 0){
		options.valid = false;
		options.error_message = "--coord-max must be non-negative.";
		return options;
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

int read_input_mode(){
	cout << "Select input mode:\n";
	cout << "1: Use built-in sample point set\n";
	cout << "2: Enter points manually\n";
	cout << "3: Generate random points\n";
	while(true){
		int value = 0;
		if(cin >> value && value >= 1 && value <= 3){
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			return value;
		}

		cin.clear();
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
		cout << "Please enter 1, 2, or 3.\n";
	}
}

int read_coordinate_max(){
	cout << "What should the coordinate max be? Points will be generated within [-max, max].\n";
	while(true){
		int value = 0;
		if(cin >> value && value >= 0){
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			return value;
		}

		cin.clear();
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
		cout << "Please enter an integer greater than or equal to 0.\n";
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

bool are_all_points_collinear(const vector<Point>& points){
	if(points.size() < 3){
		return true;
	}

	const Point& anchor = points.front();
	size_t second_index = 1;
	while(second_index < points.size() && points.at(second_index) == anchor){
		++second_index;
	}
	if(second_index >= points.size()){
		return true;
	}

	for(size_t i = second_index + 1; i < points.size(); ++i){
		if(std::fabs(orient2d(anchor, points.at(second_index), points.at(i))) > 1e-9){
			return false;
		}
	}
	return true;
}

vector<Point> generate_random_point_set(int count, int coord_max){
	vector<Point> points;
	if(count < 3){
		cout << "Random generation requires at least 3 points.\n";
		return points;
	}

	long long axis_count = static_cast<long long>(coord_max) * 2LL + 1LL;
	long long capacity = axis_count * axis_count;
	if(capacity < count){
		cout << "Random generation cannot place " << count
		     << " unique integer points inside [-" << coord_max << ", " << coord_max << "].\n";
		return points;
	}

	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<int> coord_dist(-coord_max, coord_max);

	const int max_attempts = 256;
	for(int attempt = 0; attempt < max_attempts; ++attempt){
		points.clear();
		points.reserve(static_cast<size_t>(count));
		set<pair<int, int>> used;

		while(static_cast<int>(points.size()) < count){
			int x = coord_dist(gen);
			int y = coord_dist(gen);
			if(!used.insert({x, y}).second){
				continue;
			}
			points.push_back(Point(static_cast<double>(x), static_cast<double>(y)));
		}

		if(!are_all_points_collinear(points)){
			return points;
		}
	}

	cout << "Random generation failed to produce a non-collinear point set after many attempts.\n";
	return {};
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

vector<Point> select_input_points(RuntimeOptions& options){
	if(!options.use_sample && !options.use_random){
		int mode = read_input_mode();
		if(mode == 1){
			options.use_sample = true;
		}else if(mode == 3){
			options.use_random = true;
		}
	}

	if(options.use_sample){
		cout << "Using built-in sample point set.\n";
		return make_sample_point_set();
	}

	if(options.use_random){
		if(options.random_count == 0){
			options.random_count = read_point_count();
		}
		if(!options.coord_max_explicit){
			options.coord_max = read_coordinate_max();
		}
		vector<Point> points = generate_random_point_set(options.random_count, options.coord_max);
		if(!points.empty()){
			cout << "Generated " << options.random_count << " random points within [-"
			     << options.coord_max << ", " << options.coord_max << "].\n";
		}
		return points;
	}

	return read_point_set();
}

} // namespace

int main(int argc, char* argv[]){
	logger::apply_runtime_inputs(argc, argv);

	RuntimeOptions options = parse_runtime_options(argc, argv);
	if(!options.valid){
		cout << options.error_message << "\n";
		return 1;
	}
	vector<Point> points = select_input_points(options);
	if(points.empty()){
		return 1;
	}

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
