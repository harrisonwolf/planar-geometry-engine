#include <cstdlib>
#include <iostream>
#include <list>
#include <optional>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "build_info.h"
#include "helper.h"
#include "logger.h"
#include "polygon.h"
#include "polygon_app_support.h"
#include "random_polygon_generator.h"

using namespace std;

namespace {

constexpr int kRandomVertexMin = 4;
constexpr int kRandomVertexMax = 99;

struct RuntimeOptions {
	bool show_version = false;
	bool skip_browser_launch = false;
};

struct RandomPolygonSettings {
	std::optional<int> vertex_count;
	std::optional<double> min_coord;
	std::optional<double> max_coord;
};

RuntimeOptions parse_runtime_options(int argc, char* argv[]) {
	RuntimeOptions options;
	options.skip_browser_launch = parse_bool_env(std::getenv("GEOM_SKIP_BROWSER"));

	for(int i = 1; i < argc; ++i) {
		string arg = argv[i];
		if(arg == "--version" || arg == "--about") {
			options.show_version = true;
		}else if(arg == "--no-browser-launch") {
			options.skip_browser_launch = true;
		}
	}

	return options;
}

void print_capabilities() {
	cout << "Stable release workflows:\n";
	cout << "- Create polygons manually\n";
	cout << "- Generate random polygons\n";
	cout << "- Store and review created polygons\n";
	cout << "- Inspect ear-clipping triangulations\n";
	cout << "- Export polygon and triangulation artifacts to the Desmos visualizer\n";
}

void print_about() {
	cout << "Planar Geometry\n";
	cout << "Stable release surface for polygon creation, storage, triangulation, and visualizer export.\n";
	cout << "Build profile: " << build_info::profile() << "\n";
	cout << "Branch: " << build_info::branch() << "\n";
	cout << "Commit: " << build_info::commit_full() << "\n";
	cout << "Built at: " << build_info::build_time_utc() << "\n";
	cout << "Working tree dirty at build time: " << (build_info::dirty() ? "yes" : "no") << "\n\n";
	print_capabilities();
}

void print_release_menu() {
	cout << "\033[1mPlanar Geometry\n\033[0m";
	cout << "1: Create polygon\n";
	cout << "2: View stored geometry\n";
	cout << "3: Export latest result to visualizer\n";
	cout << "4: About / version\n";
	cout << "5: Quit\n";
}

pair<double, double> read_coordinate_bounds() {
	while(true) {
		cout << "Enter minimum coordinate bound:\n";
		double min_coord = 0.0;
		if(!(cin >> min_coord)) {
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			cout << "Please enter numeric coordinate bounds.\n";
			continue;
		}

		cout << "Enter maximum coordinate bound:\n";
		double max_coord = 0.0;
		if(!(cin >> max_coord)) {
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			cout << "Please enter numeric coordinate bounds.\n";
			continue;
		}

		cin.ignore(numeric_limits<streamsize>::max(), '\n');
		if(min_coord >= max_coord) {
			cout << "Minimum coordinate bound must be less than maximum coordinate bound.\n";
			continue;
		}

		return {min_coord, max_coord};
	}
}

string format_setting_value(const optional<int>& value) {
	if(!value.has_value()) {
		return "random";
	}
	return to_string(*value);
}

string format_coordinate_bounds(const RandomPolygonSettings& settings) {
	if(!settings.min_coord.has_value() || !settings.max_coord.has_value()) {
		return "random";
	}

	ostringstream out;
	out << "[" << *settings.min_coord << ", " << *settings.max_coord << "]";
	return out.str();
}

int choose_random_vertex_count() {
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<int> dist(kRandomVertexMin, kRandomVertexMax);
	return dist(gen);
}

Polygon generate_random_polygon_with_settings(const RandomPolygonSettings& settings) {
	const int vertex_count = settings.vertex_count.value_or(choose_random_vertex_count());

	return settings.min_coord.has_value() && settings.max_coord.has_value()
		? generate_random_polygon(vertex_count, *settings.min_coord, *settings.max_coord)
		: generate_random_polygon(vertex_count);
}

void print_random_polygon_settings(const RandomPolygonSettings& settings) {
	cout << "Random polygon generation settings\n";
	cout << "Number of vertices: " << format_setting_value(settings.vertex_count) << "\n";
	cout << "Coordinate bounds: " << format_coordinate_bounds(settings) << "\n";
	cout << "Area: random\n\n";
	cout << "1: Generate\n";
	cout << "2: Set number of vertices\n";
	cout << "3: Set coordinate bounds\n";
	cout << "4: Set area\n";
}

optional<Polygon> configure_random_polygon() {
	RandomPolygonSettings settings;

	while(true) {
		print_random_polygon_settings(settings);
		const int choice = read_menu_choice(1, 4);
		cout << "\n";

		if(choice == 1) {
			return generate_random_polygon_with_settings(settings);
		}
		if(choice == 2) {
			settings.vertex_count = read_positive_int("How many vertices should the random polygon have?\n", 3);
		}else if(choice == 3) {
			const pair<double, double> bounds = read_coordinate_bounds();
			settings.min_coord = bounds.first;
			settings.max_coord = bounds.second;
		}else {
			cout << "Area-targeted random polygon generation is not implemented yet.\n";
			return nullopt;
		}

		cout << "\n";
	}
}

void create_polygon(vector<Polygon>& stored_polygons) {
	cout << "Polygon creation\n";
	cout << "1: Enter polygon manually\n";
	cout << "2: Generate random polygon\n";
	cout << "3: Back\n";
	const int subchoice = read_menu_choice(1, 3);

	if(subchoice == 1) {
		Polygon polygon = read_polygon();
		stored_polygons.push_back(polygon);
		print_polygon_summary(stored_polygons.back(), "Stored manual polygon");
	}else if(subchoice == 2) {
		optional<Polygon> polygon = configure_random_polygon();
		if(polygon.has_value()) {
			stored_polygons.push_back(*polygon);
			print_polygon_summary(stored_polygons.back(), "Stored random polygon");
		}
	}
}

bool prompt_to_continue() {
	cout << "\nPress 1 to return to the main menu or 2 to quit:\n";
	return read_menu_choice(1, 2) == 1;
}

void run_release_menu(const RuntimeOptions& options) {
	vector<Polygon> stored_polygons;
	const PolygonAppPaths paths;

	clear_screen();
	print_geometry_input_guidance();

	while(true) {
		print_release_menu();
		const int choice = read_menu_choice(1, 5);
		cout << "\n";

		if(choice == 1) {
			create_polygon(stored_polygons);
		}else if(choice == 2) {
			view_stored_geometry(stored_polygons);
		}else if(choice == 3) {
			if(stored_polygons.empty()) {
				cout << "No stored polygon to export. Create one first.\n";
			}else {
				export_polygon_bundle(stored_polygons.back(), paths, options.skip_browser_launch);
			}
		}else if(choice == 4) {
			print_about();
		}else {
			return;
		}

		if(!prompt_to_continue()) {
			return;
		}

		clear_screen();
	}
}

} // namespace

int main(int argc, char* argv[]) {
	logger::apply_runtime_inputs(argc, argv);
	const RuntimeOptions options = parse_runtime_options(argc, argv);

	if(options.show_version) {
		print_about();
		return 0;
	}

	run_release_menu(options);
	return 0;
}
