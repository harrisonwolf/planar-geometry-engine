//helper program to create a CONVEX polygon in the correct format for the program and write it to
//a file so that you can run this, it'll create a polygon of (n) vertices that is SIMPLE AND
//CONVEX, and then write it to a file that is ready to be fed into main

//USAGE: "./create_convex_polygon [number of vertices] [filename to write to]"

#include <iostream>
#include <fstream>
#include <vector>
#include "point.h"
#include "helper.h"
#include "polygon_new.h"

using namespace std;

void print_usage_message(){
	cout << "USAGE: \"./create_convex_polygon_file [number of vertices] [filename to write to]\"";

}

int main(int argc, char **argv){
	if(argc > 2){ 
		print_usage_message();
		cout << endl;
	}else{
		cout << "STUB\n";
	}

	return 0;
}
