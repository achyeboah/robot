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
			float col_r, col_g, col_b; // rgba
			robotSeg* parent;

			// hopefully unique id
			unsigned int id;
			// is this an axis (0) or not?
			unsigned int isAxis;

		public:
			robotSeg(); // presumes dimensions are 1,1,1, color red, 
			~robotSeg();

			// link management items
			void get_dimensions(float &l, float &w, float &h) const;
			void set_dimensions(const float l=1.0f, const float w=0.1f, const float h=0.1f);
			void get_colors(float &r, float &g, float &b) const;
			void set_colors(const float r, const float g, const float b);
			void set_axis(const unsigned int axis);
			unsigned int get_axis(void) const;

			robotSeg* getParent(void) const;
			unsigned int getParentID(void) const;
			void setParent(robotSeg* parent);

			unsigned int getID(void) const;
			void setID(const unsigned int id);
			};
}
#endif
