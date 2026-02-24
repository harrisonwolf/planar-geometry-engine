//header file for a point set
//Need to decide whether to allow modification of points. If it's just a pointset then sure, no problem. But if a pointset inherently needs a triangulation and/or representative
//polygon, then that could be an issue, since it'd need to be recalculated/retriangulated each time a point is modified
//I guess I first need to ask what the specific purpose of the pointset class is
//Like if it's just to contain a set of points with no real addnl structure, then I might as well just use the std unordered_set or unordered_map.

#ifndef POINTSET_H
#define POINTSET_H

#include "point.h"
//#include <unordered_set>
#include <unordered_map> //since x-vals are necessarily unique and y-vals are not,
						 //I think it makes the most sense to use unordered map with
						 //x-vals as keys; search can still be done including y-val if
						 //user wishes

class Pointset{
	private:
		std::unordered_map<Point> points;
		Polygon convex_hull; // "default" polygon, but many triangulations exist. 
							 // might scrap this part entirely

	public:
		/* CONSTRUCTORS */

		/*
		 * Default constructor that makes an empty pointset
		 */

		/* ACCESSORS */
		bool contains(double xval){ return points.contains(xval); }
		bool contains(Point p); //FIXME
		Point at(double xval){ return points.at(xval); }
			

		/* MUTATORS */
		void add(Point p); //add a point to the pointset
		void erase(Point p); //removes a point from the pointset 

		/* MISC */
};





#endif
