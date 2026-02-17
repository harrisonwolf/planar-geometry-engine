//header file for polygon class which is composed of a set of points
//THE polygon is defined by its points, but to really figure out anything about it (eg area,
//we use its triangulation
#ifndef POLYGON_NEW_H
#define POLYGON_NEW_H

#include "point.h"
#include "triangle.h"
#include "die.h"
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <sstream>
#include <list>

class Polygon{
private:
	std::list<Point> vertex_list;
	std::unordered_map<double,Point> dict;
	std::unordered_map<double,Point> reflex_vertices;
	std::unordered_map<double,Point> convex_vertices;
	//need to add to these as I'm constructing the polygon, using the fact that points are given
	//in clockwise order, so the inside of the polygon is always to the "right,"
	//with respect to the movement along the perimeter
	std::vector<Triangle> triangulation;
	double area = 0.0;
	bool convex = true;

public:
	/* Constructors */ 
	//Each polygon must have at least 3 vertices
	Polygon();
	Polygon(std::list<Point> vertices);//constructor which takes a given point list
	
	/* Accessors */
	std::list<Point> get_vertex_list(){ return vertex_list; }
	std::unordered_map<double,Point> get_reflex_vertices(){ return reflex_vertices; }
	std::unordered_map<double,Point> get_convex_vertices(){ return convex_vertices; }
	double get_area(){ return area; }
	std::vector<Triangle> get_triangulation(){ return triangulation; }

	/*
	 * Actually calculates the area (using triangulation)
	 */
	double calculate_area();
	std::vector<Triangle> calculate_triangulation(); //returns a set of triangles that form the 
					     //triangulation of the polygon (may be useful 
					     //for calulating area 


	/* Mutators */ 
	void add_point(Point p); //add a point to the point set (this may be pretty hard to do, 
				 //at least runtime-wise)
	
	/* Miscellaneous */

	/*
	 * Returns string in the following format
	 * Vertices:
	 * {x1,y1}
	 * {x2,y2}
	 * ...
	 * {xn,yn}
	 * Area:
	 * Convex/Concave (TODO)
	 */
	std::string to_string();
	void print_triangulation();
	bool contains(Point p); //returns true if p is inside of (or comprises an edge or vertex of)
				//this->polygon
};

/*
 * Takes a string of 3 vertices in counter-clockwise order of a simple polygon
 * Returns true if curr_v is convex, false if it's reflex
 */
bool is_convex(Point prev_v, Point curr_v, Point next_v);

#endif
