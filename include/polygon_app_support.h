#ifndef POLYGON_APP_SUPPORT_H
#define POLYGON_APP_SUPPORT_H

#include <string>
#include <vector>

#include "polygon.h"

struct PolygonAppPaths {
	std::string latest_polygon_output_path = "tools/desmos-bridge/latest-polygon-export.json";
	std::string latest_triangulation_output_path = "tools/desmos-bridge/latest-triangulation-export.json";
	std::string legacy_polygon_output_path = "tools/desmos-bridge/polygon-export.json";
	std::string legacy_triangulation_output_path = "tools/desmos-bridge/triangulation-export.json";
	std::string bridge_autoload_output_path = "tools/desmos-bridge/latest-session.js";
	std::string bridge_path = "tools/desmos-bridge/index.html";
};

bool parse_bool_env(const char* value);
void clear_screen();
int read_menu_choice(int min_choice, int max_choice);
int read_positive_int(const std::string& prompt, int minimum);
Polygon make_sample_polygon();
void print_geometry_input_guidance();
void print_polygon_summary(const Polygon& polygon, const std::string& label);
bool export_polygon_bundle(const Polygon& polygon, const PolygonAppPaths& paths, bool skip_browser_launch);
void view_stored_geometry(const std::vector<Polygon>& stored_polygons);

#endif
