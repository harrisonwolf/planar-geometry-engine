//header file for a point set

#ifndef POINTSET_H
#define POINTSET_H

#include "point.h"
#include <unordered_set>
#include <unordered_map>

class Pointset{
	private:
		std::unordered_map<Point> points;
		Polygon convex_hull;

	public:
		/* CONSTRUCTORS */

		/*
		 * Default constructor that makes an empty pointset
		 */

		/* ACCESSORS */
		Point contains(double xval){ return points.contains }
			

		/* MUTATORS */
		void add(Point p); //add a point to the pointset
		void erase(Point p); //removes a point from the pointset 

		/* MISC */
};





#endif
