#include "defs.h"
#include "mpu6050.h"
#include "mpu9250.h"
#include "ak8963.h"
#include "robotCurses.h"
#include "robotGL.h"
#include "robotSeg.h"

#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

using namespace samsRobot;

void init_motors(void);
void print_usage(int argc, char **argv);

// other handlers
void* update_robot_status(void*);
void* draw_curses(void*);
void* drive_motors(void*);
void* draw_graphics(void*);

// current user key, info returned from draw_curses();
int current_key = 0;
bool quit_gl = FALSE;
bool do_fullscreen = FALSE;
int previous_key = 0;
float robotGL_fps = 0.0f;

// current motor_status, info returned from drive_motors() and update_robot_status();
int motor_status = 0;

// create just one segment to start with
// global because accessed by multiple threads
robotSeg x_axis, y_axis, z_axis, seg1, seg2, seg3;

int main(int argc, char **argv)
{
	/* options for this program are as follows
	 * c|C displays the ncurses screen
	 * g|G displays the opengl screen
	 * f|F displays the opengl screen in fullscreen
	 * absence of either of these, or any other displays usage screen
	 */

	int key = 0;
	bool do_curses = FALSE;
	bool do_gl = FALSE;
	int opt; // track options

	// create axes here
	x_axis.setID(1);
	x_axis.set_dimensions(100,0.05,0.05);
	x_axis.set_pivot(0,0,0);
	x_axis.set_colors(1,0.0,0.0);
	y_axis.setID(2);
	y_axis.set_dimensions(0.05,100,0.05);
	y_axis.set_pivot(0,0,0);
	y_axis.set_colors(0,1,0);
	z_axis.setID(3);
	z_axis.set_dimensions(0.05,0.05,100);
	z_axis.set_pivot(0,0,0);
	z_axis.set_colors(0,0,1);

	// create robot here
	seg1.setID(5);
	seg1.set_dimensions(3,1,1);
	seg1.set_pivot(2.9,0.5,0.5);
	seg1.set_colors(0.5,0.1,0.1);
	seg2.setID(6);
	seg2.set_dimensions(2,1,1);
	seg2.set_pivot(1.9,0.5,0.5);
	seg2.set_colors(0.1,0.5,0.1);
	seg2.setParent(&seg1);
	seg3.setID(7);
	seg3.set_dimensions(2,1,1);
	seg3.set_pivot(1.9,0.5,0.5);
	seg3.set_colors(0.1,0.1,0.5);
	seg3.setParent(&seg2);

	if (argc < 2){
		print_usage(argc, argv);
		exit(EXIT_FAILURE);
	}

	while((opt = getopt(argc, argv, "cfgCFG")) != -1){
		switch(opt){
			case 'c':
			case 'C':
				do_curses = TRUE;
				break;
			case 'f':
			case 'F':
				do_fullscreen = TRUE;
				do_gl = TRUE;
				break;
			case 'g':
			case 'G':
				do_gl = TRUE;
				break;
			default:
				print_usage(argc, argv);
				do_gl = TRUE;
				// exit(EXIT_FAILURE);
		}
	}

	// if both curses and opengl are not started, nothing to show, so just exit
	if ((do_gl == FALSE) && (do_curses == FALSE))
		exit(0);

	// prepare the motors
	init_motors();

	// create a thread to handle draw screens and receive user input
	// create a thread to handle motors
	// create a thread to get the status of the motors/controller
	// create a thread to draw the opengl window
	pthread_t userThreadID, motorThreadID, statusThreadID, openGLThreadID;
	pthread_attr_t userThreadAttr;
	pthread_attr_t motorThreadAttr;
	pthread_attr_t statusThreadAttr;
	pthread_attr_t openGLThreadAttr;
	pthread_attr_init(&userThreadAttr);
	pthread_attr_init(&motorThreadAttr);
	pthread_attr_init(&statusThreadAttr);
	pthread_attr_init(&openGLThreadAttr);

	if (do_curses == TRUE)
		pthread_create(&userThreadID, &userThreadAttr, draw_curses, &key);
	if (do_gl == TRUE)
		pthread_create(&openGLThreadID, &openGLThreadAttr, draw_graphics, &key);

	pthread_create(&motorThreadID, &motorThreadAttr, drive_motors, &key);
	pthread_create(&statusThreadID, &statusThreadAttr, update_robot_status, &key);

	// do other stuff here;

	if (do_gl == TRUE)
		pthread_join(openGLThreadID, NULL);
	if (do_curses == TRUE)
		pthread_join(userThreadID, NULL);

	pthread_join(motorThreadID, NULL);
	pthread_join(statusThreadID, NULL);

	exit(0);
}

void* draw_curses(void*){
	int keys = 0;
	robotCurses myscreen;

	if(myscreen.getValidWins() != TRUE)
		pthread_exit(0);

	// dont need to check for valid windows - we've got this far already!
	do {
		// check if some other thread said to quit
		if (quit_gl != TRUE){

			myscreen.set_ogl_fps(robotGL_fps);
			keys = myscreen.update(motor_status); 

			// make the current key available to other threads
			previous_key = current_key;
			current_key = keys;
			usleep(20000); // rest a bit
		}else{
			keys = 'q';
			current_key = keys;
		}

	} while ((keys != 'q') && (keys != 'Q'));

	// received a q
	pthread_exit(0);
}


void* drive_motors(void*){
	// we've received this key. check to see if it requires movement
	do{
		usleep(20000); // run the motor

	}while ((current_key != 'q') && (current_key != 'Q') && (quit_gl != TRUE));

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
	}while ((current_key != 'q') && (current_key!='Q') && (quit_gl !=TRUE));
	pthread_exit(0);
}

void init_motors(void){
}

void* draw_graphics(void*){
	robotGL glWin(do_fullscreen);
	// need to pass in some parameters
	glWin.set_bg(0.0f, 0.0f, 0.1f, 0.2f);

	glWin.create_cuboid(seg1);	
	glWin.create_cuboid(seg2);	
	glWin.create_cuboid(seg3);	
	glWin.create_cuboid(x_axis);
	glWin.create_cuboid(y_axis);
	glWin.create_cuboid(z_axis);

	do{
	 	glWin.updateScreen();
		robotGL_fps = glWin.get_fps();
		usleep(20000); // achieving 80fps with 10ms sleep, 20fps with 50ms sleep
	}while(glWin.get_progFinished() == FALSE);

	// send a message to other threads that its time to quit!
	quit_gl = TRUE;
	pthread_exit(0);

}

void print_usage(int argc, char** argv){
	printf("\n\nWelcome! ");
	printf("This is %s v%02d.%02d.%02d\n",
			argv[0],
			samsRobot_VERSION_MAJOR,
			samsRobot_VERSION_MINOR,
			samsRobot_VERSION_PATCH );
	printf("Please note that program %s is as follows. \n", argv[0]);
	printf("\t-c\tUse NCurses window\n");
	printf("\t-g\tUse openGL window\n");
	printf("\t-f\tUse full screen (calls -g)\n");
	printf("\nOptions may be combined eg (-gc)\n\n");
	printf("Keys inside the application:\n");
	printf("\tLArrow\tMove camera left\n");
	printf("\tRArrow\tMove camera right\n");
	printf("\tUArrow\tMove camera up\n");
	printf("\tDArrow\tMove camera down\n");
	printf("\tPgUp\tMove camera towards view\n");
	printf("\tPgDn\tMove camera away from view\n");
	printf("\n");
}
