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
	int NUMBER_OF_VERTICES = -1;
	if(argc < 2){
		NUMBER_OF_VERTICES = 7;
	}else{
		NUMBER_OF_VERTICES = stoi(argv[1]);
	}
	cout << "Beginning random_polygon_generator driver program.\n";	
	cout << "Testing first 3 points insertion...\n";
	Polygon p = generate_random_polygon(NUMBER_OF_VERTICES);
	cout << "Random polygon generated: \n" << p.to_string() << endl;
	string output_path = "tools/desmos-bridge/polygon-export.json";
	if(write_polygon_schema_file(p, "poly1", output_path)){
		cout << "Exported polygon schema to " << output_path << "\n";
		open_desmos_bridge_page("file:///workspace/planar-geometry-engine-v1/tools/desmos-bridge/index.html");
	}else{
		cout << "Failed to write polygon export file.\n";
	}


	return 0;
}
