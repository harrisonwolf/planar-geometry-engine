//driver program for development of random_polygon_generator

#include <iostream>
#include "polygon_new.h"
#include "random_polygon_generator.h"
#include "helper.h"
#include "logger.h"

using namespace std;

int main(){
	//don't really need to use DBG since this entire program is specifically to drive development for rand poly gen
	//dbg statements belong in random poly gen file so that anywhere that that function is used, I can turn on debug statements and see what's going one
	cout << "Beginning random_polygon_generator driver program.\n";	
	cout << "Testing first 3 points insertion...\n";
	Polygon p = generate_random_polygon(7);
	cout << "Random polygon generated: \n" << p.to_string() << endl;
	cout << "Desmos version: \n" << p.to_desmos() << endl;

}
