//header file for Point class which has a Point object, its distance from the origin
#ifndef POINT_H
#define POINT_H

#include <iostream>
#include <cmath>
#include <sstream>
#include <string>
#include "die.h"

class Point{
private:
	double x = 0, y = 0;
	double distance = 0; //distance from origin

public:
	/* Constructors */
	Point();
	Point(double new_x, double new_y);

	/* Accessors */
	double get_x(){ return x; }
	double get_y(){ return y; }
	double get_distance(){ return distance; }
	double distance_to(Point other);

	/* Mutators */
	void set_x(double new_x); //set x and update distance
	void set_y(double new_y); //set y and update distance
	void set_coords(double new_x, double new_y); //set x and y and update distance
	void transpose(double x_shift, double y_shift);
						     
	/* Miscellaneous */
	bool operator==(Point other);
	double angle_to(Point other); //returns angle to other Point in radians, where 0 is a
				      //horizontal line
	void print();
	std::string to_string();
	Point midpoint(Point other); //returns the midpoint between 2 points
	//bool is_inside(Polygon p); //returns true if the point is inside (or comprises an
				   //edge of) Polygon p
				 
};


/*
bool x_comp(Point a, Point b){
	return a.get_x() < b.get_x();
}

bool y_comp(Point a, Point b){
	return a.get_y() < b.get_y();
}
*/

/*
bool x_comp(Point a, Point b);

bool y_comp(Point a, Point b);
*/
	
#endif

