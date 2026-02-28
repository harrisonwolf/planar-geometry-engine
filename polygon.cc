//implementation file for polygon class

#include "polygon.h"
#include "ear_clipping_triangulation.h"
#include "logger.h"
#include <iomanip>

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

	for(Point p: vertex_list){
		dict.insert({p.get_x(),p});
	}

	//TODO: Fill reflex/convex vertex maps

	triangulation = this->calculate_triangulation();
	area = this->calculate_area();
}

/*
 * Constructor with a given point list (in cyclic order)
 */
Polygon::Polygon(std::list<Point> given_vertices){//constructor which takes a given point list
	vertex_list = given_vertices;

	for(Point p: vertex_list){
		dict.insert({p.get_x(),p});
	}

	Point curr_v;
	Point prev_v;
	Point next_v;
	for(auto it=given_vertices.begin(); it!=given_vertices.end(); it++){
		//add each vertex to reflex or convex list
		//first check v1
		curr_v = *it;
		if(curr_v == given_vertices.front()){ //dealing with first vertex;
						      //check next and last
			prev_v = given_vertices.back();
			next_v = *next(it,1);

		}else if(curr_v == given_vertices.back()){ //dealing with last vertex;
							   //check prev and first
			prev_v = *prev(it,1);
			next_v = given_vertices.front();

		}else{ //general case, check prev and next
			next_v = *next(it,1);
			prev_v = *prev(it,1);
		}
		//prev, curr, next vertices have been set

		if(is_convex(prev_v,curr_v,next_v)){ //convex vertex
			convex_vertices.insert({curr_v.get_x(),curr_v});

		}else{ //reflex vertex
			reflex_vertices.insert({curr_v.get_x(),curr_v});
		}


	}

	DBG("About to call calculate_triangulation in polygon constructor\n");
	triangulation = this->calculate_triangulation();
	DBG("About to call calculate_area in polygon constructor\n");
	area = this->calculate_area();
	if(reflex_vertices.size() > 0) convex = false;
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
	DBG("stepped into is_convex function in polygon.cc\n");
	DBG("Checking if the sequence: \n");
	DBG(prev_v.to_string() << "\n");
	DBG(curr_v.to_string() << "\n");
	DBG(next_v.to_string() << "\n");
	DBG("is convex.\n");
	//first check to see if any of the lines have slope 0
	Line l1(curr_v,prev_v);
	Line l2(curr_v,next_v);
	//don't even need the whole horizontal line check since I ended up not doing the 
	//perpendiculars anyway
	bool inside_above = false; //true if "inside" is above curr-prev; false otherwise
	if(prev_v.get_x() > curr_v.get_x()) inside_above = true;
	else inside_above = false; //yes I know this line is unnecessary
	Point p = curr_v.midpoint(next_v);
	bool p_above = (p.get_y() > l1.get_slope()*p.get_x() + l1.get_y_intercept());
	//this calculates is p is "above" the line l2
	bool ans = (p_above == inside_above);
	if(ans) DBG("Yes\n");
	else DBG("No\n");
	return(p_above == inside_above);
	//if p is above (and inside is) or both are below, we're good
}
