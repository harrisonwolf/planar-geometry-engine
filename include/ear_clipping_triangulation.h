//header file for the algorithm which implements the ear-clipping triangulation of a polygon
#ifndef EAR_CLIPPING_TRIANGULATION_H
#define EAR_CLIPPING_TRIANGULATION_H

#include "triangle.h"
#include "polygon.h"
#include <vector>

std::vector<Triangle> triangulate(Polygon curr_polygon, std::vector<Triangle> &current_ears);

std::vector<Triangle> triangulate(Polygon p);

#endif
