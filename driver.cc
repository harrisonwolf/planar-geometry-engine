#include <iostream>
#include <vector>
#include "point.h"
#include "triangle.h"
#include "polygon.h"
#include "line.h"

using namespace std;

int main(){
	Point a(1,2);
	cout << "a: (" << a.get_x() << ", " << a.get_y() << ")\n";	
	cout << "Distance from a to origin: " << a.get_distance() << endl;

	vector<Point> points;
	for(int i=0; i<4; i++){
		points.push_back(Point(i,i+1));
	}	
	for(auto curr: points){ 
		curr.print();
		cout << "\n";
	}

	Polygon p(points);
	cout << "\nPoints in polygon p:\n";

	vector<Point> my_vec = p.get_vertices();
	for(Point curr: my_vec){
		curr.print();
		cout << "\n";
	}

	Triangle t(Point(5,5),1.0);
	cout << "\nTriangle t:\n";
	t.get_a().print();
	cout << "\n";
	t.get_b().print();
	cout << "\n";
	t.get_c().print();
	cout << "\n";
	cout << "Area: " << t.get_area() << endl;
	
	Triangle t2(Point(0,0),Point(1,0),Point(0,1));
	cout << "\nTriangle t2:\n";
	t2.get_a().print();
	cout << "\n";
	t2.get_b().print();
	cout << "\n";
	t2.get_c().print();
	cout << "\n";
	cout << "Center of mass: " << t2.get_center_of_mass().to_string() << endl;

	cout << "\nPrinting t2 using print function:\n";
	cout << t2.to_string() << endl;

	cout << "\nTrying out area function on a weird scalene:\n";
	vector<Point> t3_points;
	t3_points.push_back(Point(1,1));
	t3_points.push_back(Point(2,2));
	t3_points.push_back(Point(2,3));
	Triangle t3 = Triangle(t3_points.at(0),t3_points.at(1),t3_points.at(2));

	cout << "t3:\n" << t3.to_string() << endl;

	cout << "\nTransposing t3 up 3 units, right 5 units:\n";
	t3.transpose(5,3);
	cout << "t3:\n" << t3.to_string() << endl;

	cout << "Testing line intersection: \n";
	Line l1(2, 5);
	Line l2(-5, 1);

	cout << "L1: " << l1.to_string() << "\n";
	cout << "L2: " << l2.to_string() << "\n";

	cout << "\nL1 intersects L2? " << l1.intersects(l2) << endl;

	cout << "Point of intersection: " << l1.intersection(l2).to_string() << endl;

	vector<Point> input_points;
	while(1){
		input_points.clear();

		cout << "\nEnter 1 to create a triangle, 2 to quit:\n";
		int choice;
		cin >> choice;
		if(choice != 1) break;

		cout << "\nEnter a set of 3 points to create a triangle:\n\n";
		double x,y;
		cout << "Point 1: [x y]\n";
		cin >> x >> y;
		input_points.push_back(Point(x,y));
		cout << "Point 2: [x y]\n";
		cin >> x >> y;
		input_points.push_back(Point(x,y));
		cout << "Point 3: [x y]\n";
		cin >> x >> y;
		input_points.push_back(Point(x,y));
		cout << endl;

		Triangle input_triangle( input_points.at(0), input_points.at(1), input_points.at(2));
		cout << "Your triangle:\n" << input_triangle.to_string() << endl;

	}


	return 0;
}
