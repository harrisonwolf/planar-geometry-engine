//header file for the algorithm which implements the ear-clipping triangulation of a polygon
#ifndef EAR_CLIPPING_TRIANGULATION_H
#define EAR_CLIPPING_TRIANGULATION_H

#include "die.h"
#include "point.h"
#include "triangle.h"
#include "polygon_new.h"
#include <list>
#include <vector>
#include <iostream>

std::vector<Triangle> triangulate(Polygon curr_polygon, std::vector<Triangle> &current_ears);

std::vector<Triangle> triangulate(Polygon p);

#endif
