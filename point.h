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
	double get_x() const { return x; }
	double get_y() const { return y; }
	double get_distance() const { return distance; }
	double distance_to(Point other) const;

	/* Mutators */
	void set_x(double new_x); //set x and update distance
	void set_y(double new_y); //set y and update distance
	void set_coords(double new_x, double new_y); //set x and y and update distance
	void transpose(double x_shift, double y_shift);
						     
	/* Miscellaneous */
	bool operator==(Point other) const;
	double angle_to(Point other) const; //returns angle to other Point in radians, where 0 is a
				      //horizontal line
	inline friend Point operator-(const Point& a, const Point& b) {
		return {a.x - b.x, a.y - b.y};
	}

	inline friend double dot(const Point& u, const Point& v) {
		return u.x * v.x + u.y * v.y;
	}

	inline friend double cross(const Point& u, const Point& v) {
		return u.x * v.y - u.y * v.x;
	}
	void print();
	std::string to_string() const;
	Point midpoint(Point other) const; //returns the midpoint between 2 points
	//bool is_inside(Polygon p); //returns true if the point is inside (or comprises an
				   //edge of) Polygon p
				 
	bool is_between(Point p1, Point p2) const; //checks if this point is between two others
	bool strict_is_between(Point p1, Point p2) const; //checks if this point is strictly between two others
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

