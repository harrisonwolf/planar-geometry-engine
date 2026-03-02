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
	const int LOOKAHEAD_DEPTH = n/2;
	DBG("Entered generate_random_polygon function\n");
	if(n<3){
		throw invalid_argument("Called generate_random_polygon with n less than 3.");
	}
	random_device rd;
	mt19937 gen(rd());
	uniform_real_distribution<double> unif(-100,100);
	list<Point> points;
	unordered_set<double> xvals;
	double rand_x = 0;
	double rand_y = 0;
	rand_x = unif(gen);
	rand_y = unif(gen);
	points.push_back(Point(rand_x,rand_y));
	xvals.insert(rand_x);
	DBG_TAG("poly.rand_gen.outer","Points list and xvals set initialized, first point added. \nFirst point: " << points.front().to_string() << "\n");
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
	DBG_TAG("poly.rand_gen.outer","First 3 points: \n");
	for(auto it=points.begin(); it!=points.end(); it++){
		DBG_TAG("poly.rand_gen.outer",(*it).to_string() << "\n");
	}

	DBG_TAG("poly.rand_gen", "About to begin random point generation outer loop.\n");
	for(int i=3; i<n-1; i++){
		DBG_TAG("poly.rand_gen", "Entered loop with i = " << i << ".\n");
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

			DBG_TAG("easy.poly.rand_gen", ANSI::GREEN << "Accepted vertex " << potential_new.to_string() << " after successful " << LOOKAHEAD_DEPTH << "-step lookahead.\n" << ANSI::RESET);
			points.push_back(potential_new);
			xvals.insert(rand_x);
			committed = true;
			break;
		}
		if(!committed){
			throw runtime_error("generate_random_polygon could not commit an interior vertex after lookahead-guided search");
		}
	}

	pair<Point,Point> potential_new_pair_1 = {Point(0,0),Point(0,0)};
	pair<Point,Point> potential_new_pair_2 = {Point(0,0),Point(0,0)};
	Point first = points.front();
	Point potential_new(0,0);
	Point last(points.back());
	pair<Point,Point> iterating_pair{{0,0},{0,0}};
	Point curr1(0,0);
	Point curr2(0,0);
	auto it = points.begin();
	int last_vertex_gen_attempts = 0;
	DBG_TAG("easy.poly.rand_gen",ANSI::CYAN << "About to begin last vertex gen.\n" << ANSI::RESET);
	while(1){
while_start:
		DBG_TAG("poly.rand_gen","Generating LAST pot new vertex\n");
		if(last_vertex_gen_attempts%50000 == 0){
			DBG_TAG("easy.poly.rand_gen","Gen attempt " << last_vertex_gen_attempts << "\n");
		}
		rand_x = unif(gen);
		rand_y = unif(gen);
		if(has_duplicate_x(xvals, rand_x) || has_duplicate_point(points, Point(rand_x,rand_y))){
			goto while_start;
		}
		potential_new.set_x(rand_x);
		potential_new.set_y(rand_y);
		last = points.back();
		potential_new_pair_1 = {last,potential_new};
		potential_new_pair_2 = {first,potential_new};
		it = points.begin();

		for(int j=0; j<n-2; j++){
			curr1 = *it;
			it++;
			curr2 = *it;
			iterating_pair = {curr1,curr2};
			if(j == 0 or j == n-3){
				if(strict_collides(iterating_pair,potential_new_pair_1) or strict_collides(iterating_pair,potential_new_pair_2)){
					last_vertex_gen_attempts++;
					it--;
					goto while_start;
				}
			}else{
				if(collides(iterating_pair,potential_new_pair_1) or collides(iterating_pair,potential_new_pair_2)){
					last_vertex_gen_attempts++;
					it--;
					goto while_start;
				}
			}
		}
		points.push_back(potential_new);
		xvals.insert(rand_x);
		break;
	}
	DBG_TAG("poly.rand_gen",ANSI::BOLD_GREEN << "Successfully finished gen function!\n" << ANSI::RESET);
	if(!validate_generated_vertex_order(points)){
		DBG_TAG("poly.rand_gen", ANSI::RED << "Post-generation validation failed: polygon edges self-intersect.\n" << ANSI::RESET);
	}

	DBG_TAG("poly.rand_gen", "In rand poly generator, about to attempt initialization of polygon with rand gen points.\n");
	return Polygon(points);
}
