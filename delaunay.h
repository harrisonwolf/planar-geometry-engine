//header file for delaunay triangulation algorithm(s)
#include <vector>
#include "triangle.h"
//#include "pointset.h"
//#include <unordered_map>
#include <unordered_set>

//need to decide how to handle is_delaunay function for a given triangle within an existing triangulation
//maybe I make a separate triangulation class..?
//or with flipping alg, construct new triangulation in parallel..?

bool is_delaunay(std::vector<Triangle> triangulation

std::vector<Triangle> bowyer_watson_triangulate(std::unordered_set<Point> points);

std::vector<Triangle> dc_triangulate(std::unordered_set<Point> points);

std::vector<Triangle> flip_triangulate(std::vector<Triangle> triangulation);


