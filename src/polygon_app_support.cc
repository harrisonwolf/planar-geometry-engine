#include "polygon_app_support.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <limits>
#include <list>
#include <string>
#include <unistd.h>
#include <vector>

#include "helper.h"

using namespace std;

bool parse_bool_env(const char* value) {
	if(value == nullptr) {
		return false;
	}

	string env_value = value;
	return env_value == "1" || env_value == "true" || env_value == "TRUE" || env_value == "on";
}

void clear_screen() {
	if(std::getenv("TERM") == nullptr || !isatty(STDOUT_FILENO)) {
		return;
	}
	cout << "\033[2J\033[H";
}

int read_menu_choice(int min_choice, int max_choice) {
	int choice = 0;
	while(true) {
		if(cin >> choice && choice >= min_choice && choice <= max_choice) {
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			return choice;
		}

		cin.clear();
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
		cout << "Please enter a number between " << min_choice << " and " << max_choice << ".\n";
	}
}

int read_positive_int(const string& prompt, int minimum) {
	cout << prompt;
	while(true) {
		int value = 0;
		if(cin >> value && value >= minimum) {
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			return value;
		}

		cin.clear();
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
		cout << "Please enter an integer greater than or equal to " << minimum << ".\n";
	}
}

Polygon make_sample_polygon() {
	return Polygon(list<Point>{
		Point(-4.0, 0.0),
		Point(-1.0, 3.0),
		Point(3.0, 4.0),
		Point(5.0, 1.0),
		Point(2.0, -3.0),
		Point(-3.0, -4.0)
	});
}

void print_geometry_input_guidance() {
	cout << "\033[1;31mGeneral-position assumptions apply: no shared x-coordinates and no three colinear points.\033[0m\n";
	cout << "\033[1;31mProvide polygon vertices in clockwise order and ensure polygons are simple.\033[0m\n\n";
}

void print_polygon_summary(const Polygon& polygon, const string& label) {
	cout << label << "\n";
	cout << polygon.to_string() << "\n";
	cout << "\nTriangulation:\n";
	polygon.print_triangulation();
	cout << "\n";
}

bool export_polygon_bundle(const Polygon& polygon, const PolygonAppPaths& paths, bool skip_browser_launch) {
	const bool latest_polygon_ok = write_polygon_schema_file(polygon, "poly1", paths.latest_polygon_output_path);
	const bool latest_triangulation_ok =
		write_triangulation_schema_file(polygon, "poly1_triangulation", paths.latest_triangulation_output_path);
	const bool legacy_polygon_ok = write_polygon_schema_file(polygon, "poly1", paths.legacy_polygon_output_path);
	const bool legacy_triangulation_ok =
		write_triangulation_schema_file(polygon, "poly1_triangulation", paths.legacy_triangulation_output_path);
	const bool bridge_autoload_ok = write_bridge_autoload_file(polygon, paths.bridge_autoload_output_path);

	if(!latest_polygon_ok || !latest_triangulation_ok || !legacy_polygon_ok ||
	   !legacy_triangulation_ok || !bridge_autoload_ok) {
		cout << "Failed to export geometry artifacts for the visualizer.\n";
		return false;
	}

	const std::filesystem::path latest_polygon_path = std::filesystem::absolute(paths.latest_polygon_output_path);
	const std::filesystem::path latest_triangulation_path = std::filesystem::absolute(paths.latest_triangulation_output_path);
	const std::filesystem::path autoload_path = std::filesystem::absolute(paths.bridge_autoload_output_path);
	cout << "Exported latest polygon artifact to " << latest_polygon_path.string() << "\n";
	cout << "Exported latest triangulation artifact to " << latest_triangulation_path.string() << "\n";
	cout << "Exported bridge autoload session to " << autoload_path.string() << "\n";
	cout << "Also refreshed compatibility copies at polygon-export.json and triangulation-export.json.\n";

	if(skip_browser_launch) {
		cout << "Browser launch skipped.\n";
		cout << "Open " << paths.bridge_path << " and load latest-polygon-export.json plus latest-triangulation-export.json.\n";
		return true;
	}

	if(!open_desmos_bridge_page(paths.bridge_path, true)) {
		cout << "If the browser did not open, manually open " << paths.bridge_path << "\n";
		cout << "Then load latest-polygon-export.json and latest-triangulation-export.json.\n";
	}

	return true;
}

void view_stored_geometry(const vector<Polygon>& stored_polygons) {
	if(stored_polygons.empty()) {
		cout << "No stored polygons yet. Create one first.\n";
		return;
	}

	cout << "Stored polygons: " << stored_polygons.size() << "\n\n";
	for(size_t i = 0; i < stored_polygons.size(); ++i) {
		cout << "Polygon #" << (i + 1) << "\n";
		cout << stored_polygons[i].to_string() << "\n";
		cout << "Triangulation triangle count: " << stored_polygons[i].get_triangulation().size() << "\n\n";
	}
}
