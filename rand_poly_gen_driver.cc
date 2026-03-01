//driver program for development of random_polygon_generator

#include <iostream>
#include <string>
#include "polygon.h"
#include "random_polygon_generator.h"
#include "helper.h"
#include "logger.h"

using namespace std;

int main(int argc, char* argv[]){
	logger::apply_runtime_inputs(argc, argv);

	//TODO: Add arg for running program with given n
	//don't really need to use DBG since this entire program is specifically to drive development for rand poly gen
	//dbg statements belong in random poly gen file so that anywhere that that function is used, I can turn on debug statements and see what's going one
	int NUMBER_OF_VERTICES = 7;
	for(int i = 1; i < argc; ++i){
		std::string arg = argv[i];
		if(arg.rfind("--", 0) == 0){
			continue;
		}
		NUMBER_OF_VERTICES = stoi(arg);
		break;
	}
	cout << "Beginning random_polygon_generator driver program.\n";	
	cout << "Testing first 3 points insertion...\n";
	Polygon p = generate_random_polygon(NUMBER_OF_VERTICES);
	cout << "Random polygon generated: \n" << p.to_string() << endl;
	if(1){ //export to desmos API
	  //can be annoying when just trying to run program
		string output_path = "tools/desmos-bridge/polygon-export.json";
		string triangulation_output_path = "tools/desmos-bridge/triangulation-export.json";
		string bridge_path = "tools/desmos-bridge/index.html";
		if(write_polygon_schema_file(p, "poly1", output_path)){
			cout << "Exported polygon schema to " << output_path << "\n";
			if(write_triangulation_schema_file(p, "poly1_triangulation", triangulation_output_path)){
				cout << "Exported triangulation schema to " << triangulation_output_path << "\n";
			}else{
				cout << "Failed to write triangulation export file.\n";
			}
			if(!open_desmos_bridge_page(bridge_path)){
				cout << "If the browser did not open, manually open " << bridge_path
					<< " and load polygon-export.json, then triangulation-export.json.\n";
			}
		}else{
			cout << "Failed to write polygon export file.\n";
		}
	}


	return 0;
}
