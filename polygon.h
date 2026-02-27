//header file for polygon class which is composed of a set of points
#ifndef POLYGON_H
#define POLYGON_H

#include "point.h"
#include "triangle.h"
#include "die.h"
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <sstream>

class Polygon{
private:
	std::vector<Point> vertices; //used for actually manipulating/using vertices
	std::unordered_map<double,Point> dict;
	std::vector<Triangle> triangulation;
	double area = 0.0;

public:
	/* Constructors */ 
	//Each polygon must have at least 3 vertices
	Polygon();
	Polygon(std::vector<Point> vertices);//constructor which takes a given point set 
	
	/* Accessors */
	std::vector<Point> get_vertices(){ return vertices; }
	double get_area(){ return area; }
	std::vector<Triangle> get_triangulation(){ return triangulation; }

	/*
	 * Actually calculates the area (using triangulation)
	 */
	double calculate_area();
	std::vector<Triangle> triangulate(); //returns a set of triangles that form the 
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
	std::string to_desmos();
	void print_triangulation();
	bool contains(Point p); //returns true if p is inside of (or comprises an edge or vertex of)
				//this->polygon
};

#endif
