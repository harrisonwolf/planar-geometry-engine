#ifndef VORONOI_H
#define VORONOI_H

#include <vector>

#include "delaunay.h"

struct VoronoiEdge {
	int start_vertex = -1;
	int end_vertex = -1;
	int left_site = -1;
	int right_site = -1;
};

struct VoronoiDiagram {
	std::vector<Point> sites;
	std::vector<Point> vertices;
	std::vector<VoronoiEdge> edges;
};

VoronoiDiagram build_voronoi_diagram(const DelaunayTriangulation& triangulation);

#endif
