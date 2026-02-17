//source code for an algorithm that triangulates a given polygon p using ear_clipping
#include "ear_clipping_triangulation.h"

using namespace std;

/*
 * Outline:
 * 1: Iterate over vertex list, find all vertices that determine ears
 * 	1.1 For each vertex V_i (0<i<n), create a triangle using it and the two vertices 
 * 	adjacent to it in order
 * 	1.2: Test all other vertices to see if any are inside the current triangle.
 * 	If none inside, current vertex determines an ear
 *
 * For each recursive iteration, pass the current polygon and current clipped ears vector
 */

//function will originally be called as follows:
// assume Polygon p has been initialized
// vector<Triangle> triangulation = triangulate(p)

vector<Triangle> triangulate(Polygon p){
	cerr << "Calling initial triangulate.\n";
	vector<Triangle> clipped_ears;
	return triangulate(p,clipped_ears);
}

vector<Triangle> triangulate(Polygon curr_polygon, vector<Triangle> &current_ears){ 
	cerr << "Recursing on polygon of " << curr_polygon.get_vertex_list().size() << " vertices.\n";
	cerr << "curr_polygon: " << curr_polygon.to_string() << "\n";
	//base case 3 vertices
	if(curr_polygon.get_vertex_list().size() == 3){
		cerr << "Hit base case in triangulate recursion.\n";
		//make the remaining 3 vertices into a triangle, add it to ears, return ears
		vector<Point> points;
		for(Point p: curr_polygon.get_vertex_list()){
			points.push_back(p);
		}
		if(points.size() > 3){ //sanity check
			cout << "In triangulate recursion, base case polygon was turned into a triangle, and the resulting triangle has more than 3 vertices...\n";
			die();
		}else{ //so far, so good
		       Point p1 = points.at(0);
		       Point p2 = points.at(1);
		       Point p3 = points.at(2);
		       Triangle t(p1,p2,p3);
		       current_ears.push_back(t);
		       return current_ears;
		}
	}else{ //We still have vertices to contend with
		//find an ear, clip it, recurse with new smaller polygon and bigger ear vector
		
		// 1. Find an ear
		// 	1.1 Take current vertex, construct a triangle from it and its neighboring
		// 	vertices
		// 	1.2 Check all other vertices (excluding these 3) to see if they are 
		// 	within this triangle
		// 		1.2.1 If a vertex is, this vertex does not determine an ear;
		// 		move on
		// 		1.2.2 If no other vertex is within; this is an ear
		// 			1.2.2.1 Add this triangle to ears vector
		// 			1.2.2.2 Remove this vertex from the polygon, connect 
		// 			its neighbors (careful with this step - linked list)
		// 			1.2.2.3 Recurse on new polygon and ears vector

		list<Point> curr_vertices = curr_polygon.get_vertex_list();

		for(auto it = curr_vertices.begin(); it != curr_vertices.end(); it++){
			//std::next and std::prev (ex. next(it,1) ) return an iterator;
			//still need to dereference it! Don't forget
continue2:
			Point p = *it;
			Point prev_p;
			Point next_p;
			Triangle t;
			//FIRST!! DETERMINE IF REFLEX VERTEX
			//We do not consider reflex vertices since the triangle formed by
			//a reflex vertex and its neighbors will always be outside of
			//the actual polygon
			//Since points are given in clockwise order, the interior of the
			//polygon is always to the right (positive x)

			//first check if this vertex is the first or last
			if(p == curr_vertices.front()){ //first vertex; connect to next and last
				next_p = *next(it,1);
				prev_p = curr_vertices.back();
				//you never actually set this!!! you just made the triangle out of
				//the back point but then you tried using prev_p without actually
				//having set it
				t = Triangle(p, next_p, prev_p); 
				cerr << "p = front - prev_p = back() = " << curr_vertices.back().to_string() << "\n";
			}else if(p == curr_vertices.back()){ //last vertex; connect to prev and first
				next_p = curr_vertices.front();
				prev_p = *prev(it,1);
				t = Triangle(p, prev_p, next_p);
				cerr << "p = back() = " << p.to_string() << "\n";
			}else{ //general case; connect to prev and next
			       	next_p = *next(it,1);
				prev_p = *prev(it,1);
				cerr << "General case - prev_p = " << prev_p.to_string() << "\n";
				t = Triangle(p, next_p, prev_p);
			}

			//now the curr vertex and its triangle have been set
			//check all other vertices to see if they are inside
			//FIXME: FIRST MAKE SURE THIS VERTEX IS CONVEX
			//IF IT IS NOT, SKIP IT

			cerr << "Checking if vertex " << p.to_string() << " determines an ear.\n";
			cerr << "First checking if convex:\n";
			if(curr_polygon.get_reflex_vertices().count(p.get_x()) > 0){
				cerr << "Vertex reflex, continuing...\n";
			       	continue;
			}
					//this vertex is reflex; skip it
					//Note: map::contains is only C++20 and later

			cerr << p.to_string() << " convex, now see if any other vertices inside.\n";
			for(auto it_2 = curr_vertices.begin(); it_2 != curr_vertices.end(); it_2++){
				Point test_vertex = *it_2; //this is the vertex we're testing to
							 //see if it's in our potential ear
				//first make sure it's not one of the vertices of the triangle
				cerr << "test_vertex: " << test_vertex.to_string() << "\n";
				cerr << "prev_p : " << prev_p.to_string() << "\n";
				cerr << "next_p : " << next_p.to_string() << "\n";
				if(test_vertex == p or test_vertex == next_p or test_vertex == prev_p) continue;
				//not a vertex of the triangle - keep going

				if(t.contains(test_vertex)){
					cerr << p.to_string() << " contains " << test_vertex.to_string() << ".\n";
					//need to jump back to start of outer for loop but
					//can't just put this right at the end (and let the loop
					//itself increment stuff and handle things) because then
					//I end up "skipping past" an initialization of a polygon,
					//which the compiler does't like
					it++;
					//may need to check the outer loop conditional here
					//but it should be ok (since every polygon should theoret.
					//have at least one ear
					goto continue2;
				       	//break; //DO NOT BREAK HERE
					       //NEED TO RESTART FROM BEGINNING OF FIRST
					       //FOR LOOP WITH NEXT P
				}
				//if the vertex we're looking at is in the potential ear triangle,
				//it's not an ear; break out and look at the next potential ear
				//if we get to the end of the vertices and none are in the potential
				//ear, we found one! Add it to ears, remove the vertex from P,
				//and recurse


			}

			//if we still have a potential ear after testing all other vertices,
			//this one is an ear

			current_ears.push_back(t);
			curr_vertices.erase(it);
			Polygon new_p(curr_vertices);
			return triangulate(new_p,current_ears);

		}


		cout << "Never recursed in the ear clipping triangulation...";
		cout << " Something's gone wrong.\n";
		exit(1);
	}
}
