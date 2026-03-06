#include <iostream>
//#include "die.h"
#include "point.h"
#include "line.h"
#include "triangle.h"

using namespace std;

int main(){
	Point a(2.893,4);
	Point b(-4.762,8);
	Point c(-1.33,-3.333);
	Triangle t(a,b,c);
	Point p1(-1,3);
	Point p2(100,100);
	cout << "t: " << t.to_string() << endl;
	cout << "p1: " << p1.to_string() << endl;
	cout << "p2: " << p2.to_string() << endl;
	cout << boolalpha << "p1 inside? " << t.contains(p1) << endl;
	cout << boolalpha << "p2 inside? " << t.contains(p2) << endl;

	return 0;
}
