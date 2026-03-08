//header file for a line class which has a slope and intercept. should be able to calculate 
//if it intersects with another given line, and if so, where

#ifndef LINE_H
#define LINE_H

#include <sstream>
#include <string>
#include "point.h"
#include "die.h"

class Line{
private:
	double slope;
	double y_intercept;

public:
	/* Constructors */
	Line();
	Line(double slope); //makes a line with given slope that passes through the origin
	Line(double slope, double y_intercept); //makes a line with a given slope that has a 
						//given y-intercept
	Line(double slope, Point intercept); //makes a line with given slope that passes 
					     //through a given point
	Line(Point p1, Point p2); //makes a line that passes through 2 given points
				  //if p1 = p2, will just give line of slope 1
				  //passing through that point
	
	/* Accessors */
	double get_slope(){ return slope; }
	double get_y_intercept(){ return y_intercept; }


	/* Mutators */

	/* Misc */
	bool intersects(Line other); //returns true if this line intersects a given other line
	Point intersection(Line other); //returns the point of intersection with another line 
					//if any such point exists (if the lines are the same, 
					//it will simply return their y-intercept
	void print(); //prints the line
	std::string to_string() const; //returns a formatted string with the line's information
};


#endif
