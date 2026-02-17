#include "choice.h"

using namespace std;

void print_choices(){ //print available choices

	cout << "\033[1mPlease select an option:\n\033[0m";
	cout << "1: Create a Triangle\n";
	cout << "2: Find the intersection of 2 lines:\n";
	cout << "3: View Created Points, Lines, Triangles, or Polygons\n";
	cout << "4: Create a Polygon\n";
	cout << "5: Create a Polygon with randomly selected vertices\n";
	cout << "6: Quit\n";
}
