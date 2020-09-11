#include "robotSeg.h"
#include <stdio.h>

namespace samsRobot{

	robotSeg::robotSeg(){
		// init as unit cube
		size_x = 1.0f;
		size_y = 1.0f;
		size_z = 1.0f;
		// set colors (renderer will set alpha channel)
		col_r = 0.1f;
		col_g = 0.1f;
		col_b = 0.1f;

		parent = NULL;
		id = 1;
		isAxis = 0;
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
	void robotSeg::get_colors(float &r, float &g, float &b) const{
		r = col_r; g = col_g; b = col_b;
	}
	void robotSeg::set_colors(const float r, const float g, const float b){
		col_r = r; col_g = g; col_b = b;
	}
	void robotSeg::set_axis(const unsigned int axis){
		isAxis = axis;
	}
	unsigned int robotSeg::get_axis(void) const{ return isAxis;}

	void robotSeg::setParent(robotSeg* parent){ this->parent = parent;}
	robotSeg* robotSeg::getParent(void) const{ return this->parent;}
	unsigned int robotSeg::getParentID(void) const {
		if(this->getParent() != NULL)
			return (this->getParent())->getID();
		else
			return 0;
	}

	unsigned int robotSeg::getID(void) const {return this->id;}
	void robotSeg::setID(const unsigned int id){this->id = id;}
};

