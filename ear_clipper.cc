#include <iostream>
#include "polygon_new.h"
#include "helper.h"
#include <vector>
#include "triangle.h"
#include "ear_clipping_triangulation.h"

using namespace std;

int main(){
	cout << "How many vertices does the polygon have?\n";
	int n = 0;
	cin >> n;
	list<Point> points = read_point_list(n);
	Polygon p(points);

	cout << "Here is your polygon:\n" << p.to_string() << endl;

	vector<Triangle> triangulation = triangulate(p);
	cout << "Here is the triangulation of p:\n\n";
	for(Triangle t: triangulation){
		cout << t.to_string() << "\n";
	}

}

