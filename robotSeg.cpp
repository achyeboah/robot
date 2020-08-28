#include "robotSeg.h"
#include <stdio.h>

namespace samsRobot{

	robotSeg::robotSeg(){
		size_x = 1.0f;
		size_y = 1.0f;
		size_z = 1.0f;
		centre_x = 0.50f;
		centre_y = 0.50f;
		centre_z = 0.50f;
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
		fprintf(stderr, "<%d>\tDims l = %02.2f, w = %02.2f, h = %02.2f\n", id, size_x, size_y, size_z);
	}
	void robotSeg::get_centre(float &x, float &y, float &z) const{
		x = centre_x; y = centre_y; z = centre_y;
	}
	void robotSeg::set_centre(const float x, const float y, const float z){
		centre_x = x; centre_y = y; centre_z = z;
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

