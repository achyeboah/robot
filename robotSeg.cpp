#include "robotSeg.h"
#include <stdio.h>

namespace samsRobot{

	robotSeg::robotSeg(){
		// init as unit cube
		size_x = 1.0f;
		size_y = 1.0f;
		size_z = 1.0f;
		// place centre in middle
		centre_x = size_x/2.0f;
		centre_y = size_y/2.0f;
		centre_z = size_z/2.0f;
		// place pivot  (ie rotation point for child object at middle of end face
		pivot_x = size_x;
		pivot_y = size_y/2;
		pivot_z = size_z/2;
		// set colors (renderer will set alpha channel)
		col_r = 0.1f;
		col_g = 0.1f;
		col_b = 0.1f;

		parent = NULL;
		id = 1;
	}

	robotSeg::~robotSeg(){
		// do nothing for now;
	}

	// link management items
	void robotSeg::get_dimensions(float &l, float &w, float &h) const{
		l = size_x; w = size_y; h = size_z;
	}
	void robotSeg::set_dimensions(const float l, const float w, const float h){
		size_x = l; size_y = w; size_z = h;
	}
	void robotSeg::get_centre(float &x, float &y, float &z) const{
		x = centre_x; y = centre_y; z = centre_y;
	}
	void robotSeg::set_centre(const float x, const float y, const float z){
		centre_x = x; centre_y = y; centre_z = z;
	}
	void robotSeg::get_pivot(float &x, float &y, float &z) const{
		x = pivot_x; y = pivot_y; z = pivot_y;
	}
	void robotSeg::set_pivot(const float x, const float y, const float z){
		pivot_x = x; pivot_y = y; pivot_z = z;
	}
	void robotSeg::get_colors(float &r, float &g, float &b) const{
		r = col_r; g = col_g; b = col_b;
	}
	void robotSeg::set_colors(const float r, const float g, const float b){
		col_r = r; col_g = g; col_b = b;
	}

	robotSeg* robotSeg::getParent(void) const{ return this->parent;}
	void robotSeg::setParent(robotSeg* parent){ this->parent = parent;}

	int robotSeg::getID(void) const {return this->id;}
	void robotSeg::setID(const int id){this->id = id;}
};

