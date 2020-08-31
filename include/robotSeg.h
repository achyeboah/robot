#ifndef ROBOTSEG_H
#define ROBOTSEG_H

namespace samsRobot{
	/*
	class dcmotor;
	class mpu6050;
	class mpu9250;
	class ak8963;
*/
	// a segment of the robot. 
	// provides geometrical coords of a part of the robot eg the boom or tab
	// a motor is tied to the control of this segment
	// an IMU is tied to the sensing of this segment
	//
	class robotSeg{
		private:
			// graphics attributes, potentially useful also for physics
			float size_x, size_y, size_z; // in mm
			float centre_x, centre_y, centre_z; // displacement from 0,0,0
			float pivot_x, pivot_y, pivot_z; // pivot for child objects
			float col_r, col_g, col_b; // rgba
			robotSeg* parent;

			// hopefully unique id
			unsigned int id;

		public:
			robotSeg(); // presumes dimensions are 1,1,1, color red, centred around origin
			~robotSeg();

			// link management items
			void get_dimensions(float &l, float &w, float &h) const;
			void set_dimensions(const float l, const float w, const float h);
			void get_centre(float &x, float &y, float &z) const;
			void set_centre(const float x, const float y, const float z);
			void get_pivot(float &x, float &y, float &z) const;
			void set_pivot(const float x, const float y, const float z);
			void get_colors(float &r, float &g, float &b) const;
			void set_colors(const float r, const float g, const float b);

			robotSeg* getParent(void) const;
			unsigned int getParentID(void) const;
			void setParent(robotSeg* parent);

			unsigned int getID(void) const;
			void setID(const unsigned int id);
			};
}
#endif
