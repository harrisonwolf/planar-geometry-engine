#include <cstdlib>
#include <iostream>
#include <limits>
#include <list>
#include <string>
#include <vector>
#include <filesystem>
#include <unistd.h>

#include "build_info.h"
#include "choice.h"
#include "helper.h"
#include "logger.h"
#include "polygon.h"
#include "random_polygon_generator.h"

using namespace std;

namespace {

struct RuntimeOptions {
	bool show_version = false;
	bool run_sample_demo = false;
	bool skip_browser_launch = false;
	bool classic_menu = false;
};

struct DemoPaths {
	string latest_polygon_output_path = "tools/desmos-bridge/latest-polygon-export.json";
	string latest_triangulation_output_path = "tools/desmos-bridge/latest-triangulation-export.json";
	string legacy_polygon_output_path = "tools/desmos-bridge/polygon-export.json";
	string legacy_triangulation_output_path = "tools/desmos-bridge/triangulation-export.json";
	string bridge_autoload_output_path = "tools/desmos-bridge/latest-session.js";
	string bridge_path = "tools/desmos-bridge/index.html";
};

bool parse_bool_env(const char* value) {
	if(value == nullptr) {
		return false;
	}

	string env_value = value;
	return env_value == "1" || env_value == "true" || env_value == "TRUE" || env_value == "on";
}

RuntimeOptions parse_runtime_options(int argc, char* argv[]) {
	RuntimeOptions options;
	options.skip_browser_launch = parse_bool_env(std::getenv("GEOM_SKIP_BROWSER"));

	for(int i = 1; i < argc; ++i) {
		string arg = argv[i];
		if(arg == "--version" || arg == "--about") {
			options.show_version = true;
		}else if(arg == "--run-sample-demo") {
			options.run_sample_demo = true;
		}else if(arg == "--no-browser-launch") {
			options.skip_browser_launch = true;
		}else if(arg == "--classic-menu") {
			options.classic_menu = true;
		}
	}

	return options;
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

void print_capabilities() {
	cout << "Supported demo workflows:\n";
	cout << "- Polygon creation and storage\n";
	cout << "- Polygon triangulation visualization\n";
	cout << "- Desmos bridge export for polygon and triangulation artifacts\n";
	cout << "\nFuture-ready release surface:\n";
	cout << "- Additional triangulation algorithms can be added as new workflows\n";
	cout << "- Additional visual artifact types (for example Delaunay or Voronoi) can reuse the same export/bridge structure\n";
}

void print_about() {
	cout << "Planar Geometry Demo\n";
	cout << "Interactive polygon demo for interview/release packaging.\n";
	cout << "Build profile: " << build_info::profile() << "\n";
	cout << "Branch: " << build_info::branch() << "\n";
	cout << "Commit: " << build_info::commit_full() << "\n";
	cout << "Built at: " << build_info::build_time_utc() << "\n";
	cout << "Working tree dirty at build time: " << (build_info::dirty() ? "yes" : "no") << "\n\n";
	print_capabilities();
}

void print_polygon_summary(const Polygon& polygon, const string& label) {
	cout << label << "\n";
	cout << polygon.to_string() << "\n";
	cout << "\nTriangulation:\n";
	polygon.print_triangulation();
	cout << "\n";
}

bool export_polygon_bundle(const Polygon& polygon, const DemoPaths& paths, bool skip_browser_launch) {
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
		cout << "No stored polygons yet. Create one or run the sample demo first.\n";
		return;
	}

	cout << "Stored polygons: " << stored_polygons.size() << "\n\n";
	for(size_t i = 0; i < stored_polygons.size(); ++i) {
		cout << "Polygon #" << (i + 1) << "\n";
		cout << stored_polygons[i].to_string() << "\n";
		cout << "Triangulation triangle count: " << stored_polygons[i].get_triangulation().size() << "\n\n";
	}
}

void run_sample_demo(vector<Polygon>& stored_polygons, const DemoPaths& paths, bool skip_browser_launch) {
	clear_screen();
	cout << "Running sample demo...\n\n";
	Polygon sample_polygon = make_sample_polygon();
	stored_polygons.push_back(sample_polygon);
	print_polygon_summary(stored_polygons.back(), "Sample polygon");
	export_polygon_bundle(stored_polygons.back(), paths, skip_browser_launch);
}

void run_interview_menu(const RuntimeOptions& options) {
	vector<Polygon> stored_polygons;
	const DemoPaths paths;

	cout << "\033[1;31mGeneral-position assumptions apply: no shared x-coordinates and no three colinear points.\033[0m\n";
	cout << "\033[1;31mProvide polygon vertices in clockwise order and ensure polygons are simple.\033[0m\n\n";

	while(true) {
		print_choices();
		const int choice = read_menu_choice(1, 7);
		cout << "\n";

		if(choice == 1) {
			run_sample_demo(stored_polygons, paths, options.skip_browser_launch);
		}else if(choice == 2) {
			Polygon polygon = read_polygon();
			stored_polygons.push_back(polygon);
			print_polygon_summary(stored_polygons.back(), "Stored manual polygon");
		}else if(choice == 3) {
			cout << "1: Use the curated sample polygon\n";
			cout << "2: Generate a random polygon\n";
			const int subchoice = read_menu_choice(1, 2);
			if(subchoice == 1) {
				Polygon polygon = make_sample_polygon();
				stored_polygons.push_back(polygon);
				print_polygon_summary(stored_polygons.back(), "Stored sample polygon");
			}else {
				const int n = read_positive_int("How many vertices should the random polygon have?\n", 3);
				Polygon polygon = generate_random_polygon(n);
				stored_polygons.push_back(polygon);
				print_polygon_summary(stored_polygons.back(), "Stored random polygon");
			}
		}else if(choice == 4) {
			view_stored_geometry(stored_polygons);
		}else if(choice == 5) {
			if(stored_polygons.empty()) {
				cout << "No stored polygon to export. Create one or run the sample demo first.\n";
			}else {
				export_polygon_bundle(stored_polygons.back(), paths, options.skip_browser_launch);
			}
		}else if(choice == 6) {
			print_about();
		}else if(choice == 7) {
			return;
		}

		cout << "\nPress 1 to continue or 2 to quit:\n";
		const int next_step = read_menu_choice(1, 2);
		if(next_step == 2) {
			return;
		}

		clear_screen();
	}
}

void run_classic_menu() {
	clear_screen();

	vector<Point> stored_points;
	vector<Line> stored_lines;
	vector<Triangle> stored_triangles;
	vector<Polygon> stored_polygons;

	bool first_iteration = true;
	cout << "\n";

	while(true) {
		if(!first_iteration) {
			clear_screen();
		}else {
			first_iteration = false;
		}

		cout << "\033[1mClassic developer menu\n\033[0m";
		cout << "1: Create a Triangle\n";
		cout << "2: Find the intersection of 2 lines\n";
		cout << "3: View Created Points, Lines, Triangles, or Polygons\n";
		cout << "4: Create a Polygon\n";
		cout << "5: Create a Polygon with randomly selected vertices\n";
		cout << "6: Export latest polygon to Desmos bridge\n";
		cout << "7: Quit\n";

		const int choice = read_menu_choice(1, 7);

		if(choice == 1) {
			Triangle t = read_triangle();
			stored_triangles.push_back(t);
			cout << "Your triangle:\n" << t.to_string() << endl;
		}else if(choice == 2) {
			cout << "\n";
			vector<Line> my_lines = read_lines(2);
			Line first = my_lines.at(0);
			Line second = my_lines.at(1);

			Point poi = first.intersection(second);
			stored_lines.push_back(first);
			stored_lines.push_back(second);
			stored_points.push_back(poi);

			cout << "\nPoint of intersection:\n" << poi.to_string() << endl;
		}else if(choice == 3) {
			cout << "Please select which you would like to view:\n";
			cout << "1: Stored Points\n";
			cout << "2: Stored Lines\n";
			cout << "3: Stored Triangles\n";
			cout << "4: Stored Polygons\n";
			cout << "5: All\n";
			const int subchoice = read_menu_choice(1, 5);

			if(subchoice == 1) {
				cout << "The points that have been created are:\n\n";
				for(const Point& p : stored_points) {
					cout << p.to_string() << "\n\n";
				}
			}else if(subchoice == 2) {
				cout << "The lines that have been created are:\n\n";
				for(const Line& l : stored_lines) {
					cout << l.to_string() << "\n\n";
				}
			}else if(subchoice == 3) {
				cout << "The triangles that have been created are:\n\n";
				for(const Triangle& t : stored_triangles) {
					cout << t.to_string() << "\n\n";
				}
			}else if(subchoice == 4) {
				cout << "The polygons that have been created are:\n\n";
				for(const Polygon& p : stored_polygons) {
					cout << p.to_string() << "\n\n";
				}
			}else {
				cout << "Points:\n";
				for(const Point& p : stored_points) {
					cout << p.to_string() << "\n";
				}
				cout << "\nLines:\n";
				for(const Line& l : stored_lines) {
					cout << l.to_string() << "\n";
				}
				cout << "\nTriangles:\n";
				for(const Triangle& t : stored_triangles) {
					cout << t.to_string() << "\n";
				}
				cout << "\nPolygons:\n";
				for(const Polygon& p : stored_polygons) {
					cout << p.to_string() << "\n";
				}
				cout << "\n";
			}
		}else if(choice == 4) {
			Polygon p = read_polygon();
			stored_polygons.push_back(p);
			cout << p.to_string() << endl;
			cout << "\nTriangulation of p: \n";
			p.print_triangulation();
			cout << "\n\n";
		}else if(choice == 5) {
			const int n = read_positive_int("Please enter how many vertices your polygon has:\n", 3);
			Polygon rand_poly = generate_random_polygon(n);
			stored_polygons.push_back(rand_poly);
			cout << "Your polygon:\n" << rand_poly.to_string() << "\n\n";
		}else if(choice == 6) {
			if(stored_polygons.empty()) {
				cout << "No stored polygons to export. Create one first.\n";
			}else{
				export_polygon_bundle(stored_polygons.back(), DemoPaths{}, false);
			}
		}else {
			return;
		}

		cout << "Please enter 1 to continue, 2 to quit:\n";
		const int next_step = read_menu_choice(1, 2);
		if(next_step == 2) {
			return;
		}
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

	if(options.run_sample_demo) {
		vector<Polygon> stored_polygons;
		run_sample_demo(stored_polygons, DemoPaths{}, options.skip_browser_launch);
		return 0;
	}

	if(options.classic_menu) {
		run_classic_menu();
		return 0;
	}

	run_interview_menu(options);
	return 0;
}
