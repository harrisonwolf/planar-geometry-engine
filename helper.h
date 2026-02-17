//header file for helper functions mainly dealing with taking as input points, lines, triangles, etc

#ifndef HELPER_H
#define HELPER_H

#include <iostream>
#include <vector>
#include "point.h"
#include "triangle.h"
//#include "polygon.h"
#include "polygon_new.h"
#include "line.h"

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
 * Returns true if p is inside t; false otherwise
 */
bool is_inside(Point p, Triangle t);

/*
 * Returns true if p is inside the triangle formed by a, b, and c (without actually wasting time
 * constructing the triangle)
 */
bool is_inside(Point p, Point a, Point b, Point c);

#endif
