#ifndef RANDOM_POLYGON_GENERATOR_H
#define RANDOM_POLYGON_GENERATOR_H
//header file for function which generates a random polygon given an (optional) number of vertices and (optional) upper/lower coord bounds
//need to decide whether to make the generator a class or just have a header file with a bunch of functions in it, like the STL headers
#include "polygon.h"

/* Generate a random polygon with between 4 and 99 vertices, with coordinates from -250 to 250 */
Polygon generate_random_polygon();

Polygon generate_random_polygon(int v);

Polygon generate_random_polygon(int v, double c);

Polygon generate_random_polygon(int v, double c1, double c2);




#endif
