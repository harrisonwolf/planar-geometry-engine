//implementation file for polygon class

#include "polygon.h"
#include "ear_clipping_triangulation.h"
#include "logger.h"
#include <iomanip>
#include <cmath>

using namespace std;

/* Constructors */

/*
 * Default Constructor with no args
 */
Polygon::Polygon(){
	Point p1 = Point(-2,0);
	Point p2 = Point(-1,1);
	Point p3 = Point(1,1);
	Point p4 = Point(2,0);
	vertex_list = list<Point>{p1,p2,p3,p4};

	const vector<Point> vertices(vertex_list.begin(), vertex_list.end());
	double area2 = 0.0;
	for(size_t i = 0; i < vertices.size(); ++i){
		const Point& a = vertices[i];
		const Point& b = vertices[(i + 1) % vertices.size()];
		area2 += a.get_x() * b.get_y() - b.get_x() * a.get_y();
	}
	const bool ccw = area2 > 0.0;
	for(size_t i = 0; i < vertices.size(); ++i){
		const Point& prev_v = vertices[(i + vertices.size() - 1) % vertices.size()];
		const Point& curr_v = vertices[i];
		const Point& next_v = vertices[(i + 1) % vertices.size()];
		double turn = cross(curr_v - prev_v, next_v - curr_v);
		if(std::fabs(turn) < 1e-12 || (ccw ? turn > 0.0 : turn < 0.0)) convex_vertices.push_back(curr_v);
		else reflex_vertices.push_back(curr_v);
	}

	triangulation = this->calculate_triangulation();
	area = this->calculate_area();
	if(!reflex_vertices.empty()) convex = false;
}

/*
 * Constructor with a given point list (in cyclic order)
 */
Polygon::Polygon(std::list<Point> given_vertices){//constructor which takes a given point list
	vertex_list = std::move(given_vertices);

	const vector<Point> vertices(vertex_list.begin(), vertex_list.end());
	double area2 = 0.0;
	for(size_t i = 0; i < vertices.size(); ++i){
		const Point& a = vertices[i];
		const Point& b = vertices[(i + 1) % vertices.size()];
		area2 += a.get_x() * b.get_y() - b.get_x() * a.get_y();
	}
	const bool ccw = area2 > 0.0;
	for(size_t i = 0; i < vertices.size(); ++i){
		const Point& prev_v = vertices[(i + vertices.size() - 1) % vertices.size()];
		const Point& curr_v = vertices[i];
		const Point& next_v = vertices[(i + 1) % vertices.size()];
		double turn = cross(curr_v - prev_v, next_v - curr_v);
		if(std::fabs(turn) < 1e-12 || (ccw ? turn > 0.0 : turn < 0.0)){
			convex_vertices.push_back(curr_v);
		}else{
			reflex_vertices.push_back(curr_v);
		}
	}

	DBG("About to call calculate_triangulation in polygon constructor\n");
	triangulation = this->calculate_triangulation();
	DBG("About to call calculate_area in polygon constructor\n");
	area = this->calculate_area();
	if(!reflex_vertices.empty()) convex = false;
}

/* Accessors */

/*
 * Actually calculates the area (using triangulation)
 */
double Polygon::calculate_area(){
	double total_area = 0.0;
	for(Triangle t: triangulation){
                total_area += t.get_area();
        }
	return total_area;
}

vector<Triangle> Polygon::calculate_triangulation(){ //returns a set of triangles that form the 
						     //triangulation of the polygon
	DBG("Beginning calc_triang function call\n");
	vector<Triangle> retvec = triangulate(*this); //call it like this because the
						      //triangulate function exists outside
						      //of the Polygon class (why I have to
						      //dereference this)
	return retvec;
}


/* Mutators */ 

/* Miscellaneous */

/*
 * Returns string in the following format
 * Vertices:
 * {x1,y1}
 * {x2,y2}
 * ...
 * {xn,yn}
 * Area:
 * Convex/Concave
 */
string Polygon::to_string(){
	DBG("Entered Polygon::to_string() function\n");
	string concavity = "";
	if(convex) concavity = "Convex";
	else concavity = "Concave";
	stringstream ss;
        ss << "Vertices:\n";
        for(Point p: vertex_list){
                ss << p.to_string() << "\n";
        }
        ss << "Area: " << area << "\n";
        ss << "Concave/Convex? " << concavity;
        return ss.str();
}

string Polygon::to_desmos(){
	DBG("Entered Polygon::to_desmos() function\n");
	stringstream ss;
	ss << fixed << setprecision(6);
	for(Point p: vertex_list){
		ss << p.get_x() << "\t" << p.get_y() << "\n";
	}
	if(!vertex_list.empty()){
		Point first_vertex = vertex_list.front();
		ss << first_vertex.get_x() << "\t" << first_vertex.get_y();
	}
	return ss.str();
}

void Polygon::print_triangulation(){
	for(Triangle t: triangulation)
		cout << t.to_string() << "\n";
}

void Polygon::print_triangulation_desmos(){
	for(Triangle t: triangulation)
		cout << "FIXME"; //FIXME
}

bool Polygon::contains(Point p){ //returns true if p is inside of (or comprises an edge or vertex of)
				 //this->polygon
	for(Triangle& t: triangulation){
		if(t.contains(p)) return true;
	}
	return false;
}

bool is_convex(Point prev_v, Point curr_v, Point next_v){
	const Point a = curr_v - prev_v;
	const Point b = next_v - curr_v;
	const double turn = cross(a,b);
	if(std::fabs(turn) < 1e-12) return true;
	// Preserve historic contract (clockwise point order expected by callers).
	return turn < 0.0;
}
