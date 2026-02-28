//implementation file for random_polygon_generator.h
#include "random_polygon_generator.h"
#include "logger.h"
#include <random>
#include <vector>
#include <cmath>

using namespace std;

namespace {

// Methodology:
// - Preserve original sequential generator style: vertices are appended one-by-one.
// - For each new candidate, validate uniqueness and reject edge intersections.
// - The final vertex is handled specially because it creates two new edges
//   (to the previous vertex and back to the first vertex).
// - After construction, perform a full simplicity check as a final guard.
//
// This keeps the same algorithmic shape the project started with, while fixing
// intersection logic and endpoint-adjacency handling bugs.

constexpr double kDefaultBound = 100.0;
constexpr double kEps = 1e-9;
constexpr int kMaxVertexAttempts = 20000;
constexpr int kMaxWholePolygonAttempts = 200;

struct Segment {
	Point a;
	Point b;
};

bool almost_equal(double a, double b){
	return fabs(a - b) < kEps;
}

bool same_point(const Point& a, const Point& b){
	return almost_equal(a.get_x(), b.get_x()) && almost_equal(a.get_y(), b.get_y());
}

double orient2d(const Point& a, const Point& b, const Point& c){
	return (b.get_x() - a.get_x()) * (c.get_y() - a.get_y()) -
	       (b.get_y() - a.get_y()) * (c.get_x() - a.get_x());
}

// Collinear point-on-segment test with epsilon bounds.
bool on_segment(const Point& a, const Point& b, const Point& p){
	if(fabs(orient2d(a, b, p)) > kEps) return false;
	double min_x = min(a.get_x(), b.get_x()) - kEps;
	double max_x = max(a.get_x(), b.get_x()) + kEps;
	double min_y = min(a.get_y(), b.get_y()) - kEps;
	double max_y = max(a.get_y(), b.get_y()) + kEps;
	return p.get_x() >= min_x && p.get_x() <= max_x && p.get_y() >= min_y && p.get_y() <= max_y;
}

// Robust segment intersection: proper crossing + collinear overlap cases.
bool segments_intersect(const Point& p1, const Point& p2, const Point& q1, const Point& q2){
	double o1 = orient2d(p1, p2, q1);
	double o2 = orient2d(p1, p2, q2);
	double o3 = orient2d(q1, q2, p1);
	double o4 = orient2d(q1, q2, p2);

	if(((o1 > kEps && o2 < -kEps) || (o1 < -kEps && o2 > kEps)) &&
	   ((o3 > kEps && o4 < -kEps) || (o3 < -kEps && o4 > kEps))){
		return true;
	}

	if(fabs(o1) <= kEps && on_segment(p1, p2, q1)) return true;
	if(fabs(o2) <= kEps && on_segment(p1, p2, q2)) return true;
	if(fabs(o3) <= kEps && on_segment(q1, q2, p1)) return true;
	if(fabs(o4) <= kEps && on_segment(q1, q2, p2)) return true;
	return false;
}

// Allow only one intentional shared endpoint (adjacent edges), reject all others.
bool intersects_disallowing_shared_endpoint(const Segment& s1, const Segment& s2, const Point& allowed_shared){
	if(!segments_intersect(s1.a, s1.b, s2.a, s2.b)) return false;

	// If intersection is only at the allowed shared endpoint, that's OK.
	bool s1_has_allowed = same_point(s1.a, allowed_shared) || same_point(s1.b, allowed_shared);
	bool s2_has_allowed = same_point(s2.a, allowed_shared) || same_point(s2.b, allowed_shared);
	if(s1_has_allowed && s2_has_allowed){
		Point other1 = same_point(s1.a, allowed_shared) ? s1.b : s1.a;
		Point other2 = same_point(s2.a, allowed_shared) ? s2.b : s2.a;
		if(!same_point(other1, other2)) return false;
	}

	return true;
}

bool point_is_usable(const Point& p, const vector<Point>& points){
	for(const Point& existing: points){
		if(same_point(existing, p)) return false;
		if(almost_equal(existing.get_x(), p.get_x())) return false; // avoid vertical-line degeneracy in this codebase
	}
	return true;
}

bool polygon_is_simple(const vector<Point>& points){
	if(points.size() < 3) return false;
	int n = static_cast<int>(points.size());
	for(int i = 0; i < n; ++i){
		Point a1 = points.at(i);
		Point a2 = points.at((i + 1) % n);
		for(int j = i + 1; j < n; ++j){
			if(j == i) continue;
			if((j + 1) % n == i) continue;
			if((i + 1) % n == j) continue;
			Point b1 = points.at(j);
			Point b2 = points.at((j + 1) % n);
			if(segments_intersect(a1, a2, b1, b2)) return false;
		}
	}
	return true;
}

Polygon generate_random_polygon_with_bounds(int n, double min_coord, double max_coord){
	if(n < 3){
		cout << "Error: Called generate_random_polygon with n less than 3. Exiting.\n";
		exit(1);
	}
	if(min_coord >= max_coord){
		cout << "Error: invalid coordinate bounds for random polygon generator.\n";
		exit(1);
	}

	random_device rd;
	mt19937 gen(rd());
	uniform_real_distribution<double> unif(min_coord, max_coord);

	for(int polygon_attempt = 0; polygon_attempt < kMaxWholePolygonAttempts; ++polygon_attempt){
		vector<Point> points;
		points.reserve(n);

		// First 3 points
		while(points.size() < 3){
			Point candidate(unif(gen), unif(gen));
			if(!point_is_usable(candidate, points)) continue;
			points.push_back(candidate);
			if(points.size() == 3 && fabs(orient2d(points.at(0), points.at(1), points.at(2))) <= kEps){
				points.pop_back();
			}
		}

		bool failed = false;

		// Sequentially generate interior chain vertices 4..(n-1).
		// Each new edge must avoid all existing non-adjacent edges.
		for(int i = 3; i < n - 1; ++i){
			bool placed = false;
			for(int attempts = 0; attempts < kMaxVertexAttempts; ++attempts){
				Point candidate(unif(gen), unif(gen));
				if(!point_is_usable(candidate, points)) continue;

				Segment candidate_edge{points.back(), candidate};
				bool collides = false;
				for(size_t e = 0; e + 1 < points.size(); ++e){
					Segment existing{points.at(e), points.at(e + 1)};
					if(intersects_disallowing_shared_endpoint(candidate_edge, existing, points.back())){
						collides = true;
						break;
					}
				}
				if(collides) continue;
				points.push_back(candidate);
				placed = true;
				break;
			}
			if(!placed){
				failed = true;
				break;
			}
		}

		if(failed) continue;

		// Final vertex closes the polygon and therefore creates two edges.
		// Both must be collision-free against the existing chain.
		bool placed_last = false;
		for(int attempts = 0; attempts < kMaxVertexAttempts; ++attempts){
			Point candidate(unif(gen), unif(gen));
			if(!point_is_usable(candidate, points)) continue;

			Segment edge_to_last{points.back(), candidate};
			Segment edge_to_first{candidate, points.front()};
			bool collides = false;

			for(size_t e = 0; e + 1 < points.size(); ++e){
				Segment existing{points.at(e), points.at(e + 1)};
				if(intersects_disallowing_shared_endpoint(edge_to_last, existing, points.back())){
					collides = true;
					break;
				}
				if(intersects_disallowing_shared_endpoint(edge_to_first, existing, points.front())){
					collides = true;
					break;
				}
			}
			if(collides) continue;

			if(segments_intersect(edge_to_last.a, edge_to_last.b, edge_to_first.a, edge_to_first.b)){
				// These share candidate endpoint; anything beyond that means degenerate overlap.
				if(on_segment(edge_to_last.a, edge_to_last.b, edge_to_first.b) ||
				   on_segment(edge_to_first.a, edge_to_first.b, edge_to_last.a)){
					continue;
				}
			}

			points.push_back(candidate);
			placed_last = true;
			break;
		}

		if(!placed_last) continue;
		if(!polygon_is_simple(points)) continue;

		list<Point> point_list(points.begin(), points.end());
		return Polygon(point_list);
	}

	cout << "Error: Failed to generate a valid random polygon after many attempts.\n";
	exit(1);
}

} // namespace

Polygon generate_random_polygon(){
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<int> v_dist(4, 99);
	return generate_random_polygon(v_dist(gen));
}

Polygon generate_random_polygon(int n){
	return generate_random_polygon_with_bounds(n, -kDefaultBound, kDefaultBound);
}

Polygon generate_random_polygon(int n, double c){
	if(c <= 0){
		cout << "Error: coordinate bound must be positive.\n";
		exit(1);
	}
	return generate_random_polygon_with_bounds(n, -c, c);
}

Polygon generate_random_polygon(int n, double c1, double c2){
	return generate_random_polygon_with_bounds(n, c1, c2);
}
