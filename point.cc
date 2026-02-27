#include "point.h"
#include "logger.h"

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

string Point::to_string(){
	stringstream ss;
	ss << "(" << x << "," << y << ")";
	string retval = ss.str();
	return retval;
}

/* Member Functions */

bool Point::operator==(Point other){
	return ( this->x == other.get_x() and this->y == other.get_y() );
}

double Point::angle_to(Point other){
	if(x == other.get_x()) die();
	double rise = y - other.get_y();
	double run = x - other.get_x();
	double slope = ( rise/run );
	return atan(slope);
}

double Point::distance_to(Point other){
	return sqrt( pow((x-other.get_x()),2) + pow((y-other.get_y()),2) );
}

Point Point::midpoint(Point other){
	return Point( (x + (other.get_x() - x)/2 ), ((y + (other.get_y() - y)/2 ))  );
}

bool Point::is_between(Point p1, Point p2){
	//could use angle_to, but that is likely highly computationally expensive, esp. if using for each iteration of for loop in rand_poly_gen
	//worth a shot though
	//NOTE: angle_to just returns the angle of the segment between two points!!
	//Does not matter which point is given first
	DBG("Entered Point::is_between. Testing if " << this->to_string() << " is between "
			<< p1.to_string() << " and " << p2.to_string() << ".\n");
	double angle1 = this->angle_to(p1);
	double angle2 = this->angle_to(p2);
	if(angle1 == angle2) return true;
	return false;
}

/*
bool x_comp(Point a, Point b){
	return a.get_x() < b.get_x();
}

bool y_comp(Point a, Point b){
	return a.get_y() < b.get_y();
}
*/


