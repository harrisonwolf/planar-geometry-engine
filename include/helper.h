//header file for helper functions mainly dealing with taking as input points, lines, triangles, etc

#ifndef HELPER_H
#define HELPER_H

#include <iostream>
#include <vector>
#include <list>
#include "point.h"
#include "triangle.h"
//#include "polygon.h"
#include "polygon.h"
#include "line.h"
#include "delaunay.h"
#include "voronoi.h"
#include <utility> //for std::pair
#include <string>

enum class GeometryArtifactType {
	Polygon,
	Triangulation,
	DelaunayTriangulation,
	VoronoiDiagram
};

struct GeometryArtifactExport {
	GeometryArtifactType type;
	std::string artifact_id;
	std::string output_path;
};

/*
 * Reads a point
 */
Point read_point();

/*
 * Reads a point as input and returns it, with some output to the user
 */
Point read_point_verbose();

/*
 * Read n points from the user, prompting them with each one
 */
std::vector<Point> read_points(int n);
std::list<Point> read_point_list(int n);

/*
 * Read n lines from the user, giving them the option of how to
 * define each line
 */
std::vector<Line> read_lines(int n);

/*
 * Read one triangle from the user, prompting them how they want to define it
 */
Triangle read_triangle();

/*
 * Read n triangles from the user, prompting them how they want to define each triangle each time
 */
std::vector<Triangle> read_triangles(int n);

/*
 * Read one polygon from the user (currently must define by the set of vertices)
 */
Polygon read_polygon();

/*
 * TODO
 * Read multiple polygons from the user
 */
std::vector<Polygon> read_polygons(int n);

/*
 * Writes a polygon schema JSON file suitable for the Desmos bridge page.
 */
bool write_polygon_schema_file(const Polygon& polygon, const std::string& polygon_id,
                               const std::string& output_path);

/*
 * Writes a triangulation schema JSON file suitable for the Desmos bridge page.
 */
bool write_triangulation_schema_file(const Polygon& polygon, const std::string& triangulation_id,
                                     const std::string& output_path);

/*
 * Writes a Delaunay triangulation schema JSON file for the dedicated viewer.
 */
bool write_delaunay_schema_file(const DelaunayTriangulation& triangulation,
                                const std::string& triangulation_id,
                                const std::string& output_path);

/*
 * Writes a Voronoi diagram schema JSON file for the dedicated viewer.
 */
bool write_voronoi_schema_file(const VoronoiDiagram& diagram,
                               const std::string& diagram_id,
                               const std::string& output_path);

/*
 * Writes an export artifact by type. Polygon and triangulation are currently supported.
 * Future release surfaces can add Delaunay/Voronoi artifacts without changing callers.
 */
bool write_geometry_artifact_file(const Polygon& polygon, const GeometryArtifactExport& export_spec);

/*
 * Writes a JS bridge session file containing the latest polygon and triangulation artifacts
 * so the browser visualizer can load them directly on page open.
 */
bool write_bridge_autoload_file(const Polygon& polygon, const std::string& output_path);

/*
 * Attempts to open the local Desmos bridge page in the default browser.
 * Returns true if an opener command succeeded, false otherwise.
 */
bool open_desmos_bridge_page(const std::string& bridge_path, bool autoload_latest = false);

/*
 * Attempts to open the dedicated local Delaunay/Voronoi SVG viewer.
 */
bool open_delaunay_viewer_page(const std::string& viewer_path, bool autoload_latest = false);

/*
 * Returns true if p is inside t; false otherwise
 */
bool is_inside(Point p, Triangle t);

/*
 * Returns true if p is inside the triangle formed by a, b, and c (without actually wasting time
 * constructing the triangle)
 */
bool is_inside(Point p, Point a, Point b, Point c);

/*
 * Checks if the line segment btwn one pair of points collides with that between another pair 
 */
bool collides(std::pair<Point,Point> pair1, std::pair<Point,Point> pair2);

/* Does not count collision at endpoint */ 
bool strict_collides(std::pair<Point,Point> pair1, std::pair<Point,Point> pair2);

#endif
