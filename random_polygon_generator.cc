//implementation file for random_polygon_generator.h
//suppose you are at a vertex v_i in the sequential generation. the "line of sight" is somewhat the same as the prev one but changes slightly with each new vertex added. so I could keep some variable or datum about "curr line of sight/vision area" and update it after each new vertex is added. question is how to actually implement this
//suppose first points are eg. (6,1), (2,-4), (-3,-3), (-1,-6)
//TODO: I guess I first need to do a "line of sight" computation alg. Either within this file as a helper function or in general... but in general sounds pretty involved
//could also just gen points randomly, each time checking if the newly generated line would intersect any existing lines, but that would get runtime intensive pretyy fast. but probably only for polygons of like crazy numbers of vertices
#include "random_polygon_generator.h"
#include "point.h"
#include "edge.h"
#include "triangle.h"
#include "line.h"
#include "logger.h"
//#include "pointset.h"
#include "helper.h"
#include <random>
#include <vector>
#include <list>
#include <unordered_set>
#include <stdexcept>
#include <cmath>

using namespace std;

namespace {

constexpr int LOOKAHEAD_TRIES_PER_LEVEL = 500;
constexpr int MAX_COMMIT_ATTEMPTS_PER_VERTEX = 10000;
constexpr int MAX_LAST_VERTEX_ATTEMPTS = 250000;
constexpr int MAX_POLYGON_RESTARTS = 200;

bool has_duplicate_point(const list<Point>& points, const Point& candidate){
	for(const Point& p: points){
		if(p == candidate) return true;
	}
	return false;
}

bool has_duplicate_x(const unordered_set<double>& xvals, double x){
	return xvals.count(x) > 0;
}

bool append_would_collide(const list<Point>& points, const Point& candidate){
	if(points.size() < 2) return false;
	Point last = points.back();
	pair<Point,Point> potential_new_pair{last, candidate};
	auto it = points.begin();
	int limit = static_cast<int>(points.size()) - 2;
	for(int j = 0; j < limit; ++j){
		Point curr1 = *it;
		++it;
		Point curr2 = *it;
		pair<Point,Point> iterating_pair{curr1,curr2};
		if(collides(iterating_pair,potential_new_pair)) return true;
	}
	return false;
}

bool can_find_lookahead_chain(list<Point> points,
			      unordered_set<double> xvals,
			      int depth,
			      mt19937& gen,
			      uniform_real_distribution<double>& unif){
	if(depth <= 0) return true;
	for(int attempt = 0; attempt < LOOKAHEAD_TRIES_PER_LEVEL; ++attempt){
		double x = unif(gen);
		double y = unif(gen);
		Point candidate(x,y);
		if(has_duplicate_x(xvals, x) || has_duplicate_point(points, candidate)) continue;
		if(append_would_collide(points, candidate)) continue;
		list<Point> temp_points = points;
		unordered_set<double> temp_xvals = xvals;
		temp_points.push_back(candidate);
		temp_xvals.insert(x);
		if(can_find_lookahead_chain(temp_points, temp_xvals, depth-1, gen, unif)) return true;
	}
	return false;
}

bool validate_generated_vertex_order(const list<Point>& points){
	if(points.size() < 3) return false;
	vector<Point> pts(points.begin(), points.end());
	int n = static_cast<int>(pts.size());
	for(int i = 0; i < n; ++i){
		pair<Point,Point> edge1{pts[i], pts[(i+1)%n]};
		for(int j = i + 1; j < n; ++j){
			if(abs(i-j) <= 1 || (i == 0 && j == n-1)) continue;
			pair<Point,Point> edge2{pts[j], pts[(j+1)%n]};
			if(collides(edge1, edge2)) return false;
		}
	}
	return true;
}

}

Polygon generate_random_polygon(int n){
	const int LOOKAHEAD_DEPTH = std::max(2, std::min(10, n/4));
	DBG("Entered generate_random_polygon function\n");
	if(n<3){
		throw invalid_argument("Called generate_random_polygon with n less than 3.");
	}
	random_device rd;
	mt19937 gen(rd());
	uniform_real_distribution<double> unif(-100,100);
	for(int restart = 0; restart < MAX_POLYGON_RESTARTS; ++restart){
		list<Point> points;
		unordered_set<double> xvals;
		double rand_x = unif(gen);
		double rand_y = unif(gen);
		points.push_back(Point(rand_x,rand_y));
		xvals.insert(rand_x);

		for(int i=0; i<2; i++){
			rand_x = unif(gen);
			rand_y = unif(gen);
			Point candidate(rand_x, rand_y);
			if(has_duplicate_x(xvals, rand_x) || has_duplicate_point(points, candidate)){
				i--;
				continue;
			}
			points.push_back(candidate);
			xvals.insert(rand_x);
		}

		bool interior_generation_failed = false;
		for(int i=3; i<n-1; i++){
			bool committed = false;
			for(int commit_attempt = 0; commit_attempt < MAX_COMMIT_ATTEMPTS_PER_VERTEX; ++commit_attempt){
				rand_x = unif(gen);
				rand_y = unif(gen);
				Point potential_new(rand_x, rand_y);
				if(has_duplicate_x(xvals, rand_x) || has_duplicate_point(points, potential_new)) continue;
				if(append_would_collide(points, potential_new)) continue;

				list<Point> trial_points = points;
				unordered_set<double> trial_xvals = xvals;
				trial_points.push_back(potential_new);
				trial_xvals.insert(rand_x);

				if(!can_find_lookahead_chain(trial_points, trial_xvals, LOOKAHEAD_DEPTH, gen, unif)) continue;

				points.push_back(potential_new);
				xvals.insert(rand_x);
				committed = true;
				break;
			}
			if(!committed){
				interior_generation_failed = true;
				break;
			}
		}
		if(interior_generation_failed) continue;

		Point first = points.front();
		Point last(points.back());
		bool finalized = false;
		for(int attempt = 0; attempt < MAX_LAST_VERTEX_ATTEMPTS; ++attempt){
			rand_x = unif(gen);
			rand_y = unif(gen);
			Point potential_new(rand_x, rand_y);
			if(has_duplicate_x(xvals, rand_x) || has_duplicate_point(points, potential_new)) continue;

			pair<Point,Point> potential_new_pair_1{last,potential_new};
			pair<Point,Point> potential_new_pair_2{first,potential_new};
			auto it = points.begin();
			bool collides_existing_edge = false;

			for(int j=0; j<n-2; j++){
				Point curr1 = *it;
				it++;
				Point curr2 = *it;
				pair<Point,Point> iterating_pair{curr1,curr2};
				if(j == 0 || j == n-3){
					if(strict_collides(iterating_pair,potential_new_pair_1) || strict_collides(iterating_pair,potential_new_pair_2)){
						collides_existing_edge = true;
						break;
					}
				}else{
					if(collides(iterating_pair,potential_new_pair_1) || collides(iterating_pair,potential_new_pair_2)){
						collides_existing_edge = true;
						break;
					}
				}
			}

			if(collides_existing_edge) continue;

			points.push_back(potential_new);
			xvals.insert(rand_x);
			finalized = true;
			break;
		}

		if(!finalized) continue;
		if(!validate_generated_vertex_order(points)) continue;
		return Polygon(points);
	}

	throw runtime_error("generate_random_polygon exhausted restart budget while searching for a valid polygon");
}
