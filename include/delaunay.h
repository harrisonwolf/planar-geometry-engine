#ifndef DELAUNAY_H
#define DELAUNAY_H

#include <array>
#include <vector>

#include "point.h"
#include "triangle.h"

struct IndexedTriangle {
	int a = -1;
	int b = -1;
	int c = -1;
};

struct DelaunayTriangulation {
	std::vector<Point> sites;
	std::vector<IndexedTriangle> triangles;
	// Neighbor order matches triangle edges (a,b), (b,c), (c,a).
	std::vector<std::array<int, 3>> neighbors;
};

bool point_lexicographic_less(const Point& lhs, const Point& rhs);
double orient2d(const Point& a, const Point& b, const Point& c);
Point circumcenter(const Point& a, const Point& b, const Point& c);
bool circumcircle_contains(const Point& query,
	                   const Point& a,
	                   const Point& b,
	                   const Point& c);
Triangle materialize_triangle(const DelaunayTriangulation& triangulation,
	                      const IndexedTriangle& indexed_triangle);

bool is_delaunay(const DelaunayTriangulation& triangulation);
DelaunayTriangulation bowyer_watson_triangulate(const std::vector<Point>& points);

#endif

