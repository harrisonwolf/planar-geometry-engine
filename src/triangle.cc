//implementation file for Triangle class

#include <iomanip>
#include "triangle.h"

using namespace std;

const double sqrt3 = sqrt(3); //inlining for runtime savings; one sqrt calc is ~50 clock cycles

Triangle::Triangle(){ //default constructor, initializes all points and area to zero
	a = Point(0,0);
	b = Point(0,0);
	c = Point(0,0);
	area = 0.0;
}

Triangle::Triangle(Point a, Point b, Point c){
	this->a = a;
	this->b = b;
	this->c = c;
	center_of_mass = this->calculate_center_of_mass();
	area = this->calculate_area();
	
	
}

/*
 * Makes an equilateral triangle centered at center with distance from the center to a vertex 
 * distance (can be circumscribed by a circle with radius distance)
 *
 * Triangle will be positioned so that one point is directly above center 
 */  
Triangle::Triangle(Point center, double distance){
	center_of_mass = center;
	area = (2*distance*distance/sqrt3);
	a = Point(center.get_x(), center.get_y()+distance);
	b = Point(center.get_x()+distance/sqrt3, center.get_y()-distance);	
	c = Point(center.get_x()-distance/sqrt3, center.get_y()-distance);
}

Triangle::Triangle(Line a, Line b, Line c){
	//just find the intersections of the 3 lines and make a triangle w/ those points
	Point a_b = a.intersection(b);	
	Point a_c = a.intersection(c);	
	Point b_c = b.intersection(c);	

	Triangle t (a_b, a_c, b_c);
	this->a = t.get_a();
	this->b = t.get_b();
	this->c = t.get_c();
	area = t.get_area();
	center_of_mass = t.get_center_of_mass();
}

/*
 * Calculates and returns area
 */
double Triangle::calculate_area(){
	double area = 0;
	double s = 0; //semiperimeter
	double x=0, y=0, z=0; //sides of the triangle
				//let's say x=ab, y=bc, z=ac
	x = a.distance_to(b);
	y = b.distance_to(c);
	z = a.distance_to(c);
	s = (x + y + z) / 2;

	area = sqrt( s*(s-x)*(s-y)*(s-z) );
	return area;
}

Point Triangle::calculate_center_of_mass(){
	//need to calculate midpoint of each side, then connect it to the opposite vertex. 
	//Center of mass is where the three constructed lines meet
	
	//first, find the midpoints and connect them to the opp. vertex
	Point ab_midpt = a.midpoint(b); 
	Point bc_midpt = b.midpoint(c); //technically don't need this one since they all intersect 
					//at the same point
	Point ac_midpt = a.midpoint(c);

	Line first, second;
	if(c.get_x() == ab_midpt.get_x()){ //don't make a line from these two
		first = Line(b,ac_midpt);
		second = Line(a,bc_midpt);
	}else if(b.get_x() == ac_midpt.get_x()){ //don't make a line from these two
		first = Line(c,ab_midpt);
		second = Line(a,bc_midpt);
	}else{ //just use the first two lines
		first = Line(c,ab_midpt);
		second = Line(b,ac_midpt);
	}

	Point center = first.intersection(second);
	return center;
}


/*
 * Could either make this just change the actual triangle given or just reconstruct a new triangle,
 * since certain things (like center of mass) will change. Problem is if I give a triangle more
 * information, I don't want to reconstruct it every time it's changed. Probably just stick to 
 * changing the actual given triangle for now.
 */
void Triangle::transpose(double x, double y){
	//first move all the points that make the vertices
	a.set_x(a.get_x()+x);	
	b.set_x(b.get_x()+x);	
	c.set_x(c.get_x()+x);	
	a.set_y(a.get_y()+y);	
	b.set_y(b.get_y()+y);	
	c.set_y(c.get_y()+y);	
	//area of the triangle will not change but center of mass will
	center_of_mass.transpose(x,y);
}

string Triangle::to_string() const{
	stringstream s;
	s << "{" << a.to_string() << ", " << b.to_string() << ", " << c.to_string() << "}\n";
	s << "Center: " << center_of_mass.to_string() << "\n";
	s << "Area: " << area;
	string retval = s.str();
	return retval;

}

string Triangle::to_desmos_polygon() const{
	stringstream ss;
	ss << "polygon(" << a.to_string() << "," << b.to_string() << "," << c.to_string() << ")";
	return ss.str();
}

bool Triangle::contains(Point p){
	//how do I know if a given point is within a triangle?	
	//find the area of t
	//get the vertices (a,b,c) of t
	//make 3 new triangles pab pac pbc, find their areas
	//if A1+A2+A3 = A, p is inside
	//if not, p is outside
	
	//1. get area of t
	double A = this->area;
	//cerr << "Area of big triangle (A): " << A << "\n";
	//2. get vertices
	Point a = this->a;
	Point b = this->b;
	Point c = this->c;
	Triangle t1(p,a,b);
	double a1 = t1.get_area();
	//cerr << "t1 area: " << a1 << "\n";
	Triangle t2(p,a,c);
	double a2 = t2.get_area();
	//cerr << "t2 area: " << a2 << "\n";
	Triangle t3(p,b,c);
	double a3 = t3.get_area();
	double aT = a1 + a2 + a3;
	//cerr << "t3 area: " << a3 << "\n";
	//cerr << "a1 + a2 + a3 = " << setprecision(17) << a1 + a2 + a3 << "\n";
	//cerr << "A = " << A << "\n";
	//cerr << "( (a1 + a2 + a3) == A )? " << ( (a1 + a2 + a3) == A ) << "\n";
	//cerr << "aT (total area of smaller t's) = " << aT << "\n";
	//cerr << "aT - A = " << (aT-A) << "\n";
	//return ( (a1 + a2 + a3) == A );
	return ( (std::fabs(aT-A)) < 0.000001 ); //rounding errors in double calculations force
					 //me to do this, as much as I don't want to
}

// 2d * d/sqrt3 
