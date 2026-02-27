#include "point.h"
#include "logger.h"
#include <cmath>
#include <algorithm>

using namespace std;

/* Constructors */

Point::Point(){
	x = 0.0;
	y = 0.0;
	distance = 0.0;
}

Point::Point(double new_x, double new_y){
	x = new_x;
	y = new_y;
	distance = sqrt(x*x + y*y);
}

/* Setters */

void Point::set_x(double new_x){ //set x and update distance
	x = new_x;
	distance = sqrt(x*x + y*y);
}

void Point::set_y(double new_y){ //set x and update distance
	y = new_y;
	distance = sqrt(x*x + y*y);
}

void Point::set_coords(double new_x, double new_y){
	x = new_x;
	y = new_y;
	distance = sqrt(x*x + y*y);
}

void Point::transpose(double x_shift, double y_shift){
	x = x+x_shift;
	y = y+y_shift;
	distance = sqrt(x*x + y*y);
}

void Point::print(){
	std::cout << "(" << x << ", " << y << ")";
}

string Point::to_string() const{
	stringstream ss;
	ss << "(" << x << "," << y << ")";
	string retval = ss.str();
	return retval;
}

/* Member Functions */

bool Point::operator==(Point other) const{
	return ( this->x == other.get_x() and this->y == other.get_y() );
}

double Point::angle_to(Point other) const{
	if(x == other.get_x()) die();
	double rise = y - other.get_y();
	double run = x - other.get_x();
	double slope = ( rise/run );
	return atan(slope);
}

double Point::distance_to(Point other) const{
	return sqrt( pow((x-other.get_x()),2) + pow((y-other.get_y()),2) );
}

Point Point::midpoint(Point other) const{
	return Point( (x + (other.get_x() - x)/2 ), ((y + (other.get_y() - y)/2 ))  );
}

bool Point::is_between(Point p1, Point p2) const{
	//could use angle_to, but that is likely highly computationally expensive, esp. if using for each iteration of for loop in rand_poly_gen
	//worth a shot though
	//NOTE: angle_to just returns the angle of the segment between two points!!
	//Does not matter which point is given first
	//THAT MEANS IT DOES NOT WORK FOR THIS APPLICATION
	DBG("Entered Point::is_between. Testing if " << this->to_string() << " is between "
			<< p1.to_string() << " and " << p2.to_string() << ".\n");
	Point twoMinus1 = p2 - p1;
    Point thisMinus1 = *this - p1;

    // Scale epsilon to the problem size a bit.
    double scale = max({1.0, fabs(twoMinus1.get_x()), fabs(twoMinus1.get_y()), fabs(thisMinus1.get_x()), fabs(thisMinus1.get_y())});
    double eps = 1e-12 * scale;

    // 1) Collinearity
    if (fabs(cross(twoMinus1, thisMinus1)) > eps) return false;

    // 2) Between-ness via projection
    double proj = dot(thisMinus1, twoMinus1);
    if (proj < -eps) return false;

    double len2 = dot(twoMinus1, twoMinus1);
    if (proj > len2 + eps) return false;

    return true;
}

/*
   bool x_comp(Point a, Point b){
   return a.get_x() < b.get_x();
   }

   bool y_comp(Point a, Point b){
   return a.get_y() < b.get_y();
   }
   */


