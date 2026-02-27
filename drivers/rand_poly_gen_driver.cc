//driver program for development of random_polygon_generator

#include <iostream>
#include "polygon_new.h"
#include "random_polygon_generator.h"
#include "helper.h"
#include "logger.h"

using namespace std;

int main(int argc, char* argv[]){
	//TODO: Add arg for running program with given n
	//don't really need to use DBG since this entire program is specifically to drive development for rand poly gen
	//dbg statements belong in random poly gen file so that anywhere that that function is used, I can turn on debug statements and see what's going one
	int n = -1;
	if(argc < 2){
		n = 7;
	}else{
		n = stoi(argv[1]);
	}
	cout << "Beginning random_polygon_generator driver program.\n";	
	cout << "Testing first 3 points insertion...\n";
	Polygon p = generate_random_polygon(n);
	cout << "Random polygon generated: \n" << p.to_string() << endl;
	cout << "Desmos version: \n" << p.to_desmos() << endl;
	cout << "Triangulation: \n";
	p.print_triangulation();


	return 0;
}
