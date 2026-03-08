// Implementation file for Delaunay triangulation support.

#include "delaunay.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <map>
#include <vector>

using namespace std;

namespace {

constexpr double kGeomEpsilon = 1e-9;

struct EdgeKey {
	int u = -1;
	int v = -1;

	bool operator<(const EdgeKey& other) const {
		if(u != other.u) return u < other.u;
		return v < other.v;
	}
};

struct BoundaryEdgeRecord {
	int from = -1;
	int to = -1;
	int count = 0;
};

double squared_distance(const Point& a, const Point& b){
	double dx = a.get_x() - b.get_x();
	double dy = a.get_y() - b.get_y();
	return dx * dx + dy * dy;
}

EdgeKey make_edge_key(int a, int b){
	if(a < b) return EdgeKey{a, b};
	return EdgeKey{b, a};
}

bool same_point_exact(const Point& lhs, const Point& rhs){
	return lhs.get_x() == rhs.get_x() && lhs.get_y() == rhs.get_y();
}

bool indices_are_valid(const DelaunayTriangulation& triangulation, const IndexedTriangle& triangle){
	int site_count = static_cast<int>(triangulation.sites.size());
	if(triangle.a < 0 || triangle.b < 0 || triangle.c < 0) return false;
	if(triangle.a >= site_count || triangle.b >= site_count || triangle.c >= site_count) return false;
	if(triangle.a == triangle.b || triangle.b == triangle.c || triangle.a == triangle.c) return false;
	return true;
}

bool is_all_collinear(const vector<Point>& points){
	if(points.size() < 3) return true;
	const Point& base = points.front();
	const Point& second = points.at(1);
	for(size_t i = 2; i < points.size(); ++i){
		if(std::fabs(orient2d(base, second, points.at(i))) > kGeomEpsilon){
			return false;
		}
	}
	return true;
}

IndexedTriangle orient_triangle_ccw(int a, int b, int c, const vector<Point>& sites){
	if(orient2d(sites.at(a), sites.at(b), sites.at(c)) >= 0.0){
		return IndexedTriangle{a, b, c};
	}
	return IndexedTriangle{b, a, c};
}

vector<array<int, 3>> build_neighbors(const vector<IndexedTriangle>& triangles){
	vector<array<int, 3>> neighbors(triangles.size(), array<int, 3>{-1, -1, -1});
	map<EdgeKey, pair<int, int>> owners;

	for(size_t i = 0; i < triangles.size(); ++i){
		const IndexedTriangle& triangle = triangles.at(i);
		const pair<int, int> edges[3] = {
			{triangle.a, triangle.b},
			{triangle.b, triangle.c},
			{triangle.c, triangle.a}
		};

		for(int slot = 0; slot < 3; ++slot){
			EdgeKey key = make_edge_key(edges[slot].first, edges[slot].second);
			auto found = owners.find(key);
			if(found == owners.end()){
				owners.insert({key, {static_cast<int>(i), slot}});
				continue;
			}

			int other_triangle = found->second.first;
			int other_slot = found->second.second;
			neighbors[i][slot] = other_triangle;
			neighbors[other_triangle][other_slot] = static_cast<int>(i);
		}
	}

	return neighbors;
}

vector<Point> normalize_points(const vector<Point>& points){
	vector<Point> normalized = points;
	sort(normalized.begin(), normalized.end(), point_lexicographic_less);

	vector<Point> unique_points;
	unique_points.reserve(normalized.size());
	for(const Point& point : normalized){
		if(unique_points.empty() || !same_point_exact(unique_points.back(), point)){
			unique_points.push_back(point);
		}
	}
	return unique_points;
}

bool has_non_finite_point(const vector<Point>& points){
	for(const Point& point : points){
		if(!std::isfinite(point.get_x()) || !std::isfinite(point.get_y())){
			return true;
		}
	}
	return false;
}

} // namespace

bool point_lexicographic_less(const Point& lhs, const Point& rhs){
	if(lhs.get_x() != rhs.get_x()) return lhs.get_x() < rhs.get_x();
	return lhs.get_y() < rhs.get_y();
}

double orient2d(const Point& a, const Point& b, const Point& c){
	return (b.get_x() - a.get_x()) * (c.get_y() - a.get_y()) -
	       (b.get_y() - a.get_y()) * (c.get_x() - a.get_x());
}

Point circumcenter(const Point& a, const Point& b, const Point& c){
	double ax = a.get_x();
	double ay = a.get_y();
	double bx = b.get_x();
	double by = b.get_y();
	double cx = c.get_x();
	double cy = c.get_y();

	double denominator = 2.0 * (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by));
	if(std::fabs(denominator) <= kGeomEpsilon){
		double nan = std::numeric_limits<double>::quiet_NaN();
		return Point(nan, nan);
	}

	double a_sq = ax * ax + ay * ay;
	double b_sq = bx * bx + by * by;
	double c_sq = cx * cx + cy * cy;

	double ux = (a_sq * (by - cy) + b_sq * (cy - ay) + c_sq * (ay - by)) / denominator;
	double uy = (a_sq * (cx - bx) + b_sq * (ax - cx) + c_sq * (bx - ax)) / denominator;

	return Point(ux, uy);
}

bool circumcircle_contains(const Point& query, const Point& a, const Point& b, const Point& c){
	Point center = circumcenter(a, b, c);
	if(!std::isfinite(center.get_x()) || !std::isfinite(center.get_y())){
		return false;
	}

	double radius2 = squared_distance(center, a);
	double query_distance2 = squared_distance(center, query);
	double scale = std::max({1.0, radius2, query_distance2});
	double eps = kGeomEpsilon * scale;
	return query_distance2 < (radius2 - eps);
}

Triangle materialize_triangle(const DelaunayTriangulation& triangulation,
	                      const IndexedTriangle& indexed_triangle){
	if(!indices_are_valid(triangulation, indexed_triangle)){
		return Triangle();
	}
	return Triangle(triangulation.sites.at(indexed_triangle.a),
	                triangulation.sites.at(indexed_triangle.b),
	                triangulation.sites.at(indexed_triangle.c));
}

bool is_delaunay(const DelaunayTriangulation& triangulation){
	if(triangulation.sites.size() < 3 || triangulation.triangles.empty()){
		return false;
	}

	for(const IndexedTriangle& triangle : triangulation.triangles){
		if(!indices_are_valid(triangulation, triangle)){
			return false;
		}

		const Point& a = triangulation.sites.at(triangle.a);
		const Point& b = triangulation.sites.at(triangle.b);
		const Point& c = triangulation.sites.at(triangle.c);
		if(std::fabs(orient2d(a, b, c)) <= kGeomEpsilon){
			return false;
		}

		for(size_t site_index = 0; site_index < triangulation.sites.size(); ++site_index){
			if(static_cast<int>(site_index) == triangle.a ||
			   static_cast<int>(site_index) == triangle.b ||
			   static_cast<int>(site_index) == triangle.c){
				continue;
			}
			if(circumcircle_contains(triangulation.sites.at(site_index), a, b, c)){
				return false;
			}
		}
	}

	return true;
}

DelaunayTriangulation bowyer_watson_triangulate(const vector<Point>& points){
	DelaunayTriangulation result;

	if(points.size() < 3){
		cout << "Delaunay triangulation requires at least 3 points.\n";
		return result;
	}
	if(has_non_finite_point(points)){
		cout << "Delaunay triangulation requires finite point coordinates.\n";
		return result;
	}

	vector<Point> normalized_points = normalize_points(points);
	if(normalized_points.size() != points.size()){
		cout << "Delaunay triangulation does not support duplicate points.\n";
		return result;
	}
	if(normalized_points.size() < 3){
		cout << "Delaunay triangulation requires at least 3 distinct points.\n";
		return result;
	}
	if(is_all_collinear(normalized_points)){
		cout << "Delaunay triangulation requires a non-collinear point set.\n";
		return result;
	}

	double min_x = normalized_points.front().get_x();
	double max_x = normalized_points.front().get_x();
	double min_y = normalized_points.front().get_y();
	double max_y = normalized_points.front().get_y();
	for(const Point& point : normalized_points){
		min_x = std::min(min_x, point.get_x());
		max_x = std::max(max_x, point.get_x());
		min_y = std::min(min_y, point.get_y());
		max_y = std::max(max_y, point.get_y());
	}

	double span = std::max(max_x - min_x, max_y - min_y);
	if(span <= 0.0) span = 1.0;
	double mid_x = (min_x + max_x) / 2.0;
	double mid_y = (min_y + max_y) / 2.0;
	double super_scale = span * 32.0 + 1.0;

	vector<Point> working_sites = normalized_points;
	int super_a = static_cast<int>(working_sites.size());
	working_sites.push_back(Point(mid_x - 2.0 * super_scale, mid_y - super_scale));
	int super_b = static_cast<int>(working_sites.size());
	working_sites.push_back(Point(mid_x, mid_y + 2.0 * super_scale));
	int super_c = static_cast<int>(working_sites.size());
	working_sites.push_back(Point(mid_x + 2.0 * super_scale, mid_y - super_scale));

	vector<IndexedTriangle> working_triangles{
		orient_triangle_ccw(super_a, super_b, super_c, working_sites)
	};

	for(size_t site_index = 0; site_index < normalized_points.size(); ++site_index){
		vector<bool> bad_flags(working_triangles.size(), false);
		map<EdgeKey, BoundaryEdgeRecord> boundary_edges;
		int bad_count = 0;

		for(size_t triangle_index = 0; triangle_index < working_triangles.size(); ++triangle_index){
			const IndexedTriangle& triangle = working_triangles.at(triangle_index);
			if(!circumcircle_contains(working_sites.at(site_index),
			                          working_sites.at(triangle.a),
			                          working_sites.at(triangle.b),
			                          working_sites.at(triangle.c))){
				continue;
			}

			bad_flags[triangle_index] = true;
			++bad_count;

			const pair<int, int> edges[3] = {
				{triangle.a, triangle.b},
				{triangle.b, triangle.c},
				{triangle.c, triangle.a}
			};
			for(const auto& edge : edges){
				EdgeKey key = make_edge_key(edge.first, edge.second);
				BoundaryEdgeRecord& record = boundary_edges[key];
				if(record.count == 0){
					record.from = edge.first;
					record.to = edge.second;
				}
				record.count++;
			}
		}

		if(bad_count == 0){
			cout << "Delaunay triangulation failed to find a containing cavity for point "
			     << working_sites.at(site_index).to_string() << ".\n";
			return DelaunayTriangulation{};
		}

		vector<IndexedTriangle> next_triangles;
		next_triangles.reserve(working_triangles.size() + boundary_edges.size());
		for(size_t triangle_index = 0; triangle_index < working_triangles.size(); ++triangle_index){
			if(!bad_flags.at(triangle_index)){
				next_triangles.push_back(working_triangles.at(triangle_index));
			}
		}

		for(const auto& entry : boundary_edges){
			const BoundaryEdgeRecord& record = entry.second;
			if(record.count != 1) continue;

			IndexedTriangle triangle = orient_triangle_ccw(record.from,
			                                              record.to,
			                                              static_cast<int>(site_index),
			                                              working_sites);
			if(std::fabs(orient2d(working_sites.at(triangle.a),
			                      working_sites.at(triangle.b),
			                      working_sites.at(triangle.c))) <= kGeomEpsilon){
				continue;
			}
			next_triangles.push_back(triangle);
		}

		working_triangles = std::move(next_triangles);
	}

	result.sites = normalized_points;
	for(const IndexedTriangle& triangle : working_triangles){
		if(triangle.a >= static_cast<int>(normalized_points.size()) ||
		   triangle.b >= static_cast<int>(normalized_points.size()) ||
		   triangle.c >= static_cast<int>(normalized_points.size())){
			continue;
		}
		result.triangles.push_back(triangle);
	}
	result.neighbors = build_neighbors(result.triangles);
	return result;
}
