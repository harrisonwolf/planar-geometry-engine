//inplementation file for the helper functions
#include "helper.h"
#include "logger.h"
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <cmath>
#include <filesystem>
#include <vector>

using namespace std;

/*
 * Reads a point
 */
Point read_point(){
	cin.clear();
	double x = 0, y = 0;
	cin >> x >> y;
	//could do some input checking here
	return Point(x,y);
}

/*
 * Reads a point as input and returns it, with some output to the user
 */
Point read_point_verbose(){
	cout << "Please enter a point: [x y]\n\n";
	double x = 0, y = 0;
	cin.clear();
	cin >> x >> y;
	//could add some input checking here
	return Point(x,y);
}

vector<Point> read_points(int n){
	vector<Point> retvec;

	cout << "\nEnter a set of " << n << " points:\n\n";
	Point curr;
	for(int i=0; i<n; i++){
		cout << "Point " << i+1 << ": [x y]\n";
		curr = read_point();
		retvec.push_back(curr);
	}

	return retvec;
}

list<Point> read_point_list(int n){
	list<Point> retvec;

	cout << "\nEnter a set of " << n << " points:\n\n";
	Point curr;
	for(int i=0; i<n; i++){
		cout << "Point " << i+1 << ": [x y]\n";
		curr = read_point();
		retvec.push_back(curr);
	}

	return retvec;
}

/*
 * Read n lines from the user, giving them the option of how to
 * define each line
*/

vector<Line> read_lines(int n){ 

	//TODO: Make sure n>0
	
	vector<Line> retvec = vector<Line>(n);

	int def_choice = 0;

	cin.clear();

	while(def_choice < 1 or def_choice > 4) {

		cout << "Please select how you would like to define your lines:\n";
		cout << "1: Slope-Origin\n";
		cout << "2: Slope-Intercept\n";
		cout << "3: Point-Slope\n";
		cout << "4: Two Points\n";

		cin >> def_choice;
		
		if(def_choice < 1 or def_choice > 4){
			cout << "Must select an option between 1 and 4." << endl;
			cin.clear();
			continue;
		}
	}

	if(def_choice == 1){ //slope-origin
		double m = 0;
		for(int i=0; i<n; i++){
			cout << "\nEnter a slope to define line #" << i+1 << ": [m]\n\n";
			cin >> m;
			Line l(m);
			retvec.push_back(l);
		}

	}else if (def_choice == 2){ //slope-intercept
		double m=0, b=0;
		for(int i=0; i<n; i++){
			cout << "Enter a slope and y-intercept to define line #" << i+1 << ":\n\n";
			cout << "Slope: [m]\n";
			cin >> m;
			cout << "y-intercept: [b]\n";
			cin >> b;
			Line l(m,b);
			retvec.push_back(l);
		}

	}else if(def_choice == 3){ //point-slope
		Point p(0,0);
		double m = 0;

		for(int i=0; i<n; i++){
			cout << "Please enter a point on line #" << i+1 << " and its slope:\n\n";
			cout << "Point: [x y]\n";
			p = read_point();
			cout << "Slope: [m]\n";
			cin >> m;

			Line l(m,p);
			retvec.push_back(l);
		}

	}else if(def_choice == 4){ //two points
		double x=0, y=0;
		Point p1, p2;

		for(int i=0; i<n; i++){
			cout << "\nEnter 2 points to define line #" << i+1 << ":\n\n";
			cout << "Point 1: [x y]\n";
			cin >> x >> y;
			p1 = Point(x,y);
			cout << "Point 2: [x y]\n";
			cin >> x >> y;
			p2 = Point(x,y);

			Line l(p1,p2);
			retvec.push_back(l);
		}

	}

	return retvec;

}

Triangle read_triangle(){
	Triangle t; //we will set this later

	//first figure out how they want to define the triangle
	cout << "\nPlease choose how you would like to define your triangle:\n\n";
	cout << "1: Vertices\n";
	cout << "2: Equilateral Triangle with Given Center and Radius\n";
	cout << "3: Containing Edges\n";

	int subchoice = 0;
	cin.clear();
	cin >> subchoice;
	while(subchoice < 1 or subchoice > 3){ 
		cout << "Please select an option between 1 and 3.\n";
		cin.clear();
		cin >> subchoice;
		continue;
	}
	

	if(subchoice == 1){ //defined by vertices
		Point p1, p2, p3;
		vector<Point> my_points = read_points(3);

		p1 = my_points.at(0);
		p2 = my_points.at(1);
		p3 = my_points.at(2);

		t = Triangle(p1,p2,p3); 

	}else if(subchoice == 2){ //defined by center, radius
		cout << "Enter a center point and radius (to each vertex):\n\n";
		cout << "Center: [x y]\n";
		Point c = read_point();
		cout << "Radius: [r]\n";
		double r = 0;
		cin >> r;
		t = Triangle(c,r);

	}else if(subchoice == 3){ 

		cout << endl;
		Line first, second, third;

		vector<Line> my_lines = read_lines(3);
		first = my_lines.at(0);
		second = my_lines.at(1);
		third = my_lines.at(2);

		t = Triangle(first, second, third);
	}

	//now triangle has been set; return it
	
	return t;

}

vector<Triangle> read_triangles(int n){
	vector<Triangle> retvec;
	if(n<1){
		cout << "Please enter a number of triangles greater than 0. (Returning empty vector)\n";
		return retvec;
	}

	for(int i=0; i<n; i++){
		cout << "Triangle #" << i+1 << ":\n";
		Triangle t = read_triangle();
		retvec.push_back(t);
	}

	return retvec;

}

Polygon read_polygon(){
	cout << "Please enter how many vertices your polygon has:\n";
	int n = 0;
	cin.clear();
	cin >> n;
	while(n < 3){
		cout << "Polygons must have at least 3 vertices. Please try again:\n";
		cin.clear();
		cin >> n;
	}

	cout << "Please enter the locations of the vertices:\n";
	list<Point> vertices = read_point_list(n);

	return Polygon(vertices);

}


static std::string json_escape(const std::string& input){
	std::string escaped;
	escaped.reserve(input.size());
	for(char c: input){
		switch(c){
			case '"': escaped += "\\\""; break;
			case '\\': escaped += "\\\\"; break;
			case '\n': escaped += "\\n"; break;
			case '\r': escaped += "\\r"; break;
			case '\t': escaped += "\\t"; break;
			default: escaped += c; break;
		}
	}
	return escaped;
}

bool write_polygon_schema_file(const Polygon& polygon, const std::string& polygon_id,
                               const std::string& output_path){
	std::vector<Point> vertices;
	for(const Point& p: polygon.get_vertex_list()){
		if(!std::isfinite(p.get_x()) || !std::isfinite(p.get_y())){
			std::cout << "Polygon export error: non-numeric coordinate detected.\n";
			return false;
		}
		vertices.push_back(p);
	}

	if(vertices.size() > 1){
		const Point& first = vertices.front();
		const Point& last = vertices.back();
		if(first == last){
			vertices.pop_back();
		}
	}

	if(vertices.size() < 3){
		std::cout << "Polygon export error: fewer than 3 unique vertices.\n";
		return false;
	}

	std::ofstream out(output_path);
	if(!out.is_open()) return false;
	out << "{\n";
	out << "  \"type\": \"polygon\",\n";
	out << "  \"id\": \"" << json_escape(polygon_id) << "\",\n";
	out << "  \"points\": [\n";
	for(size_t i=0; i<vertices.size(); ++i){
		out << "    [" << vertices[i].get_x() << "," << vertices[i].get_y() << "]";
		if(i + 1 < vertices.size()) out << ",";
		out << "\n";
	}
	out << "  ]\n";
	out << "}\n";
	return true;
}

bool open_desmos_bridge_page(const std::string& bridge_path){
	std::filesystem::path canonical_bridge_path = std::filesystem::absolute(std::filesystem::path(bridge_path));
	std::string resolved_path = canonical_bridge_path.string();

	std::vector<std::string> commands;
	commands.push_back("xdg-open '" + resolved_path + "' >/dev/null 2>&1");
	commands.push_back("gio open '" + resolved_path + "' >/dev/null 2>&1");
	commands.push_back("open '" + resolved_path + "' >/dev/null 2>&1");

	for(const std::string& command: commands){
		int rc = system(command.c_str());
		if(rc == 0) return true;
	}

	std::cout << "Could not automatically open browser. Please open: " << resolved_path << "\n";
	return false;
}

bool is_inside(Point p, Triangle t){
	return t.contains(p);
}

bool is_inside(Point p, Point a, Point b, Point c){
	Triangle t(a,b,c);
	return is_inside(p,t);
}

bool collides(pair<Point,Point> pair1, pair<Point,Point> pair2){
	DBG("Entered collides function in helper file.\n");
	//now how do I actally check this
	//just find the intersection point of the two (inf) lines, and check if that point
	//is on BOTH line segments... but may run into precision errors
	Line l1(pair1.first,pair1.second);
	Line l2(pair2.first,pair2.second);
	if(!(l1.intersects(l2))){ //sanity check
		cout << "Error: in helper.cc collides, checking if parallel or superimposed line segments collide.\n";
		exit(1);
	}
	Point poi = l1.intersection(l2);
	DBG("Calculated virtual poi as " << poi.to_string() << ".\n");
	//now check if this point is between both pairs of given points
	if(poi.is_between(pair1.first,pair1.second) and poi.is_between(pair2.first,pair2.second)) return true;	
	return false; 
}
