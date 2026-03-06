//header file for a triangle class which is composed of 3 points
#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "point.h"
#include "line.h"
#include "die.h"
#include <sstream>
#include <string>
#include <algorithm>

class Triangle{
private:
	Point a;
	Point b;
	Point c;
	double area;
	//Type type;
	Point center_of_mass;

public:
	/* Constructors */

	Triangle(); //initializes all points to 0
	Triangle(Point a, Point b, Point c); //make a triangle with 3 given points
	Triangle(Point center, double distance); //make an equilateral triangle centered at a 
						 //point with a given distance from the center 
						 //to a vertex
	Triangle(Line a, Line b, Line c); //make a triangle out of 3 given lines (again assume
					  //general position, so the lines are not parallel

	/* Accessors */

	Point get_a(){ return a; }
	Point get_b(){ return b; }
	Point get_c(){ return c; }
	double get_area(){ return area; } //returns the area of the triangle
	double calculate_area(); //calculates and returns area of the triangle
	//Type get_type(){ return type; }
	Point get_center_of_mass(){ return center_of_mass; } //gives the "center of mass" of the 
							     //triangle
	Point calculate_center_of_mass(); //calculates and retruns center of mass

	/* Mutators */

	void transpose(double x, double y); //transpose the triangle by x units on the x-axis 
					    //and y units on the y-axis (x and y can be negative 
					    //or 0)
	
	/* Misc */

	/*
	 * Returns a string with all of the triangles information in the following format:
	 *
	 * "{(a.x, b.y), (b.x, b.y), (c.x, c.y)}\n"
	 * "center: (x,y)\n"
	 * "area: A" //note there is no carriage return here
	 */ 
	std::string to_string() const; 
	bool contains(Point p);
	std::string to_desmos_polygon() const;
};

#endif
