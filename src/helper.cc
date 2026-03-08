//inplementation file for the helper functions
#include "helper.h"
#include "logger.h"
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <cmath>
#include <filesystem>
#include <chrono>
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

static bool normalize_polygon_vertices(const std::list<Point>& raw_vertices,
                                       std::vector<Point>& normalized_vertices,
                                       const std::string& error_prefix){
	normalized_vertices.clear();
	for(const Point& p: raw_vertices){
		if(!std::isfinite(p.get_x()) || !std::isfinite(p.get_y())){
			std::cout << error_prefix << ": non-numeric coordinate detected.\n";
			return false;
		}
		normalized_vertices.push_back(p);
	}

	if(normalized_vertices.size() > 1){
		const Point& first = normalized_vertices.front();
		const Point& last = normalized_vertices.back();
		if(first == last){
			normalized_vertices.pop_back();
		}
	}

	if(normalized_vertices.size() < 3){
		std::cout << error_prefix << ": fewer than 3 unique vertices.\n";
		return false;
	}

	return true;
}

static const char* artifact_type_name(GeometryArtifactType type){
	switch(type){
		case GeometryArtifactType::Polygon:
			return "polygon";
		case GeometryArtifactType::Triangulation:
			return "triangulation";
		case GeometryArtifactType::DelaunayTriangulation:
			return "delaunay_triangulation";
		case GeometryArtifactType::VoronoiDiagram:
			return "voronoi_diagram";
	}

	return "unknown";
}

static void write_schema_preamble(std::ofstream& out, GeometryArtifactType type, const std::string& artifact_id){
	out << "{\n";
	out << "  \"schemaVersion\": 1,\n";
	out << "  \"type\": \"" << artifact_type_name(type) << "\",\n";
	out << "  \"id\": \"" << json_escape(artifact_id) << "\"";
}

static bool write_polygon_payload(const Polygon& polygon, const std::string& polygon_id,
                                  const std::string& output_path){
	std::vector<Point> vertices;
	if(!normalize_polygon_vertices(polygon.get_vertex_list(), vertices, "Polygon export error")){
		return false;
	}

	std::ofstream out(output_path);
	if(!out.is_open()) return false;
	write_schema_preamble(out, GeometryArtifactType::Polygon, polygon_id);
	out << ",\n";
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

static bool write_triangulation_payload(const Polygon& polygon, const std::string& triangulation_id,
                                        const std::string& output_path){
	std::vector<Triangle> triangles = polygon.get_triangulation();
	if(triangles.empty()){
		std::cout << "Triangulation export error: polygon triangulation is empty.\n";
		return false;
	}

	std::ofstream out(output_path);
	if(!out.is_open()) return false;

	write_schema_preamble(out, GeometryArtifactType::Triangulation, triangulation_id);
	out << ",\n";
	out << "  \"polygons\": [\n";
	for(size_t i=0; i<triangles.size(); ++i){
		std::vector<Point> triangle_vertices{triangles[i].get_a(), triangles[i].get_b(), triangles[i].get_c()};
		if(!normalize_polygon_vertices(std::list<Point>(triangle_vertices.begin(), triangle_vertices.end()),
					      triangle_vertices,
					      "Triangulation export error")){
			return false;
		}

		out << "    {\n";
		out << "      \"type\": \"polygon\",\n";
		out << "      \"points\": [\n";
		for(size_t j=0; j<triangle_vertices.size(); ++j){
			out << "        [" << triangle_vertices[j].get_x() << "," << triangle_vertices[j].get_y() << "]";
			if(j + 1 < triangle_vertices.size()) out << ",";
			out << "\n";
		}
		out << "      ]\n";
		out << "    }";
		if(i + 1 < triangles.size()) out << ",";
		out << "\n";
	}
	out << "  ]\n";
	out << "}\n";
	return true;
}

static bool write_delaunay_payload(const DelaunayTriangulation& triangulation,
                                   const std::string& triangulation_id,
                                   const std::string& output_path){
	if(triangulation.sites.size() < 3){
		std::cout << "Delaunay export error: triangulation must contain at least 3 sites.\n";
		return false;
	}
	if(triangulation.triangles.empty()){
		std::cout << "Delaunay export error: triangulation contains no triangles.\n";
		return false;
	}

	std::ofstream out(output_path);
	if(!out.is_open()) return false;

	write_schema_preamble(out, GeometryArtifactType::DelaunayTriangulation, triangulation_id);
	out << ",\n";
	out << "  \"sites\": [\n";
	for(size_t i = 0; i < triangulation.sites.size(); ++i){
		const Point& site = triangulation.sites.at(i);
		if(!std::isfinite(site.get_x()) || !std::isfinite(site.get_y())){
			std::cout << "Delaunay export error: non-finite site coordinate detected.\n";
			return false;
		}
		out << "    [" << site.get_x() << "," << site.get_y() << "]";
		if(i + 1 < triangulation.sites.size()) out << ",";
		out << "\n";
	}
	out << "  ],\n";
	out << "  \"triangles\": [\n";
	for(size_t i = 0; i < triangulation.triangles.size(); ++i){
		const IndexedTriangle& triangle = triangulation.triangles.at(i);
		out << "    [" << triangle.a << "," << triangle.b << "," << triangle.c << "]";
		if(i + 1 < triangulation.triangles.size()) out << ",";
		out << "\n";
	}
	out << "  ]\n";
	out << "}\n";
	return true;
}

static bool write_voronoi_payload(const VoronoiDiagram& diagram,
                                  const std::string& diagram_id,
                                  const std::string& output_path){
	if(diagram.vertices.empty() && diagram.edges.empty()){
		std::cout << "Voronoi export error: diagram is empty.\n";
		return false;
	}

	std::ofstream out(output_path);
	if(!out.is_open()) return false;

	write_schema_preamble(out, GeometryArtifactType::VoronoiDiagram, diagram_id);
	out << ",\n";
	out << "  \"sites\": [\n";
	for(size_t i = 0; i < diagram.sites.size(); ++i){
		const Point& site = diagram.sites.at(i);
		out << "    [" << site.get_x() << "," << site.get_y() << "]";
		if(i + 1 < diagram.sites.size()) out << ",";
		out << "\n";
	}
	out << "  ],\n";
	out << "  \"vertices\": [\n";
	for(size_t i = 0; i < diagram.vertices.size(); ++i){
		const Point& vertex = diagram.vertices.at(i);
		if(!std::isfinite(vertex.get_x()) || !std::isfinite(vertex.get_y())){
			std::cout << "Voronoi export error: non-finite vertex detected.\n";
			return false;
		}
		out << "    [" << vertex.get_x() << "," << vertex.get_y() << "]";
		if(i + 1 < diagram.vertices.size()) out << ",";
		out << "\n";
	}
	out << "  ],\n";
	out << "  \"edges\": [\n";
	for(size_t i = 0; i < diagram.edges.size(); ++i){
		const VoronoiEdge& edge = diagram.edges.at(i);
		out << "    {\n";
		out << "      \"vertices\": [" << edge.start_vertex << "," << edge.end_vertex << "],\n";
		out << "      \"sites\": [" << edge.left_site << "," << edge.right_site << "]\n";
		out << "    }";
		if(i + 1 < diagram.edges.size()) out << ",";
		out << "\n";
	}
	out << "  ]\n";
	out << "}\n";
	return true;
}

bool write_geometry_artifact_file(const Polygon& polygon, const GeometryArtifactExport& export_spec){
	switch(export_spec.type){
		case GeometryArtifactType::Polygon:
			return write_polygon_payload(polygon, export_spec.artifact_id, export_spec.output_path);
		case GeometryArtifactType::Triangulation:
			return write_triangulation_payload(polygon, export_spec.artifact_id, export_spec.output_path);
		case GeometryArtifactType::DelaunayTriangulation:
		case GeometryArtifactType::VoronoiDiagram:
			std::cout << "Export artifact type '" << artifact_type_name(export_spec.type)
			          << "' is not implemented yet.\n";
			return false;
	}

	return false;
}

bool write_bridge_autoload_file(const Polygon& polygon, const std::string& output_path){
	std::vector<Point> vertices;
	if(!normalize_polygon_vertices(polygon.get_vertex_list(), vertices, "Bridge autoload export error")){
		return false;
	}

	std::vector<Triangle> triangles = polygon.get_triangulation();
	if(triangles.empty()){
		std::cout << "Bridge autoload export error: polygon triangulation is empty.\n";
		return false;
	}

	std::ofstream out(output_path);
	if(!out.is_open()) return false;

	out << "window.__GEOM_AUTOLOAD__ = {\n";
	out << "  schemaVersion: 1,\n";
	out << "  artifacts: [\n";
	out << "    {\n";
	out << "      type: \"polygon\",\n";
	out << "      id: \"poly1\",\n";
	out << "      points: [\n";
	for(size_t i = 0; i < vertices.size(); ++i){
		out << "        [" << vertices[i].get_x() << "," << vertices[i].get_y() << "]";
		if(i + 1 < vertices.size()) out << ",";
		out << "\n";
	}
	out << "      ]\n";
	out << "    },\n";
	out << "    {\n";
	out << "      type: \"triangulation\",\n";
	out << "      id: \"poly1_triangulation\",\n";
	out << "      polygons: [\n";
	for(size_t i = 0; i < triangles.size(); ++i){
		std::vector<Point> triangle_vertices{triangles[i].get_a(), triangles[i].get_b(), triangles[i].get_c()};
		if(!normalize_polygon_vertices(std::list<Point>(triangle_vertices.begin(), triangle_vertices.end()),
					      triangle_vertices,
					      "Bridge autoload export error")){
			return false;
		}

		out << "        {\n";
		out << "          type: \"polygon\",\n";
		out << "          points: [\n";
		for(size_t j = 0; j < triangle_vertices.size(); ++j){
			out << "            [" << triangle_vertices[j].get_x() << "," << triangle_vertices[j].get_y() << "]";
			if(j + 1 < triangle_vertices.size()) out << ",";
			out << "\n";
		}
		out << "          ]\n";
		out << "        }";
		if(i + 1 < triangles.size()) out << ",";
		out << "\n";
	}
	out << "      ]\n";
	out << "    }\n";
	out << "  ]\n";
	out << "};\n";
	return true;
}

bool write_polygon_schema_file(const Polygon& polygon, const std::string& polygon_id,
                               const std::string& output_path){
	return write_geometry_artifact_file(polygon,
		GeometryArtifactExport{GeometryArtifactType::Polygon, polygon_id, output_path});
}

bool write_triangulation_schema_file(const Polygon& polygon, const std::string& triangulation_id,
                                     const std::string& output_path){
	return write_geometry_artifact_file(polygon,
		GeometryArtifactExport{GeometryArtifactType::Triangulation, triangulation_id, output_path});
}

bool write_delaunay_schema_file(const DelaunayTriangulation& triangulation,
                                const std::string& triangulation_id,
                                const std::string& output_path){
	return write_delaunay_payload(triangulation, triangulation_id, output_path);
}

bool write_voronoi_schema_file(const VoronoiDiagram& diagram,
                               const std::string& diagram_id,
                               const std::string& output_path){
	return write_voronoi_payload(diagram, diagram_id, output_path);
}

static std::string make_file_url(const std::filesystem::path& absolute_path){
	std::string generic_path = absolute_path.generic_string();
	if(generic_path.empty()){
		return std::string();
	}
	if(generic_path.front() != '/'){
		generic_path.insert(generic_path.begin(), '/');
	}
	return "file://" + generic_path;
}

static bool open_local_page(const std::string& page_path, const std::string& query_string){
	std::filesystem::path canonical_page_path = std::filesystem::absolute(std::filesystem::path(page_path));
	std::string resolved_path = canonical_page_path.string();
	std::string open_target = make_file_url(canonical_page_path);
	if(open_target.empty()){
		open_target = resolved_path;
	}
	if(!query_string.empty()){
		open_target += query_string;
	}

	std::vector<std::string> commands;
	commands.push_back("xdg-open '" + open_target + "' >/dev/null 2>&1");
	commands.push_back("gio open '" + open_target + "' >/dev/null 2>&1");
	commands.push_back("open '" + open_target + "' >/dev/null 2>&1");
	if(query_string.empty()){
		commands.push_back("xdg-open '" + resolved_path + "' >/dev/null 2>&1");
		commands.push_back("gio open '" + resolved_path + "' >/dev/null 2>&1");
		commands.push_back("open '" + resolved_path + "' >/dev/null 2>&1");
	}

	for(const std::string& command: commands){
		int rc = system(command.c_str());
		if(rc == 0) return true;
	}

	std::cout << "Could not automatically open browser. Please open: " << open_target << "\n";
	return false;
}

bool open_desmos_bridge_page(const std::string& bridge_path, bool autoload_latest){
	if(!autoload_latest){
		return open_local_page(bridge_path, "");
	}

	const auto now = std::chrono::system_clock::now().time_since_epoch();
	const auto nonce = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
	return open_local_page(bridge_path,
	                       "?autoload=session&artifactSet=latest&nonce=" + std::to_string(nonce));
}

bool open_delaunay_viewer_page(const std::string& viewer_path, bool autoload_latest){
	if(!autoload_latest){
		return open_local_page(viewer_path, "");
	}

	const auto now = std::chrono::system_clock::now().time_since_epoch();
	const auto nonce = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
	return open_local_page(viewer_path, "?autoload=latest&nonce=" + std::to_string(nonce));
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

bool strict_collides(pair<Point,Point> pair1, pair<Point,Point> pair2){
	DBG("Entered strict_collides function in helper file.\n");
	//now how do I actally check this
	//just find the intersection point of the two (inf) lines, and check if that point
	//is on BOTH line segments... but may run into precision errors
	Line l1(pair1.first,pair1.second);
	Line l2(pair2.first,pair2.second);
	if(!(l1.intersects(l2))){ //sanity check
		cout << "Error: in helper.cc strict_collides, checking if parallel or superimposed line segments collide.\n";
		exit(1);
	}
	Point poi = l1.intersection(l2);
	DBG("Calculated virtual poi as " << poi.to_string() << ".\n");
	//now check if this point is between both pairs of given points
	if(poi.strict_is_between(pair1.first,pair1.second) and poi.strict_is_between(pair2.first,pair2.second)) return true;	
	return false; 
}
