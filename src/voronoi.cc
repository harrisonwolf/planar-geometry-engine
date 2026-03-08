#include "voronoi.h"

#include <algorithm>
#include <cmath>
#include <iostream>

using namespace std;

namespace {

bool is_valid_triangle_ref(const DelaunayTriangulation& triangulation, const IndexedTriangle& triangle){
	int site_count = static_cast<int>(triangulation.sites.size());
	if(triangle.a < 0 || triangle.b < 0 || triangle.c < 0) return false;
	if(triangle.a >= site_count || triangle.b >= site_count || triangle.c >= site_count) return false;
	if(triangle.a == triangle.b || triangle.b == triangle.c || triangle.a == triangle.c) return false;
	return true;
}

pair<int, int> edge_for_slot(const IndexedTriangle& triangle, int slot){
	if(slot == 0) return {triangle.a, triangle.b};
	if(slot == 1) return {triangle.b, triangle.c};
	return {triangle.c, triangle.a};
}

} // namespace

VoronoiDiagram build_voronoi_diagram(const DelaunayTriangulation& triangulation){
	VoronoiDiagram diagram;
	diagram.sites = triangulation.sites;

	if(triangulation.triangles.empty()){
		return diagram;
	}
	if(triangulation.neighbors.size() != triangulation.triangles.size()){
		cout << "Voronoi diagram requires triangle neighbor adjacency.\n";
		return VoronoiDiagram{};
	}

	diagram.vertices.reserve(triangulation.triangles.size());
	for(const IndexedTriangle& triangle : triangulation.triangles){
		if(!is_valid_triangle_ref(triangulation, triangle)){
			cout << "Voronoi diagram received an invalid Delaunay triangle.\n";
			return VoronoiDiagram{};
		}

		Point center = circumcenter(triangulation.sites.at(triangle.a),
		                            triangulation.sites.at(triangle.b),
		                            triangulation.sites.at(triangle.c));
		if(!std::isfinite(center.get_x()) || !std::isfinite(center.get_y())){
			cout << "Voronoi diagram encountered a degenerate Delaunay triangle.\n";
			return VoronoiDiagram{};
		}
		diagram.vertices.push_back(center);
	}

	for(size_t triangle_index = 0; triangle_index < triangulation.triangles.size(); ++triangle_index){
		const IndexedTriangle& triangle = triangulation.triangles.at(triangle_index);
		const array<int, 3>& neighbors = triangulation.neighbors.at(triangle_index);
		for(int slot = 0; slot < 3; ++slot){
			int neighbor_index = neighbors[slot];
			if(neighbor_index < 0 || neighbor_index <= static_cast<int>(triangle_index)){
				continue;
			}

			pair<int, int> source_edge = edge_for_slot(triangle, slot);
			int left_site = std::min(source_edge.first, source_edge.second);
			int right_site = std::max(source_edge.first, source_edge.second);
			diagram.edges.push_back(VoronoiEdge{
				static_cast<int>(triangle_index),
				neighbor_index,
				left_site,
				right_site
			});
		}
	}

	return diagram;
}
