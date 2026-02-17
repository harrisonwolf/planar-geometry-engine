//implementation file for the Polygon class

#include "polygon.h"

using namespace std;


/*
 * Default constructor makes a nice little trapezoid so I don't violate my own program invariants
 * (originally the default polygon was a square)
 */
Polygon::Polygon(){
	vertices = vector<Point>();
	Point p1 = Point(-2,0);
	Point p2 = Point(-1,1);
	Point p3 = Point(1,1);
	Point p4 = Point(2,0);

	vertices.push_back(p1);
	dict.insert({p1.get_x(),p1});
	vertices.push_back(p2);
	dict.insert({p2.get_x(),p2});
	vertices.push_back(p3);
	dict.insert({p3.get_x(),p3});
	vertices.push_back(p4);
	dict.insert({p4.get_x(),p4});

	area = calculate_area();
	triangulation = triangulate();	
	
}

Polygon::Polygon(vector<Point> given_points){
	vertices = given_points;
	for(Point p: given_points){ //add points to the dict and make sure there are no repeats
		double curr_x = p.get_x();
		if(dict.count(curr_x) > 0) die;
		else dict.insert({curr_x,p});		
	}
	triangulation = triangulate(); 
	area = calculate_area();
}

double Polygon::calculate_area(){// calculates and returns the area of a given
				 // polygon (probably using triangulations)
				 // 
	double total_area = 0;
	for(Triangle t: triangulation){
		total_area += t.get_area();
	}
	return total_area;
}

vector<Triangle> Polygon::triangulate(){
	//THIS IS THE TRIANGULATION OF THE POINT SET, NOT OF THE POLYGON
	//THIS WILL ONLY BE THE TRIANGULATION OF A POLYGON IF THE POINT SET IS CONVEX,
	//IN WHICH CASE THERE IS ONLY ONE SIMPLE POLYGONIZATION OF IT
	vector<Triangle> retval;
	//FIXME: Just found a problem... this only works for convex polygons

	//incremental algorithm for triangulation
	
	//since I know each point has a unique x-val, I can actually map them by x-val in a\
	//hash table
	//so first, push them all into a hash table based on x-val
	//them push the x-vals all into a vector<double>
	//sort this vector
	//now they are all sorted by x-val and can be accessed by looking up the x-val in the
	//hash table
	//
	//Once points have been sorted by x-val, here is how you get your triangles:
	//Connect first 3 points. This is your first triangle. now stop looking at point 1
	//and instead look at point 4. This is your next triangle
	//repeat until last point
	//SO EASY!! lol
	
	vector<double> x_vals;
	for(Point p: vertices){ //add each vertex to the x vector
		double curr_x = p.get_x();
	       	x_vals.push_back(curr_x);
	}
	sort(x_vals.begin(), x_vals.end());

	//Now we start our loop
	Point curr1, curr2, curr3;
	double x1, x2, x3;
	int cursor = 0;
	Triangle curr;
	while(1){
		if(cursor+2 >= x_vals.size()) break; //go until we get past the last vertex
		x1 = x_vals.at(cursor);
		x2 = x_vals.at(cursor+1);
		x3 = x_vals.at(cursor+2);
		curr1 = dict.at(x1);
		curr2 = dict.at(x2);
		curr3 = dict.at(x3);
		curr = Triangle(curr1,curr2,curr3);
		retval.push_back(curr);
		cursor++; //you forgot to increment cursor and spent 15 mins trying to debug and
			  //figure out why you were going into an infinite loop. nice
	}

	return retval;
}

string Polygon::to_string(){
	stringstream ss;
	ss << "Vertices:\n";
	for(Point p: vertices){
		ss << p.to_string() << "\n";
	}	
	ss << "Area: " << area << "\n";
	ss << "Concave/Convex? \n"; //FIXME
	return ss.str();
}

void Polygon::print_triangulation(){
	for(int i=0; i<triangulation.size(); i++){
		cout << "Triangle #" << i+1 << ":\n" << triangulation.at(i).to_string() << "\n\n";
	}
}
