#include <iostream>
#include <vector>
#include "point.h"
#include "triangle.h"
//#include "polygon.h"
#include "polygon_new.h"
#include "line.h"
#include "helper.h"
#include <random>
#include "choice.h"

using namespace std;

int main(){

	system("clear");

	//FIXME
	//Polygon can be described by cyclic order on its vertices. This is how we must define
	//our polygons. List the vertices in sequence, with the last connecting to the first.
	//Linked list may be useful for this.
	//
	//Gonna use ear clipping for triangulating a non-convex polygon
	//Should run in O(n^2) time..?

	//TODO:
	// 1. save/load stored objects
	// 2. visualize!!!
	// ncurses maybe? Or port into desmos probably easiest way; just get the vertices of the
	// straight skeleton and polygonize it using desmos
	//maybe port random points (or any points) into desmos...
	// 3. FIXME: Ask Devadoss abt triangulation algorithm for concave polygons... because
	//the incremental algorithm gives too much area if the polygon is concave... right?
	//Also may not count all of the triangles in the triangulation, since some "appear" as the
	//result of other triangles being created, and are not directly "constructed" by the
	//algorithm
	// 4. Compute straight skeleton 
	vector<Point> stored_points;
	vector<Line> stored_lines;
	vector<Triangle> stored_triangles;
	vector<Polygon> stored_polygons;

	//cout << "Welcome to Harrison\'s planar geometry library." << endl;
	//this doesn't work, but if I put the exact same thing into a script and run it, it does
	system("./print_welcome_message.sh");
	cout << endl;
	cout << "\033[1;31;6mATTENTION\033[0m\n";
	cout << "\033[1;31mPlease remember that all points given by the user are assumed to be in general position; that is, no two points lie on the same vertical line, and no three points are colinear.\033[0m" << endl;
	cout << "\033[1;31mAlso, it is up to the user to ensure that the polygon they input is simple, and that vertices are given in clockwise order.\033[0m" << endl;
	//note: the \033 stuff is telling cout to print different text color, etc
	//\033 is the ASCII escape character, then [, then codes separated by ; to tell the console
	//how you want it to print
	
	bool first_iteration = true;
	cout << "\n";
	
	while(1){
		//print out choices the first time
		//display output
		//ask to continue or quit
		//if continue, clear screen and reprint options
		if(!first_iteration){ //if it's the first iteration, leave the welcome message up
			system("clear");
		}else{
			first_iteration = false;
		}
		print_choices();
		int choice;
		cin >> choice;
		if(choice < 1 or choice > 5) break;

		if(choice == 1){ //make a triangle
			Triangle t = read_triangle();
			//add it to the vector of stored triangles
			stored_triangles.push_back(t);
			//print it
			cout << "Your triangle:\n" << t.to_string() << endl;

		}else if(choice == 2){ //find the intersection of 2 lines
			cout << "\n";
			vector<Line> my_lines = read_lines(2);
			Line first = my_lines.at(0);
			Line second = my_lines.at(1);

			Point poi = first.intersection(second);

			//add these lines to the vector we've made, as well as the POI
			stored_lines.push_back(first);
			stored_lines.push_back(second);
			stored_points.push_back(poi);

			cout << "\nPoint of intersection:\n" << poi.to_string() << endl;

		}else if(choice == 3){ //view points, lines, or triangles
			int subchoice = 0;
			cout << "Please select which you would like to view:\n";
			cout << "1: Stored Points\n";
			cout << "2: Stored Lines\n";
			cout << "3: Stored Triangles\n";
			cout << "4: Stored Polygons\n";
			cout << "5: All\n";
			cin >> subchoice;
			//maybe add some input checking
			if(subchoice < 1 or subchoice > 5) continue;
			if(subchoice == 1){ //points
				cout << "The points that have been created are:\n\n";
				for(Point p: stored_points){
					cout << p.to_string() << "\n\n";
				}
				cout << endl;

			}else if(subchoice == 2){ //stored lines
				cout << "The lines that have been created are:\n\n";
				for(Line l: stored_lines){
					cout << l.to_string() << "\n\n";
				}
				cout << endl;

			}else if(subchoice == 3){ //stored triangles
				cout << "The triangles that have been created are:\n\n";
				for(Triangle t: stored_triangles){
					cout << t.to_string() << "\n\n";
				}
				cout << endl;

			}else if(subchoice == 4){ //view stored polygons{
				cout << "The polygons that have been created are:\n\n";
				for(Polygon p: stored_polygons){
					cout << p.to_string() << "\n\n";
				}
				cout << endl;
				
			}else if(subchoice == 5){ //view everything
				//TODO
			}

		}else if(choice == 4){ //create a polygon
			Polygon p = read_polygon();
			stored_polygons.push_back(p);
			cout << p.to_string() << endl;
			cout << "\nTriangulation of p: \n";
			p.print_triangulation();
			cout << "\n\n";
		}else if(choice == 5){ //polygon w random vertices
			cout << "Please enter how many vertices your polygon has:\n";
			int n = 0;
			cin.clear();
			cin >> n;
			while(n < 3){
				cout << "Polygons must have at least 3 vertices. Please try again:\n";
				cin.clear();
				cin >> n;
			}

			cout << "Please enter a lower bound for the coordinates:\n";
			double lower = 0.0;
			cin >> lower;
			cout << "Please enter an upper bound for the coordinates:\n";
			double upper = 0.0;
			cin >> upper;

			uniform_real_distribution<double> unif(lower,upper);
			default_random_engine re;

			list<Point> random_points;
			for(int i=0; i<n; i++){
				double random_x = unif(re);
				double random_y = unif(re);
				Point p(random_x, random_y);
				random_points.push_back(p);
			}

			Polygon rand_poly(random_points);
			//store it
			stored_polygons.push_back(rand_poly);
			//now print it
			cout << "Your polygon:\n" << rand_poly.to_string() << "\n\n";

		}else if(choice == 6){ //quit (currently)
			return 0;
		}

		//after output has been printed, ask to continue or quit
		cout << "Please enter 1 to continue, 2 to quit:\n";
		cin.clear();
		cin >> choice;
		if(choice == 1) continue;
		else return 0;


	}


	return 0;
}
