//source code for an algorithm that triangulates a given polygon p using ear_clipping
#include "ear_clipping_triangulation.h"
#include "logger.h"
#include "die.h"
#include "point.h"
#include <list>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace std;

namespace {

constexpr double kEps = 1e-12;

double signed_area2(const vector<Point>& vertices){
	double sum = 0.0;
	if(vertices.size() < 3) return 0.0;
	for(size_t i = 0; i < vertices.size(); ++i){
		const Point& a = vertices[i];
		const Point& b = vertices[(i + 1) % vertices.size()];
		sum += a.get_x() * b.get_y() - b.get_x() * a.get_y();
	}
	return sum;
}

bool is_convex_vertex(const Point& prev, const Point& curr, const Point& next, bool ccw){
	const double turn = cross(curr - prev, next - curr);
	if(std::fabs(turn) < kEps) return false;
	return ccw ? (turn > 0.0) : (turn < 0.0);
}

bool point_in_triangle_strict(const Point& p, const Point& a, const Point& b, const Point& c){
	const double c1 = cross(b - a, p - a);
	const double c2 = cross(c - b, p - b);
	const double c3 = cross(a - c, p - c);
	const bool has_neg = (c1 < -kEps) || (c2 < -kEps) || (c3 < -kEps);
	const bool has_pos = (c1 > kEps) || (c2 > kEps) || (c3 > kEps);
	if(has_neg && has_pos) return false;
	return !(std::fabs(c1) <= kEps || std::fabs(c2) <= kEps || std::fabs(c3) <= kEps);
}

bool is_ear(const vector<Point>& poly, size_t i, bool ccw){
	const size_t n = poly.size();
	const size_t ip = (i + n - 1) % n;
	const size_t in = (i + 1) % n;
	const Point& prev = poly[ip];
	const Point& curr = poly[i];
	const Point& next = poly[in];

	if(!is_convex_vertex(prev, curr, next, ccw)) return false;

	for(size_t j = 0; j < n; ++j){
		if(j == ip || j == i || j == in) continue;
		if(point_in_triangle_strict(poly[j], prev, curr, next)) return false;
	}
	return true;
}

} // namespace

vector<Triangle> triangulate(Polygon p){
	DBG("Calling initial triangulate.\n");
	vector<Triangle> clipped_ears;
	return triangulate(std::move(p),clipped_ears);
}

vector<Triangle> triangulate(Polygon curr_polygon, vector<Triangle> &current_ears){
	const list<Point>& curr_list = curr_polygon.get_vertex_list();
	vector<Point> vertices(curr_list.begin(), curr_list.end());
	if(vertices.size() < 3){
		cout << "Triangulation error: polygon has fewer than 3 vertices.\n";
		return current_ears;
	}

	const bool ccw = signed_area2(vertices) > 0.0;
	while(vertices.size() > 3){
		bool clipped = false;
		for(size_t i = 0; i < vertices.size(); ++i){
			if(!is_ear(vertices, i, ccw)) continue;
			const size_t ip = (i + vertices.size() - 1) % vertices.size();
			const size_t in = (i + 1) % vertices.size();
			current_ears.emplace_back(vertices[ip], vertices[i], vertices[in]);
			vertices.erase(vertices.begin() + static_cast<long>(i));
			clipped = true;
			break;
		}

		if(!clipped){
			cout << "Never recursed in the ear clipping triangulation... Something's gone wrong.\n";
			return current_ears;
		}
	}

	current_ears.emplace_back(vertices[0], vertices[1], vertices[2]);
	return current_ears;
}
