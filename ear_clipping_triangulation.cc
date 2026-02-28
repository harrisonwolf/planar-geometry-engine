//source code for an algorithm that triangulates a given polygon p using ear_clipping
#include "ear_clipping_triangulation.h"
#include "logger.h"
#include "die.h"
#include "point.h"
#include <list>
#include <iostream>
#include <vector>
#include <cmath>
//
using namespace std;

namespace {

double signed_area(const vector<Point>& vertices){
	double area2 = 0.0;
	if(vertices.size() < 3) return 0.0;
	for(size_t i = 0; i < vertices.size(); ++i){
		const Point& a = vertices[i];
		const Point& b = vertices[(i + 1) % vertices.size()];
		area2 += (a.get_x() * b.get_y()) - (b.get_x() * a.get_y());
	}
	return 0.5 * area2;
}

double orient2d(const Point& a, const Point& b, const Point& c){
	return (b.get_x() - a.get_x()) * (c.get_y() - a.get_y()) -
	       (b.get_y() - a.get_y()) * (c.get_x() - a.get_x());
}

bool point_in_triangle(const Point& p, const Point& a, const Point& b, const Point& c){
	const double eps = 1e-10;
	double o1 = orient2d(a, b, p);
	double o2 = orient2d(b, c, p);
	double o3 = orient2d(c, a, p);

	bool has_neg = (o1 < -eps) || (o2 < -eps) || (o3 < -eps);
	bool has_pos = (o1 > eps) || (o2 > eps) || (o3 > eps);
	return !(has_neg && has_pos);
}

bool is_convex_vertex(const Point& prev_p, const Point& p, const Point& next_p, bool is_ccw){
	double turn = orient2d(prev_p, p, next_p);
	const double eps = 1e-10;
	if(is_ccw) return turn > eps;
	return turn < -eps;
}

} // namespace

vector<Triangle> triangulate(Polygon p){
	DBG("Calling initial triangulate.\n");
	vector<Triangle> clipped_ears;
	return triangulate(p,clipped_ears);
}

vector<Triangle> triangulate(Polygon curr_polygon, vector<Triangle> &current_ears){ 
	list<Point> curr_vertices_list = curr_polygon.get_vertex_list();
	vector<Point> vertices(curr_vertices_list.begin(), curr_vertices_list.end());

	DBG("Starting ear clipping on polygon of " << vertices.size() << " vertices.\n");

	if(vertices.size() < 3){
		cout << "Cannot triangulate polygon with fewer than 3 vertices.\n";
		die();
	}

	while(vertices.size() > 3){
		bool clipped_any_ear = false;
		bool is_ccw = signed_area(vertices) > 0.0;

		for(size_t i = 0; i < vertices.size(); ++i){
			size_t prev_i = (i + vertices.size() - 1) % vertices.size();
			size_t next_i = (i + 1) % vertices.size();

			const Point& prev_p = vertices[prev_i];
			const Point& p = vertices[i];
			const Point& next_p = vertices[next_i];

			if(!is_convex_vertex(prev_p, p, next_p, is_ccw)){
				continue;
			}

			bool any_inside = false;
			for(size_t j = 0; j < vertices.size(); ++j){
				if(j == prev_i || j == i || j == next_i) continue;
				if(point_in_triangle(vertices[j], prev_p, p, next_p)){
					any_inside = true;
					break;
				}
			}

			if(any_inside) continue;

			current_ears.push_back(Triangle(prev_p, p, next_p));
			vertices.erase(vertices.begin() + static_cast<long>(i));
			clipped_any_ear = true;
			break;
		}

		if(!clipped_any_ear){
			cout << "Failed to find an ear during ear clipping triangulation.\n";
			die();
		}
	}

	current_ears.push_back(Triangle(vertices[0], vertices[1], vertices[2]));
	return current_ears;
}
