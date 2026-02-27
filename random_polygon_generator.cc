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

using namespace std;

Polygon generate_random_polygon(int n){
	DBG("Entered generate_random_polygon function\n");
	if(n<3){
		cout << "Error: Called generate_random_polygon with n less than 3. Exiting.\n";
		exit(1);
	}	
	//need to generate n vertices sequentially. first 3 are given, then for each one after, you have to check and make sure the newly created edge would not intersect any
	//existing edges. Could do it like a vision problem; eg. each say you're at vertex v_i, i>4. You basically can set the new vertex v_i+1 anywhere that v_i can "see"
	//initialize random engine
	random_device rd; //used to obtain seed for random number engine
	mt19937 gen(rd()); //mersenne twister engine seeded with rd()
	uniform_real_distribution<double> unif(-100,100);
	//initialize container for points
	//want something easy to iterate through/global... b/c when at a vertex v_i, v_i+1 may be generated anywhere that v_i can "see"... So maybe the distribution needs to be updated. I could foresee a scenario in which the next vertex cannot be generated b/c all the current sight areas are outside of the bounds set for the real distribtion
	//I think just a standard list is the best for this
	list<Point> points;
	unordered_set<double> xvals; //quickly check for no duplicate points	
	//add first 3 points
	double rand_x = 0;
	double rand_y = 0;
	rand_x = unif(gen);	
	rand_y = unif(gen);
	points.push_back(Point(rand_x,rand_y));
	xvals.insert(rand_x);
	DBG("Points list and xvals set initialized, first point added. \nFirst point: " << points.front().to_string() << "\n");
	for(int i=0; i<2; i++){
		rand_x = unif(gen);	
		rand_y = unif(gen);
		//check to make sure point doesn't exist
		if(xvals.count(rand_x) > 0){ //try again
			i--;
			continue;
		}
		points.push_back(Point(rand_x,rand_y));
		xvals.insert(rand_x);
	}
	//check contents
	DBG("First 3 points: \n");
	for(auto it=points.begin(); it!=points.end(); it++){
		DBG((*it).to_string() << "\n");
	}
	//so far, so good
	//now onto the hard part
	//now need to start adding vertices up to n, but checking with each one for intersections with existing segments
	//may need to work in edge class which currently is a stub
	//or find another way to do it
	//
	//Could just brute force it and check every other double of consecutive vertices against the double of the last, next potential and check for collisions. Probably pretty
	//easy to implement but could be very slow (probably O(n^3) or higher) but okay for getting gen to work for driving triangulator and delaunay algs
	//not planning on doing anything with huge n as of now
	//
	//First add helper function for checking if line segment between to vertices intersects that between another 2
	//Alg for vertex gen:
	//1. randomly gen x-coord and y-coord
	//2. check if last vertex - new vertex collides with any existing segments
	//	2.1. if so, go back to step 1 (make sure to decrement i)
	//	2.2. if not, add new vertex, continue
	Point potential_new(0,0);
	Point last(points.back());
	pair<Point,Point> iterating_pair{{0,0},{0,0}};
	pair<Point,Point> potential_new_pair{{0,0},{0,0}};
	Point curr1(0,0);
	Point curr2(0,0);
	auto it = points.begin();
	DBG("About to begin random point generation outer loop.\n");
	for(int i=3; i<n; i++){ //start at 3 b/c 3 verts already exist
	DBG("Entered loop with i = " << i << ".\n");
restart_curr_rand_gen:
		rand_x = unif(gen);		
		rand_y = unif(gen);		
		potential_new.set_x(rand_x);
		potential_new.set_y(rand_y);
		last = points.back();
		potential_new_pair = {last,potential_new};
		it = points.begin();
		//got my potential new point, now check for collisions
		DBG("About to begin inner loop for collision checking for pair " << potential_new_pair.first.to_string() << "-" << potential_new_pair.second.to_string() << ".\n");
		for(int j=0; j<i-1; j++){
			DBG("\n\nEntered inner loop with j = " << j << ", i = " << i << ".\n");
			curr1 = *it;
			it++;
			curr2 = *it;
			iterating_pair = {curr1,curr2};
			if(collides(iterating_pair,potential_new_pair)){ //we have a collision
				it--; //reset iterator
					  //i doesn't need to be reset since not using continue statement
				goto restart_curr_rand_gen;
			}
		}//at this point it has been verified no collisions
		 //add the new point, continue outer loop
		points.push_back(potential_new);
	}
	cerr << "In rand poly generator, about to attempt initialization of polygon with rand gen points.\n";
	return Polygon(points); 
}
