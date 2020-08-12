#include "defs.h"
#include "mpu6050.h"
#include "mpu9250.h"
#include "ak8963.h"
#include "robotWin.h"

#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

using namespace samsRobot;

void init_motors(void);

// other handlers
void* update_robot_status(void*);
void* draw_screen(void*);
void* drive_motors(void*);

// current user key, info returned from draw_screen();
int current_key = 0;
int previous_key = 0;

// current motor_status, info returned from drive_motors() and update_robot_status();
int motor_status = 0;

robotWin myscreen;

int main(int argc, char **argv)
{
    int key = 0;

    // start the curses screen
	if(myscreen.getValidWins() != TRUE)
		exit(2);

    // prepare the motors
    init_motors();

    // create a thread to handle draw screens and receive user input
    // create a thread to handle motors
    pthread_t userThreadID, motorThreadID, statusThreadID;
    pthread_attr_t userThreadAttr;
    pthread_attr_t motorThreadAttr;
    pthread_attr_t statusThreadAttr;
    pthread_attr_init(&userThreadAttr);
    pthread_attr_init(&motorThreadAttr);
    pthread_attr_init(&statusThreadAttr);
    pthread_create(&userThreadID, &userThreadAttr, draw_screen, &key);
    pthread_create(&motorThreadID, &motorThreadAttr, drive_motors, &key);
    pthread_create(&statusThreadID, &statusThreadAttr, update_robot_status, &key);

    // do other stuff here;

    pthread_join(userThreadID, NULL);
    pthread_join(motorThreadID, NULL);
    pthread_join(statusThreadID, NULL);


    exit(0);
}

void* draw_screen(void*){
	int keys = 0;

	// dont need to check for valid windows - we've got this far already!
	
		do {
			keys = myscreen.update(motor_status); 

			// make the current key available to other threads
			previous_key = current_key;
			current_key = keys;

		} while ((keys != 'q') && (keys != 'Q'));
	pthread_exit(0);
}


void* drive_motors(void*){
	// we've received this key. check to see if it requires movement
	do{
		usleep(20000); // run the motor

	}while ((current_key != 'q') && (current_key != 'Q'));

	pthread_exit(0);
}


void* update_robot_status(void*){
	float curr_yaw = 0, prev_yaw = 0;
	float curr_pitch = 0, prev_pitch = 0;
	float curr_roll = 0, prev_roll = 0;
	float temp = 0.0f;

	int x,y;
	
	// static mpu6050 boomIMU(1,0x68);

	do{
		// get the current accelerometer values
		//
		/*
		boomIMU.readSensorState();
		curr_yaw = boomIMU.getYaw();
		curr_pitch = boomIMU.getPitch();
		curr_roll = boomIMU.getRoll();
		temp = boomIMU.getTemp();

		if (((curr_yaw - prev_yaw) < YAW_THRESH) && 
			((curr_pitch - prev_pitch) < PITCH_THRESH) &&
			((curr_roll - prev_roll) < ROLL_THRESH))
			motor_status = 0;
		else
			motor_status = 1;
		
		prev_yaw = curr_yaw;
		prev_pitch = curr_pitch;
		prev_roll = curr_roll;
*/
		// go to sleep for a bit
		usleep(10000); // 10ms
	}while ((current_key != 'q') && (current_key!='Q'));
	pthread_exit(0);
}

void init_motors(void){
}
