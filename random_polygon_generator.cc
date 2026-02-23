//implementation file for random_polygon_generator.h
#include "random_polygon_generator.h"
#include "point.h"
#include "edge.h"
#include "triangle.h"
#include "line.h"
#include "logger.h"
#include "pointset.h"
#include "helper.h"

using namespace std;

Polygon generate_random_polygon(int n){
	//need to generate n vertices sequentially. first 3 are given, then for each one after, you have to check and make sure the newly created edge would not intersect any
	//existing edges. Could do it like a vision problem; eg. each say you're at vertex v_i, i>4. You basically can set the new vertex v_i+1 anywhere that v_i can "see"
		

	return Polygon();
}
