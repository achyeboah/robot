#ifndef DEFS_H_
#define DEFS_H_

#define	samsRobot_VERSION_MAJOR 2020
#define samsRobot_VERSION_MINOR 9
#define samsRobot_VERSION_PATCH 10

namespace samsRobot{

	// terminal size requirements
	#define MIN_TERM_WIDTH 60
	#define MIN_TERM_HEIGHT 20
	
	// threshold for detecting movement
	#define YAW_THRESH 3
	#define PITCH_THRESH 3
	#define ROLL_THRESH 3

	// number of motors
	#define NUM_MOTORS 3

	// motor1 pins
	#define motor1_GPIO_pin1 17 // physical pin 11
	#define motor1_GPIO_pin2 27 // physical pin 13
	#define motor1_GPIO_pinEN 22 // physical pin 15
	// motor2 pins
	#define motor2_GPIO_pin1 10 // physical pin 19
	#define motor2_GPIO_pin2 9 // physical pin 21
	#define motor2_GPIO_pinEN 11 // physical pin 23
	// motor 3 pins
	#define motor3_GPIO_pin1 5 // physical pin 29
	#define motor3_GPIO_pin2 6 // physical pin 31
	#define motor3_GPIO_pinEN  13 // physical pin 33
	// motor 4 pins
	#define motor4_GPIO_pin1 19 // physical pin 35
	#define motor4_GPIO_pin2 26 // physical pin 37
	#define motor4_GPIO_pinEN 21 // physical pin 40


} /* namespace */
#endif /* DEFS_H_ */
