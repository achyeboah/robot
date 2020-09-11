#ifndef UTILS_H_
#define UTILS_H_

namespace samsRobot{

	struct imu_data{
		unsigned int type;
		float pitch, yaw, roll;
		float wVelX, wVelY, wVelZ;
		float headX, headY, headZ;
		float temp;
	};

	unsigned int write_imu_data(const char* filename, const imu_data data);
	unsigned int read_imu_data(const char* filename, imu_data& data);
}

#endif
