//implementation file for the line class

//TODO: FIGURE OUT HOW TO HANDLE VERTICAL LINES
// Current plan is add the vert bool and handle these differently; another option is to
// extend the line class to have a vertical line
// With the current model, basically every time you're dealing with a line, need to check if it
// is vertical, and, if so, handle it differently
// Actually, I think I'm just gonna assume general position; that no two points have the same x-val

#include "line.h"

using namespace std;

Line::Line(){ //defualt constructor; makes the line y=x
	slope = 1.0;
	y_intercept = 0.0;
}

Line::Line(double slope){ //makes a line with given slope that passes through the origin
	this->slope = slope;
	y_intercept = 0.0;
}

Line::Line(double slope, double y_intercept){ //makes a line with given slope and given y-intercept
	this->slope = slope;
	this->y_intercept = y_intercept;
}

Line::Line(double slope, Point p){ //makes a line with given slope that passes through given point p
	this->slope = slope;
	//y = mx+b
	// p.y = m(p.x) + b
	// b = p.y - m(p.x)
	y_intercept = p.get_y() - slope*(p.get_x());
}

Line::Line(Point p1, Point p2){
	//TODO: make sure p1 and p2 are not the same point!!!
	if(p1.get_x() == p2.get_x() and p1.get_y() == p2.get_y()){ //same point
								   //just give slope 1
		Line l = Line(1,p1);
		slope = 1;
		y_intercept = l.get_y_intercept();	

	}else if(p1.get_x() == p2.get_x()) die(); //just detonate a nuclear bomb if they
						  //give 2 points with the same x-value
	slope = (p1.get_y() - p2.get_y()) / (p1.get_x() - p2.get_x());
	Line temp = Line(slope,p1); //haha cheating
	y_intercept = temp.get_y_intercept();
}


bool Line::intersects(Line other){ //returns true if this->line intersects the other given line
	if(slope != other.get_slope()) return true;
	if(y_intercept == other.get_y_intercept()) return true;
	return false;	
}

Point Line::intersection(Line other){
	//make sure to handle if they are the same line first
	if(slope == other.get_slope()){
		//either no intersection (return null,null) or same line (return y-intercept)
		if(this->intersects(other)) return Point(0, y_intercept);
		else return Point(0,0);
	}
	// x = (b2 - b1) / (m1-m2)
	// if the two lines have the same slope this could be problematic (divide by 0) - but this 
	// should already be solved by the initial check
	double new_x = ( (other.get_y_intercept() - y_intercept) / (slope - other.get_slope()) ); 
	//get the x-value where they intercept
	double new_y = slope * new_x + y_intercept; //just plug it into the first line equation
	return Point(new_x,new_y);
}

void Line::print(){ //print the line as follows: "y = [slope]x + [y-intercept]"
	cout << "y = " << slope << "x + " << y_intercept;
}

string Line::to_string() const{
	stringstream ss("");
	ss << "y = " << slope << "x + " << y_intercept;
	return ss.str();
}
